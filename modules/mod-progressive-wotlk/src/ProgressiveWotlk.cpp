/*
 * mod-progressive-wotlk
 * AzerothCore Progressive WotLK Phase System
 */

#include "ProgressiveWotlk.h"

#include "AllBattlegroundScript.h"
#include "AllCreatureScript.h"
#include "ArenaSeasonMgr.h"
#include "Battleground.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "LFGMgr.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerScript.h"
#include "WorldScript.h"
#include "WorldSession.h"

// ============================================================
// Database & Configuration Helpers
// ============================================================

uint8 PW_GetCurrentPhase()
{
    QueryResult result = WorldDatabase.Query("SELECT `current_phase` FROM `progressive_phase` LIMIT 1");
    if (!result)
    {
        LOG_ERROR("module", "[ProgressiveWotlk] progressive_phase jadvali topilmadi! Phase 1 da davom etamiz.");
        return 1;
    }
    return (*result)[0].Get<uint8>();
}

void PW_ApplyPhaseDisables(uint8 currentPhase)
{
    // 1. Raid va Dungeon maplarini to'g'ri diffMask bilan bloklash
    uint32 mapBlocked = 0;
    for (uint8 i = 0; i < PHASE_MAP_TABLE_SIZE; ++i)
    {
        const PhaseMapEntry& entry = PHASE_MAP_TABLE[i];
        if (entry.unlockPhase > currentPhase)
        {
            sDisableMgr->AddDisable(DISABLE_TYPE_MAP, entry.mapId, entry.diffMask, "", "");
            sDisableMgr->AddDisable(DISABLE_TYPE_LFG_MAP, entry.mapId, 0, "", "");
            ++mapBlocked;
        }
    }

    // 2. Isle of Conquest (Battleground ID 30) ni Phase 1 va 2 da bloklash
    if (currentPhase < 3)
    {
        sDisableMgr->AddDisable(DISABLE_TYPE_BATTLEGROUND, 30, 0, "", "");
    }

    // 2b. Strand of the Ancients (Берег Древних, BG ID: 9, Map ID: 607) ni bloklash
    if (PWConfig::DisableStrandOfTheAncients)
    {
        sDisableMgr->AddDisable(DISABLE_TYPE_BATTLEGROUND, 9, 0, "", "");
        sDisableMgr->AddDisable(DISABLE_TYPE_MAP, 607, 1, "", "");
        sDisableMgr->AddDisable(DISABLE_TYPE_LFG_MAP, 607, 0, "", "");
    }

    // 3. Argent Tournament boshlang'ich kvestlarini Phase 1 da bloklash
    if (currentPhase < 2)
    {
        for (uint32 qid : {
            13667, 13668, 13633, 13634, 13592, 13593, 13677, 13678, 13679, 13680, 13681, 13745, 13746, 13747, 13748, 13749
        })
        {
            sDisableMgr->AddDisable(DISABLE_TYPE_QUEST, qid, 0, "", "");
        }
    }

    // 4. ToC bilan bog'liq Argent Tournament dailylarini Phase 1-2 da bloklash
    if (currentPhase < 3)
    {
        for (uint32 qid : {
            14016, 14074, 14076, 14077, 14080, 14090, 14092, 14096, 14101, 14102, 14104, 14105, 14107, 14108, 14112, 14136, 14140, 14141, 14142, 14143, 14144, 14145, 14152
        })
        {
            sDisableMgr->AddDisable(DISABLE_TYPE_QUEST, qid, 0, "", "");
        }
    }

    // 5. Haftalik Raid Questlarni (Lan'dalock: 24579..24590) faza bo'yicha boshqarish:
    // Faza 1: Naxx, OS, Malygos (24579..24584) faol. Ulduar, ToC, ICC bloklanadi.
    // Faza 2: + Ulduar (24585..24588) ochiladi. ToC, ICC bloklanadi.
    // Faza 3: + ToC / Lord Jaraxxus (24589) ochiladi. Faqat ICC / Lord Marrowgar (24590) bloklanadi.
    // Faza 4: barcha haftalik kvestlar ochiladi.
    if (currentPhase < 2)
    {
        for (uint32 qid = 24585; qid <= 24590; ++qid)
            sDisableMgr->AddDisable(DISABLE_TYPE_QUEST, qid, 0, "", "");
    }
    else if (currentPhase < 3)
    {
        for (uint32 qid = 24589; qid <= 24590; ++qid)
            sDisableMgr->AddDisable(DISABLE_TYPE_QUEST, qid, 0, "", "");
    }
    else if (currentPhase < 4)
    {
        sDisableMgr->AddDisable(DISABLE_TYPE_QUEST, 24590, 0, "", "");
    }

    LOG_INFO("module", "[ProgressiveWotlk] Phase {} uchun {} ta map va tegishli kvestlar bloklandi.", currentPhase, mapBlocked);
}

void PW_ApplyPhaseQuestRewards(uint8 currentPhase)
{
    uint32 lfgHeroic1stItem = ITEM_EMBLEM_OF_VALOR;
    uint32 lfgHeroicNthItem = ITEM_EMBLEM_OF_HEROISM;
    uint32 lfgNormal1stItem = ITEM_EMBLEM_OF_HEROISM;
    uint32 dalaranHeroicItem = ITEM_EMBLEM_OF_VALOR;
    uint32 dalaranNormalItem = ITEM_EMBLEM_OF_HEROISM;

    uint32 weeklyRaid1stItem = ITEM_EMBLEM_OF_VALOR;
    uint32 weeklyRaid2ndItem = ITEM_EMBLEM_OF_HEROISM;

    switch (currentPhase)
    {
        case 1:
            lfgHeroic1stItem   = ITEM_EMBLEM_OF_VALOR;
            lfgHeroicNthItem   = ITEM_EMBLEM_OF_HEROISM;
            lfgNormal1stItem   = ITEM_EMBLEM_OF_HEROISM;
            dalaranHeroicItem  = ITEM_EMBLEM_OF_VALOR;
            dalaranNormalItem  = ITEM_EMBLEM_OF_HEROISM;
            weeklyRaid1stItem  = ITEM_EMBLEM_OF_VALOR;
            weeklyRaid2ndItem  = ITEM_EMBLEM_OF_HEROISM;
            break;
        case 2:
            lfgHeroic1stItem   = ITEM_EMBLEM_OF_CONQUEST;
            lfgHeroicNthItem   = ITEM_EMBLEM_OF_VALOR;
            lfgNormal1stItem   = ITEM_EMBLEM_OF_HEROISM;
            dalaranHeroicItem  = ITEM_EMBLEM_OF_CONQUEST;
            dalaranNormalItem  = ITEM_EMBLEM_OF_HEROISM;
            weeklyRaid1stItem  = ITEM_EMBLEM_OF_CONQUEST;
            weeklyRaid2ndItem  = ITEM_EMBLEM_OF_VALOR;
            break;
        case 3:
            lfgHeroic1stItem   = ITEM_EMBLEM_OF_TRIUMPH;
            lfgHeroicNthItem   = ITEM_EMBLEM_OF_CONQUEST;
            lfgNormal1stItem   = ITEM_EMBLEM_OF_CONQUEST;
            dalaranHeroicItem  = ITEM_EMBLEM_OF_TRIUMPH;
            dalaranNormalItem  = ITEM_EMBLEM_OF_CONQUEST;
            weeklyRaid1stItem  = ITEM_EMBLEM_OF_TRIUMPH;
            weeklyRaid2ndItem  = ITEM_EMBLEM_OF_CONQUEST;
            break;
        case 4:
        case 5:
        default:
            lfgHeroic1stItem   = ITEM_EMBLEM_OF_FROST;
            lfgHeroicNthItem   = ITEM_EMBLEM_OF_TRIUMPH;
            lfgNormal1stItem   = ITEM_EMBLEM_OF_TRIUMPH;
            dalaranHeroicItem  = ITEM_EMBLEM_OF_TRIUMPH;
            dalaranNormalItem  = ITEM_EMBLEM_OF_TRIUMPH;
            weeklyRaid1stItem  = ITEM_EMBLEM_OF_FROST;
            weeklyRaid2ndItem  = ITEM_EMBLEM_OF_TRIUMPH;
            break;
    }

    // LFG Random Dungeon Rewards
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 2 WHERE `ID` = 24788", lfgHeroic1stItem);
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 2 WHERE `ID` = 24789", lfgHeroicNthItem);
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 2 WHERE `ID` = 24790", lfgNormal1stItem);

    // Dalaran Daily Quests
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 2 WHERE `ID` BETWEEN 13245 AND 13256", dalaranHeroicItem);
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 1 WHERE `ID` IN (13240, 13241, 13243, 13244)", dalaranNormalItem);

    // Weekly Raid Quests (24579..24590) - 5x Primary Emblem + 5x Secondary Emblem
    WorldDatabase.Execute("UPDATE `quest_template` SET `RewardItem1` = {}, `RewardAmount1` = 5, `RewardItem2` = {}, `RewardAmount2` = 5 WHERE `ID` BETWEEN 24579 AND 24590", weeklyRaid1stItem, weeklyRaid2ndItem);

    // Pool 5678 (Weekly raid quest pool) maintenance for current phase
    if (currentPhase < 4)
    {
        WorldDatabase.Execute("DELETE FROM `pool_quest` WHERE `pool_entry` = 5678 AND `entry` = 24590");
    }
    else
    {
        WorldDatabase.Execute("INSERT IGNORE INTO `pool_quest` (`entry`, `pool_entry`, `description`) VALUES (24590, 5678, 'Lord Marrowgar Must Die!')");
    }

    sObjectMgr->LoadQuests();
    sLFGMgr->LoadRewards();

    LOG_INFO("module", "[ProgressiveWotlk] Quest va LFG emblem mukofotlari Phase {} ga yangilandi.", currentPhase);
}

void PW_ApplyPhaseLoot(uint8 currentPhase)
{
    if (currentPhase == 1)
    {
        // 10-man & 25-man Naxxramas, Obsidian Sanctum, Vault of Archavon -> Emblem of Valor (40753)
        WorldDatabase.Execute(
            "UPDATE `creature_loot_template` SET `Item` = 40753 WHERE `Item` IN (47241, 40752) AND `Entry` IN ("
            // 25-man:
            "29249, 29268, 29278, 29615, 29701, 29718, 29940, 29955, 29324, 29373, 29417, 29448, 29991, 30061, "
            "31311, 31535, 31520, 31534, 31722, "
            // 10-man (Variant C):
            "15956, 15953, 15952, 15954, 15936, 16011, 16028, 15931, 15932, 15928, 16061, 16060, 15989, 15990, "
            "28860, 30449, 30451, 30452, 31125)"
        );

        // Barcha 5-man Heroic bosslar -> Emblem of Heroism (40752)
        WorldDatabase.Execute(
            "UPDATE `creature_loot_template` SET `Item` = 40752 WHERE `Item` IN (47241, 40753) AND `Entry` IN ("
            "29932, 30397, 30398, 30510, 30529, 30530, 30532, 30540, 30748, 30774, 30788, 30807, 30810, 31211, "
            "31212, 31215, 31349, 31350, 31360, 31362, 31367, 31368, 31370, 31381, 31384, 31386, 31456, 31463, "
            "31464, 31465, 31469, 31506, 31507, 31508, 31509, 31510, 31511, 31512, 31533, 31536, 31537, 31538, "
            "31558, 31559, 31560, 31610, 31611, 31612, 31656, 31673, 31679, 32313)"
        );

        // Sandiqlar (Gameobjects): 10 & 25-man Raid sandiqlari (Four Horsemen, Malygos Alexstrasza's Gift) -> 40753 (Valor)
        // 5-man Heroic sandiqlar (Eregos 24589, Halls of Stone 26260, 25192) -> 40752 (Heroism)
        WorldDatabase.Execute("UPDATE `gameobject_loot_template` SET `Item` = 40752 WHERE `Item` IN (47241, 40753) AND `Entry` IN (24589, 26260, 25192)");
        WorldDatabase.Execute("UPDATE `gameobject_loot_template` SET `Item` = 40753 WHERE `Item` IN (47241, 40752) AND `Entry` IN (24524, 25193, 26094, 26097)");

        // Sumkalar (item_loot_template): 47241 (Triumph) va 49426 (Frost) ni tozalash
        WorldDatabase.Execute("DELETE FROM `item_loot_template` WHERE `Item` IN (47241, 49426)");
        WorldDatabase.Execute("UPDATE `item_loot_template` SET `Item` = 40752 WHERE `Entry` = 52676 AND `Item` = 47241");
    }
    else if (currentPhase == 2)
    {
        // Ulduar 10 -> Emblem of Valor (40753), Ulduar 25 -> Emblem of Conquest (45624)
        WorldDatabase.Execute(
            "UPDATE `creature_loot_template` SET `Item` = 40753 WHERE `Entry` IN ("
            "32857, 32867, 32927, 33118, 33186, 33271, 33288, 33293, 33515)"
        );
        WorldDatabase.Execute(
            "UPDATE `creature_loot_template` SET `Item` = 45624 WHERE `Entry` IN ("
            "33694, 33693, 33692, 33190, 33724, 33449, 33955, 33885, 34175)"
        );
    }
    else if (currentPhase == 3)
    {
        // Phase 3: 5-man Heroics va ToC -> Emblem of Triumph (47241)
        WorldDatabase.Execute(
            "UPDATE `creature_loot_template` SET `Item` = 47241 WHERE `Item` IN (40752, 40753, 45624) AND `Entry` IN ("
            "15956, 15953, 15952, 15954, 15936, 16011, 16061, 16060, 16028, 15931, 15932, 15928, 15989, 15990, "
            "29249, 29268, 29278, 29615, 29701, 29718, 29940, 29955, 29324, 29373, 29417, 29448, 29991, 30061, "
            "28860, 31311, 30449, 30451, 30452, 31535, 31520, 31534, 31125, 31722, "
            "29932, 30397, 30398, 30510, 30529, 30530, 30532, 30540, 30748, 30774, 30788, 30807, 30810, 31211, "
            "31212, 31215, 31349, 31350, 31360, 31362, 31367, 31368, 31370, 31381, 31384, 31386, 31456, 31463, "
            "31464, 31465, 31469, 31506, 31507, 31508, 31509, 31510, 31511, 31512, 31533, 31536, 31537, 31538, "
            "31558, 31559, 31560, 31610, 31611, 31612, 31656, 31673, 31679, 32313)"
        );
        WorldDatabase.Execute("UPDATE `gameobject_loot_template` SET `Item` = 47241 WHERE `Item` IN (40752, 40753, 45624) AND `Entry` IN (24524, 25193, 26094, 26097, 24589, 26260, 25192)");
    }

    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();

    LOG_INFO("module", "[ProgressiveWotlk] Boss va sandiq loot jadvallari Phase {} ga yangilandi.", currentPhase);
}

void PW_ApplyPhaseArenaSeason(uint8 currentPhase)
{
    uint8 targetSeason = 5;
    switch (currentPhase)
    {
        case 1: targetSeason = 5; break;
        case 2: targetSeason = 6; break;
        case 3: targetSeason = 7; break;
        case 4:
        case 5:
        default: targetSeason = 8; break;
    }

    CharacterDatabase.Execute("UPDATE `active_arena_season` SET `season_id` = {}, `season_state` = 1", targetSeason);
    sArenaSeasonMgr->LoadActiveSeason();
    LOG_INFO("module", "[ProgressiveWotlk] Arena Season {} ga yangilandi.", targetSeason);
}

void PW_ApplyPhaseVendors(uint8 currentPhase)
{
    // 1. Delete items belonging to higher phases
    WorldDatabase.Execute(
        "DELETE nv FROM `npc_vendor` nv "
        "INNER JOIN `npc_vendor_progressive` nvp "
        "ON nv.entry = nvp.entry AND nv.item = nvp.item AND nv.ExtendedCost = nvp.ExtendedCost "
        "WHERE nvp.min_phase > {}", currentPhase
    );

    // 2. Insert items that should be available in current or earlier phases
    WorldDatabase.Execute(
        "INSERT IGNORE INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`) "
        "SELECT `entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild` "
        "FROM `npc_vendor_progressive` WHERE `min_phase` <= {}", currentPhase
    );

    // 3. Update progressive creature templates npcflag
    WorldDatabase.Execute(
        "UPDATE `creature_template` ct "
        "JOIN `creature_template_progressive` ctp ON ct.entry = ctp.entry "
        "SET ct.npcflag = CASE WHEN ctp.min_phase <= {} THEN ctp.original_npcflag ELSE 0 END",
        currentPhase
    );

    // 4. Update creature spawn phaseMask
    WorldDatabase.Execute(
        "UPDATE `creature` c "
        "JOIN `creature_progressive` cp ON c.guid = cp.guid "
        "SET c.phaseMask = CASE WHEN cp.min_phase <= {} THEN 1 ELSE 0 END",
        currentPhase
    );

    sObjectMgr->LoadVendors();
    sObjectMgr->LoadCreatureTemplates();
    sObjectMgr->LoadInstanceEncounters();
    LOG_INFO("module", "[ProgressiveWotlk] Savdogarlar (vendorlar, turlar va kiyimlar) Phase {} ga moslashtirildi.", currentPhase);
}

bool PW_SetPhase(uint8 newPhase, std::string& outMessage)
{
    if (newPhase < MIN_PHASE || newPhase > MAX_PHASE)
    {
        outMessage = Acore::StringFormat("[ProgressiveWotlk] Xato: Faza {} dan {} gacha bo'lishi kerak.", MIN_PHASE, MAX_PHASE);
        return false;
    }

    uint8 oldPhase = PW_GetCurrentPhase();
    if (newPhase == oldPhase)
    {
        outMessage = Acore::StringFormat("[ProgressiveWotlk] Server allaqachon {} fazada.", newPhase);
        return false;
    }

    // 1. Bazani yangilash
    WorldDatabase.Execute("UPDATE `progressive_phase` SET `current_phase` = {}, `activated_at` = NOW()", newPhase);

    // 2. Disables jadvallarini qayta yuklash va qo'llash
    sDisableMgr->LoadDisables();
    PW_ApplyPhaseDisables(newPhase);

    // 3. Quest va Loot mukofotlarini yangilash
    PW_ApplyPhaseQuestRewards(newPhase);
    PW_ApplyPhaseLoot(newPhase);

    // 4. Arena mavsumini yangilash
    PW_ApplyPhaseArenaSeason(newPhase);

    // 5. Savdogarlar (vendorlar) va uzuklarni yangilash
    PW_ApplyPhaseVendors(newPhase);

    // 6. Yangi ochilgan kontent nomini yig'ish
    std::string unlockedNames;
    for (uint8 i = 0; i < PHASE_MAP_TABLE_SIZE; ++i)
    {
        const PhaseMapEntry& entry = PHASE_MAP_TABLE[i];
        if (entry.unlockPhase == newPhase)
        {
            if (!unlockedNames.empty())
                unlockedNames += ", ";
            unlockedNames += entry.mapName;
        }
    }

    if (unlockedNames.empty())
        unlockedNames = "(yangi raid qo'shilmadi)";

    outMessage = Acore::StringFormat(
        "|cff00ff00[Progressive WotLK]|r Faza {} boshlandi! Yangi ochilgan kontent: {}",
        newPhase, unlockedNames
    );

    return true;
}

std::string PW_GetStatusText(uint8 currentPhase)
{
    std::string text = Acore::StringFormat(
        "|cff00ccff[Progressive WotLK] Joriy faza: {}|r\n", currentPhase
    );

    uint8 arenaSeason = 4 + currentPhase;
    if (arenaSeason > 8) arenaSeason = 8;
    const char* seasonName = (arenaSeason == 5) ? "Season 5 (Deadly Gladiator)" :
                             (arenaSeason == 6) ? "Season 6 (Furious Gladiator)" :
                             (arenaSeason == 7) ? "Season 7 (Relentless Gladiator)" : "Season 8 (Wrathful Gladiator)";

    text += Acore::StringFormat("|cffffff00Faol Arena Fasli:|r {}\n", seasonName);

    text += "|cffffff00Ochiq Raidlar & Dungeonlar:|r\n";
    text += "  Naxxramas, Obsidian Sanctum, Eye of Eternity, Vault of Archavon (Archavon)\n";

    for (uint8 i = 0; i < PHASE_MAP_TABLE_SIZE; ++i)
    {
        const PhaseMapEntry& entry = PHASE_MAP_TABLE[i];
        if (entry.unlockPhase <= currentPhase)
            text += Acore::StringFormat("  {} (Phase {} da ochildi)\n", entry.mapName, entry.unlockPhase);
    }

    text += "|cffff4444Yopiq Kontent:|r\n";
    bool anyLocked = false;
    for (uint8 i = 0; i < PHASE_MAP_TABLE_SIZE; ++i)
    {
        const PhaseMapEntry& entry = PHASE_MAP_TABLE[i];
        if (entry.unlockPhase > currentPhase)
        {
            text += Acore::StringFormat("  {} (Phase {} da ochiladi)\n", entry.mapName, entry.unlockPhase);
            anyLocked = true;
        }
    }

    if (currentPhase < 2)
        text += "  Argent Tournament kvestlari (Phase 2 da ochiladi)\n";
    if (currentPhase < 3)
        text += "  Epik toshlar va retseptlar (Phase 3 da ochiladi)\n";

    if (!anyLocked && currentPhase >= 3)
        text += "  (barcha kontent to'liq ochiq!)\n";

    text += "|cffffff00Emblem Mukofotlari:|r\n";
    if (currentPhase == 1)
    {
        text += "  5-man Heroic: Emblem of Heroism\n";
        text += "  10 & 25-man Raids & Daily Heroic: Emblem of Valor\n";
    }
    else if (currentPhase == 2)
    {
        text += "  Daily Heroic & 25-man Ulduar: Emblem of Conquest\n";
        text += "  10-man Ulduar: Emblem of Valor\n";
    }
    else if (currentPhase == 3)
    {
        text += "  ToC & 5-man Heroic & Daily Heroic: Emblem of Triumph\n";
    }
    else
    {
        text += "  ICC & Daily Heroic: Emblem of Frost\n";
    }

    return text;
}

// ============================================================
// WorldScript - Server Startup da to'liq progressiv sozlash
// ============================================================

class ProgressiveWotlkWorldScript : public WorldScript
{
public:
    ProgressiveWotlkWorldScript() : WorldScript("ProgressiveWotlkWorldScript") { }

    void OnStartup() override
    {
        if (!PWConfig::Enable)
        {
            LOG_INFO("module", "[ProgressiveWotlk] Modul o'chirilgan (ProgressiveWotlk.Enable = 0).");
            return;
        }

        uint8 currentPhase = PW_GetCurrentPhase();
        LOG_INFO("module", "[ProgressiveWotlk] Progressive WotLK yuklanmoqda... Joriy faza: {}.", currentPhase);

        PW_ApplyPhaseDisables(currentPhase);
        PW_ApplyPhaseQuestRewards(currentPhase);
        PW_ApplyPhaseLoot(currentPhase);
        PW_ApplyPhaseArenaSeason(currentPhase);
        PW_ApplyPhaseVendors(currentPhase);
    }

    void OnAfterConfigLoad(bool reload) override
    {
        PWConfig::Load(reload);
    }
};

// ============================================================
// PlayerScript - Map va Portalga kirishni himoyalash
// ============================================================

class ProgressiveWotlkPlayerScript : public PlayerScript
{
public:
    ProgressiveWotlkPlayerScript() : PlayerScript("ProgressiveWotlkPlayerScript") { }

    bool OnPlayerCanEnterMap(Player* player, MapEntry const* entry,
                             InstanceTemplate const* /*instance*/,
                             MapDifficulty const* /*mapDiff*/,
                             bool /*loginCheck*/) override
    {
        if (!PWConfig::Enable)
            return true;

        if (player->IsGameMaster() && PWConfig::GMBypass)
            return true;

        uint8 currentPhase = PW_GetCurrentPhase();

        for (uint8 i = 0; i < PHASE_MAP_TABLE_SIZE; ++i)
        {
            const PhaseMapEntry& e = PHASE_MAP_TABLE[i];
            if (e.mapId == entry->MapID && e.unlockPhase > currentPhase)
            {
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffff4444[Progressive WotLK]|r {} hali ochilmagan. (Phase {} da ochiladi)",
                    e.mapName,
                    static_cast<uint32>(e.unlockPhase)
                );
                return false;
            }
        }

        return true;
    }
};

// ============================================================
// AllCreatureScript - Quartermasters va VoA Bosslarini boshqarish
// ============================================================

class ProgressiveWotlkCreatureScript : public AllCreatureScript
{
public:
    ProgressiveWotlkCreatureScript() : AllCreatureScript("ProgressiveWotlkCreatureScript") { }

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (!PWConfig::Enable)
            return;

        uint8 currentPhase = PW_GetCurrentPhase();
        uint32 entry = creature->GetEntry();

        for (uint32 i = 0; i < PHASE_CREATURE_TABLE_SIZE; ++i)
        {
            const PhaseCreatureEntry& pc = PHASE_CREATURE_TABLE[i];
            if (pc.creatureId == entry && pc.unlockPhase > currentPhase)
            {
                creature->SetVisible(false);
                creature->SetPhaseMask(0, true);
                return;
            }
        }
    }

    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (!PWConfig::Enable)
            return false;

        if (player->IsGameMaster() && PWConfig::GMBypass)
            return false;

        uint8 currentPhase = PW_GetCurrentPhase();
        uint32 entry = creature->GetEntry();

        for (uint32 i = 0; i < PHASE_CREATURE_TABLE_SIZE; ++i)
        {
            const PhaseCreatureEntry& pc = PHASE_CREATURE_TABLE[i];
            if (pc.creatureId == entry && pc.unlockPhase > currentPhase)
            {
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffff4444[Progressive WotLK]|r Bu savdogar hali mavjud emas. (Phase {} da ochiladi)",
                    static_cast<uint32>(pc.unlockPhase)
                );
                return true;
            }
        }

        return false;
    }
};

// ============================================================
// AllBattlegroundScript - Battleground Znaklari (Marks of Honor)
// ============================================================

class ProgressiveWotlkBattlegroundScript : public AllBattlegroundScript
{
public:
    ProgressiveWotlkBattlegroundScript() : AllBattlegroundScript("ProgressiveWotlkBattlegroundScript") { }

    void OnBattlegroundEndReward(Battleground* bg, Player* player, TeamId winnerTeamId) override
    {
        if (!PWConfig::Enable || !PWConfig::EnableBgMarks)
            return;

        if (!player || !player->IsInWorld())
            return;

        uint8 currentPhase = PW_GetCurrentPhase();
        if (PWConfig::BgMarksMaxPhase > 0 && currentPhase > PWConfig::BgMarksMaxPhase)
            return;

        uint32 markId = 0;
        switch (bg->GetBgTypeID(true))
        {
            case BATTLEGROUND_WS:
                markId = ITEM_WS_MARK_OF_HONOR; // 20558
                break;
            case BATTLEGROUND_AB:
                markId = ITEM_AB_MARK_OF_HONOR; // 20559
                break;
            case BATTLEGROUND_AV:
                markId = ITEM_AV_MARK_OF_HONOR; // 20560
                break;
            case BATTLEGROUND_EY:
                markId = ITEM_EY_MARK_OF_HONOR; // 29024
                break;
            case BATTLEGROUND_SA:
                markId = ITEM_SA_MARK_OF_HONOR; // 42425
                break;
            case BATTLEGROUND_IC:
                markId = ITEM_IC_MARK_OF_HONOR; // 47395
                break;
            default:
                return;
        }

        // G'olib jamoaga 3 ta, mag'lub yoki durangga 1 ta znak
        uint32 count = (player->GetBgTeamId() == winnerTeamId) ? ITEM_WINNER_COUNT : ITEM_LOSER_COUNT;

        uint32 noSpaceForCount = 0;
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, markId, count, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)
            count -= noSpaceForCount;

        if (count > 0 && !dest.empty())
        {
            Item* item = player->StoreNewItem(dest, markId, true);
            if (item)
                player->SendNewItem(item, count, true, false);
        }

        // Agar sumkada joy bo'lmasa yoki to'lib qolsa, qolganini xavfsiz ravishda pochtaga (mailbox) yuborish
        if (noSpaceForCount > 0)
        {
            player->SendItemRetrievalMail(markId, noSpaceForCount);
        }
    }
};

// ============================================================
// Script Loader
// ============================================================

void AddSC_ProgressiveWotlk()
{
    new ProgressiveWotlkWorldScript();
    new ProgressiveWotlkPlayerScript();
    new ProgressiveWotlkCreatureScript();
    new ProgressiveWotlkBattlegroundScript();
}

void AddSC_ProgressiveWotlkCommands();

void Addmod_progressive_wotlkScripts()
{
    AddSC_ProgressiveWotlk();
    AddSC_ProgressiveWotlkCommands();
}
