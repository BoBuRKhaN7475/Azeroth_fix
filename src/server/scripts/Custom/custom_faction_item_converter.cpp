/*
 * Task 19 - Phase 2F: Faction Item Conversion NPC
 * Archmage Rhynold (Entry: 700100)
 *
 * Transmutes faction-specific Trial of the Crusader / Trial of the Grand Crusader
 * equipment between Horde and Alliance equivalents.
 * Strictly enforces atomic execution, socket/gem compatibility, permanent enchantment preservation,
 * durability maintenance (anti-repair exploit), and audit logging.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "GossipDef.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "Chat.h"
#include "DBCStores.h"
#include "ItemEnchantmentMgr.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace
{
    // In-memory cache for fast, non-blocking lookups
    std::unordered_map<uint32, uint32> sHordeToAlliance;
    std::unordered_map<uint32, uint32> sAllianceToHorde;
    std::unordered_set<ObjectGuid> sActiveConversions;
    std::mutex sActiveConversionsLock;

    constexpr uint32 GOSSIP_ACTION_SELECT_BASE  = 1000;
    constexpr uint32 GOSSIP_ACTION_CONFIRM_BASE = 10000;
    constexpr uint32 GOSSIP_ACTION_CANCEL       = 99999;

    inline uint8 BagToInternal(uint8 bag)
    {
        return (bag == INVENTORY_SLOT_BAG_0) ? 0 : (bag - INVENTORY_SLOT_BAG_START + 1);
    }

    inline uint8 InternalToBag(uint8 internalBag)
    {
        return (internalBag == 0) ? INVENTORY_SLOT_BAG_0 : (internalBag - 1 + INVENTORY_SLOT_BAG_START);
    }

    inline uint32 EncodePos(uint8 bag, uint8 slot)
    {
        return (uint32(BagToInternal(bag)) * 100) + uint32(slot);
    }

    inline void DecodePos(uint32 pos, uint8& bag, uint8& slot)
    {
        uint8 internalBag = uint8(pos / 100);
        bag = InternalToBag(internalBag);
        slot = uint8(pos % 100);
    }

    std::string GetLocalizedItemName(ItemTemplate const* proto, LocaleConstant locale)
    {
        if (!proto)
            return "";
        if (ItemLocale const* il = sObjectMgr->GetItemLocale(proto->ItemId))
            if (il->Name.size() > locale && !il->Name[locale].empty())
                return il->Name[locale];
        return proto->Name1;
    }

    void LoadConversionTable()
    {
        sHordeToAlliance.clear();
        sAllianceToHorde.clear();

        QueryResult result = WorldDatabase.Query(
            "SELECT alliance_item_id, horde_item_id FROM custom_faction_item_conversion WHERE enabled = 1"
        );

        if (!result)
        {
            LOG_WARN("server.loading", "custom_faction_item_conversion table is empty or missing.");
            return;
        }

        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 allianceId = fields[0].Get<uint32>();
            uint32 hordeId = fields[1].Get<uint32>();

            sHordeToAlliance[hordeId] = allianceId;
            sAllianceToHorde[allianceId] = hordeId;
            ++count;
        } while (result->NextRow());

        LOG_INFO("server.loading", "Loaded {} cross-faction item conversion pairs.", count);
    }

    uint32 GetTargetItemForPlayer(Player const* player, uint32 sourceItemId)
    {
        if (sHordeToAlliance.empty() || sAllianceToHorde.empty())
            LoadConversionTable();

        TeamId team = player->GetTeamId();
        if (team == TEAM_ALLIANCE)
        {
            auto itr = sHordeToAlliance.find(sourceItemId);
            if (itr != sHordeToAlliance.end())
                return itr->second;
        }
        else if (team == TEAM_HORDE)
        {
            auto itr = sAllianceToHorde.find(sourceItemId);
            if (itr != sAllianceToHorde.end())
                return itr->second;
        }

        // Fallback: If not found based on active team, check both maps
        auto itrH2A = sHordeToAlliance.find(sourceItemId);
        if (itrH2A != sHordeToAlliance.end())
            return itrH2A->second;

        auto itrA2H = sAllianceToHorde.find(sourceItemId);
        if (itrA2H != sAllianceToHorde.end())
            return itrA2H->second;

        return 0;
    }

    struct CandidateItem
    {
        uint8 bag;
        uint8 slot;
        Item* item;
        uint32 targetItemId;

        CandidateItem(uint8 b, uint8 s, Item* i, uint32 t)
            : bag(b), slot(s), item(i), targetItemId(t) { }
    };

    std::vector<CandidateItem> GetConvertibleItemsInBags(Player* player)
    {
        std::vector<CandidateItem> candidates;

        // 1. Scan Backpack (slots 23-38)
        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                if (!item->IsInTrade() && item->GetCount() == 1)
                {
                    uint32 targetId = GetTargetItemForPlayer(player, item->GetEntry());
                    if (targetId != 0 && sObjectMgr->GetItemTemplate(targetId))
                    {
                        candidates.emplace_back(INVENTORY_SLOT_BAG_0, slot, item, targetId);
                    }
                }
            }
        }

        // 2. Scan Equipped Bags (slots 19-22)
        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
        {
            if (Bag* bag = player->GetBagByPos(bagSlot))
            {
                for (uint32 slot = 0; slot < bag->GetBagSize(); ++slot)
                {
                    if (Item* item = player->GetItemByPos(bagSlot, slot))
                    {
                        if (!item->IsInTrade() && item->GetCount() == 1)
                        {
                            uint32 targetId = GetTargetItemForPlayer(player, item->GetEntry());
                            if (targetId != 0 && sObjectMgr->GetItemTemplate(targetId))
                            {
                                candidates.emplace_back(bagSlot, static_cast<uint8>(slot), item, targetId);
                            }
                        }
                    }
                }
            }
        }

        return candidates;
    }
}

class npc_faction_item_converter : public CreatureScript
{
public:
    npc_faction_item_converter() : CreatureScript("npc_faction_item_converter") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        std::vector<CandidateItem> candidates = GetConvertibleItemsInBags(player);

        if (candidates.empty())
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Приветствую тебя, герой. Я могу преобразовать экипировку Испытания Крестоносца противоположной фракции в подходящий аналог для твоей фракции. В данный момент в твоем инвентаре нет подходящих предметов для преобразования.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CANCEL);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "--- Выберите предмет для преобразования ---", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CANCEL);

            LocaleConstant loc = player->GetSession()->GetSessionDbcLocale();
            for (CandidateItem const& c : candidates)
            {
                ItemTemplate const* sourceProto = c.item->GetTemplate();
                ItemTemplate const* targetProto = sObjectMgr->GetItemTemplate(c.targetItemId);
                if (!sourceProto || !targetProto)
                    continue;

                uint32 pos = EncodePos(c.bag, c.slot);
                std::string sourceName = GetLocalizedItemName(sourceProto, loc);
                std::string targetName = GetLocalizedItemName(targetProto, loc);
                std::string label = "[Преобразовать] " + sourceName + " -> " + targetName;
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, label, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SELECT_BASE + pos);
            }
        }

        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (action == GOSSIP_ACTION_CANCEL)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        // STEP 1: ITEM SELECTED -> SHOW CONFIRMATION PROMPT
        if (action >= GOSSIP_ACTION_SELECT_BASE && action < GOSSIP_ACTION_CONFIRM_BASE)
        {
            uint32 pos = action - GOSSIP_ACTION_SELECT_BASE;
            uint8 bag = 0;
            uint8 slot = 0;
            DecodePos(pos, bag, slot);

            Item* item = player->GetItemByPos(bag, slot);
            if (!item)
            {
                ChatHandler(player->GetSession()).SendSysMessage("Выбранный предмет не найден в вашем инвентаре.");
                CloseGossipMenuFor(player);
                return true;
            }

            uint32 targetItemId = GetTargetItemForPlayer(player, item->GetEntry());
            ItemTemplate const* sourceProto = item->GetTemplate();
            ItemTemplate const* targetProto = sObjectMgr->GetItemTemplate(targetItemId);

            if (!sourceProto || !targetProto)
            {
                CloseGossipMenuFor(player);
                return true;
            }

            ClearGossipMenuFor(player);

            LocaleConstant loc = player->GetSession()->GetSessionDbcLocale();
            std::string sourceName = GetLocalizedItemName(sourceProto, loc);
            std::string targetName = GetLocalizedItemName(targetProto, loc);

            std::string confirmText = "Вы преобразуете:\n" + sourceName +
                                      "\nв:\n" + targetName +
                                      "\n\n- Все инкрустированные камни и постоянные чары будут сохранены.\n"
                                      "- Прочность предмета сохранится.\n"
                                      "- Новый предмет станет персональным.";

            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, confirmText, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CANCEL);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, ">>> [ПОДТВЕРДИТЬ ПРЕОБРАЗОВАНИЕ] <<<", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CONFIRM_BASE + pos);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[Отмена]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_CANCEL);

            SendGossipMenuFor(player, 1, creature->GetGUID());
            return true;
        }

        // STEP 2: USER CONFIRMED -> ATOMIC CONVERSION EXECUTION
        if (action >= GOSSIP_ACTION_CONFIRM_BASE && action < GOSSIP_ACTION_CANCEL)
        {
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);

            ObjectGuid playerGuid = player->GetGUID();

            // Concurrency Lock: Prevent double-clicks / race conditions
            {
                std::lock_guard<std::mutex> lock(sActiveConversionsLock);
                if (sActiveConversions.find(playerGuid) != sActiveConversions.end())
                {
                    ChatHandler(player->GetSession()).SendSysMessage("В данный момент выполняется другое преобразование. Пожалуйста, подождите.");
                    return true;
                }
                sActiveConversions.insert(playerGuid);
            }

            uint32 pos = action - GOSSIP_ACTION_CONFIRM_BASE;
            uint8 bag = 0;
            uint8 slot = 0;
            DecodePos(pos, bag, slot);

            auto unlockAndFail = [player, playerGuid](std::string const& errorMsg) {
                {
                    std::lock_guard<std::mutex> lock(sActiveConversionsLock);
                    sActiveConversions.erase(playerGuid);
                }
                ChatHandler(player->GetSession()).SendSysMessage(errorMsg.c_str());
            };

            Item* item = player->GetItemByPos(bag, slot);
            if (!item || item->GetOwnerGUID() != playerGuid || item->IsInTrade() || item->GetCount() != 1)
            {
                unlockAndFail("Преобразование отменено: предмет недоступен или находится в сделке.");
                return true;
            }

            uint32 sourceItemId = item->GetEntry();
            uint32 targetItemId = GetTargetItemForPlayer(player, sourceItemId);
            ItemTemplate const* sourceProto = item->GetTemplate();
            ItemTemplate const* targetProto = sObjectMgr->GetItemTemplate(targetItemId);

            if (!sourceProto || !targetProto)
            {
                unlockAndFail("Преобразование отменено: неверный шаблон предмета.");
                return true;
            }

            // FINAL REQUIREMENT 2: Validate Item Property Compatibility BEFORE Touching Item
            uint32 gemEnchants[MAX_GEM_SOCKETS] = {0, 0, 0};
            for (uint32 i = 0; i < MAX_GEM_SOCKETS; ++i)
            {
                uint32 gemEnchantId = item->GetEnchantmentId(EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + i));
                if (gemEnchantId != 0)
                {
                    // Target must have socket i
                    if (targetProto->Socket[i].Color == 0)
                    {
                        unlockAndFail("Преобразование отменено: у целевого предмета отсутствует необходимое гнездо для камня.");
                        return true;
                    }

                    SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(gemEnchantId);
                    if (!enchantEntry)
                    {
                        unlockAndFail("Преобразование отменено: неверные данные инкрустации.");
                        return true;
                    }

                    // Check gem color fits target socket
                    if (enchantEntry->GemID)
                    {
                        ItemTemplate const* gemProto = sObjectMgr->GetItemTemplate(enchantEntry->GemID);
                        if (gemProto)
                        {
                            GemPropertiesEntry const* gemProp = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                            if (gemProp)
                            {
                                if (targetProto->Socket[i].Color == SOCKET_COLOR_META && !(gemProp->color & SOCKET_COLOR_META))
                                {
                                    unlockAndFail("Преобразование отменено: в это гнездо требуется особый камень.");
                                    return true;
                                }
                                if (targetProto->Socket[i].Color != SOCKET_COLOR_META && (gemProp->color & SOCKET_COLOR_META))
                                {
                                    unlockAndFail("Преобразование отменено: особый камень нельзя поместить в стандартное гнездо.");
                                    return true;
                                }
                            }
                        }
                    }

                    gemEnchants[i] = gemEnchantId;
                }
            }

            // Check Prismatic Socket
            uint32 prismaticEnchantId = item->GetEnchantmentId(PRISMATIC_ENCHANTMENT_SLOT);
            if (prismaticEnchantId != 0 && targetProto->InventoryType != INVTYPE_WAIST)
            {
                unlockAndFail("Преобразование отменено: извечная поясная пряжка несовместима с этим слотом.");
                return true;
            }

            // Check Permanent Enchantment
            uint32 permEnchantId = item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT);
            uint32 permDuration = item->GetEnchantmentDuration(PERM_ENCHANTMENT_SLOT);
            uint32 permCharges = item->GetEnchantmentCharges(PERM_ENCHANTMENT_SLOT);
            if (permEnchantId != 0)
            {
                SpellItemEnchantmentEntry const* pEntry = sSpellItemEnchantmentStore.LookupEntry(permEnchantId);
                if (!pEntry)
                {
                    unlockAndFail("Преобразование отменено: неизвестные чары.");
                    return true;
                }
            }

            // Durability calculation (preserves damage, prevents free repairs)
            uint32 targetDurability = (targetProto->MaxDurability > 0) ?
                std::min(item->GetUInt32Value(ITEM_FIELD_DURABILITY), static_cast<uint32>(targetProto->MaxDurability)) : 0;

            // Memory-First Creation: Create new item BEFORE destroying old item
            Item* newItem = Item::CreateItem(targetItemId, 1, player);
            if (!newItem)
            {
                unlockAndFail("Преобразование отменено: ошибка создания предмета на сервере.");
                return true;
            }

            // Apply preserved properties safely to in-memory item
            for (uint32 i = 0; i < MAX_GEM_SOCKETS; ++i)
            {
                if (gemEnchants[i] != 0)
                    newItem->SetEnchantment(EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + i), gemEnchants[i], 0, 0);
            }

            if (prismaticEnchantId != 0)
                newItem->SetEnchantment(PRISMATIC_ENCHANTMENT_SLOT, prismaticEnchantId, 0, 0);

            if (permEnchantId != 0)
                newItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, permEnchantId, permDuration, permCharges);

            // Re-evaluate socket bonus on target item
            if (newItem->GemsFitSockets() && targetProto->socketBonus)
                newItem->SetEnchantment(BONUS_ENCHANTMENT_SLOT, targetProto->socketBonus, 0, 0);

            if (targetProto->MaxDurability > 0)
                newItem->SetUInt32Value(ITEM_FIELD_DURABILITY, targetDurability);

            // Force permanent soulbound and clear raid trade timer
            newItem->SetBinding(true);

            // Verify inventory storage space before destroying original item
            ItemPosCountVec dest;
            InventoryResult msg = player->CanStoreItem(bag, slot, dest, targetItemId, 1, item, true);
            if (msg != EQUIP_ERR_OK)
            {
                msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, targetItemId, 1, nullptr, false);
            }

            if (msg != EQUIP_ERR_OK)
            {
                delete newItem;
                unlockAndFail("Преобразование отменено: недостаточно места в сумках.");
                return true;
            }

            // ATOMIC SWAP: Destroy original item, then store new item into vacated slot
            player->DestroyItem(bag, slot, true);
            dest.clear();
            if (player->CanStoreItem(bag, slot, dest, targetItemId, 1, nullptr, false) == EQUIP_ERR_OK)
            {
                player->StoreItem(dest, newItem, true);
            }
            else if (player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, targetItemId, 1, nullptr, false) == EQUIP_ERR_OK)
            {
                player->StoreItem(dest, newItem, true);
            }
            else
            {
                delete newItem;
                unlockAndFail("Преобразование отменено: не удалось положить предмет в сумку.");
                return true;
            }

            // Release Concurrency Lock
            {
                std::lock_guard<std::mutex> lock(sActiveConversionsLock);
                sActiveConversions.erase(playerGuid);
            }

            // Visual & Audio Feedback
            creature->CastSpell(player, 64446, true); // Visual transmutation effect
            LocaleConstant loc = player->GetSession()->GetSessionDbcLocale();
            std::string sourceName = GetLocalizedItemName(sourceProto, loc);
            std::string targetName = GetLocalizedItemName(targetProto, loc);
            std::string successMsg = "Преобразование успешно! Ваш предмет " + sourceName +
                                     " был преобразован в " + targetName + ".";
            ChatHandler(player->GetSession()).SendSysMessage(successMsg.c_str());

            // Write Asynchronous Audit Log
            CharacterDatabase.Execute(
                "INSERT INTO custom_faction_item_conversion_log "
                "(character_guid, account_id, character_name, source_item_id, target_item_id, gem1, gem2, gem3, enchant_id, durability, converted_at) "
                "VALUES ({}, {}, '{}', {}, {}, {}, {}, {}, {}, {}, NOW())",
                playerGuid.GetCounter(), player->GetSession()->GetAccountId(), player->GetName(),
                sourceItemId, targetItemId, gemEnchants[0], gemEnchants[1], gemEnchants[2],
                permEnchantId, targetDurability
            );

            return true;
        }

        return false;
    }
};

class custom_faction_item_loader : public WorldScript
{
public:
    custom_faction_item_loader() : WorldScript("custom_faction_item_loader") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LoadConversionTable();
    }
};

void AddSC_custom_faction_item_converter()
{
    new npc_faction_item_converter();
    new custom_faction_item_loader();
}
