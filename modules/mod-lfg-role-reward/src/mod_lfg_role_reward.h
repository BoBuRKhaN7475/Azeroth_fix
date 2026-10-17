#ifndef MOD_LFG_ROLE_REWARD_H
#define MOD_LFG_ROLE_REWARD_H

#include "Common.h"
#include "Config.h"
#include "Player.h"
#include "LFGMgr.h"
#include "LFG.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "WorldSessionMgr.h"
#include "World.h"
#include "Log.h"
#include <string>

enum DungeonMode : uint8
{
    MODE_NONE = 0,
    MODE_HEROIC = 1,
    MODE_NORMAL = 2
};

struct LfgRoleRewardConfig
{
    static inline bool Enable = true;
    static inline bool AnnounceOnLogin = true;
    static inline bool RequireRandomGroup = false;
    static inline bool CheckSpecMatch = false;

    // Heroic Mode Tank (Default: 40752 Emblem of Heroism, 25 Gold, 43347 Satchel of Spoils)
    static inline bool TankEnable = true;
    static inline uint32 TankItemId = 43347;
    static inline uint32 TankItemCount = 1;
    static inline uint32 TankMoney = 250000;
    static inline uint32 TankExtraEmblemId = 40752;
    static inline uint32 TankExtraEmblemCount = 1;

    // Heroic Mode Healer (Default: 40752 Emblem of Heroism, 15 Gold, 43347 Satchel of Spoils)
    static inline bool HealerEnable = true;
    static inline uint32 HealerItemId = 43347;
    static inline uint32 HealerItemCount = 1;
    static inline uint32 HealerMoney = 150000;
    static inline uint32 HealerExtraEmblemId = 40752;
    static inline uint32 HealerExtraEmblemCount = 1;

    // Normal Mode Settings
    static inline bool NormalEnable = true;
    static inline bool NormalLevel80Only = true;

    // Normal Mode Tank (Default: 40752 Emblem of Heroism, 12 Gold, 43347 Satchel of Spoils)
    static inline bool NormalTankEnable = true;
    static inline uint32 NormalTankItemId = 43347;
    static inline uint32 NormalTankItemCount = 1;
    static inline uint32 NormalTankMoney = 120000;
    static inline uint32 NormalTankExtraEmblemId = 40752;
    static inline uint32 NormalTankExtraEmblemCount = 1;

    // Normal Mode Healer (Default: 40752 Emblem of Heroism, 8 Gold, 43347 Satchel of Spoils)
    static inline bool NormalHealerEnable = true;
    static inline uint32 NormalHealerItemId = 43347;
    static inline uint32 NormalHealerItemCount = 1;
    static inline uint32 NormalHealerMoney = 80000;
    static inline uint32 NormalHealerExtraEmblemId = 40752;
    static inline uint32 NormalHealerExtraEmblemCount = 1;

    // DPS
    static inline bool DPSEnable = false;
    static inline uint32 DPSItemId = 0;
    static inline uint32 DPSItemCount = 0;
    static inline uint32 DPSMoney = 0;
    static inline uint32 DPSExtraEmblemId = 0;
    static inline uint32 DPSExtraEmblemCount = 0;

    static void Load()
    {
        Enable = sConfigMgr->GetOption<bool>("LfgRoleReward.Enable", true, false);
        AnnounceOnLogin = sConfigMgr->GetOption<bool>("LfgRoleReward.AnnounceOnLogin", true, false);
        RequireRandomGroup = sConfigMgr->GetOption<bool>("LfgRoleReward.RequireRandomGroup", false, false);
        CheckSpecMatch = sConfigMgr->GetOption<bool>("LfgRoleReward.CheckSpecMatch", false, false);

        // Heroic Tank
        TankEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.Tank.Enable", true, false);
        TankItemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Tank.ItemId", 43347, false);
        TankItemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Tank.ItemCount", 1, false);
        TankMoney = sConfigMgr->GetOption<uint32>("LfgRoleReward.Tank.Money", 250000, false);
        TankExtraEmblemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Tank.ExtraEmblemId", 40752, false);
        TankExtraEmblemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Tank.ExtraEmblemCount", 1, false);

        // Heroic Healer
        HealerEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.Healer.Enable", true, false);
        HealerItemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Healer.ItemId", 43347, false);
        HealerItemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Healer.ItemCount", 1, false);
        HealerMoney = sConfigMgr->GetOption<uint32>("LfgRoleReward.Healer.Money", 150000, false);
        HealerExtraEmblemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Healer.ExtraEmblemId", 40752, false);
        HealerExtraEmblemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Healer.ExtraEmblemCount", 1, false);

        // Normal Settings
        NormalEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.Normal.Enable", true, false);
        NormalLevel80Only = sConfigMgr->GetOption<bool>("LfgRoleReward.Normal.Level80Only", true, false);

        // Normal Tank
        NormalTankEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.Normal.Tank.Enable", true, false);
        NormalTankItemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Tank.ItemId", 43347, false);
        NormalTankItemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Tank.ItemCount", 1, false);
        NormalTankMoney = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Tank.Money", 120000, false);
        NormalTankExtraEmblemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Tank.ExtraEmblemId", 40752, false);
        NormalTankExtraEmblemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Tank.ExtraEmblemCount", 1, false);

        // Normal Healer
        NormalHealerEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.Normal.Healer.Enable", true, false);
        NormalHealerItemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Healer.ItemId", 43347, false);
        NormalHealerItemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Healer.ItemCount", 1, false);
        NormalHealerMoney = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Healer.Money", 80000, false);
        NormalHealerExtraEmblemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Healer.ExtraEmblemId", 40752, false);
        NormalHealerExtraEmblemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.Normal.Healer.ExtraEmblemCount", 1, false);

        // DPS
        DPSEnable = sConfigMgr->GetOption<bool>("LfgRoleReward.DPS.Enable", false, false);
        DPSItemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.DPS.ItemId", 0, false);
        DPSItemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.DPS.ItemCount", 0, false);
        DPSMoney = sConfigMgr->GetOption<uint32>("LfgRoleReward.DPS.Money", 0, false);
        DPSExtraEmblemId = sConfigMgr->GetOption<uint32>("LfgRoleReward.DPS.ExtraEmblemId", 0, false);
        DPSExtraEmblemCount = sConfigMgr->GetOption<uint32>("LfgRoleReward.DPS.ExtraEmblemCount", 0, false);
    }

    static DungeonMode GetQuestDungeonMode(uint32 questId)
    {
        // 80 Heroic Random Daily (First / Other): 24788 / 24789
        if (questId == 24788 || questId == 24789)
            return MODE_HEROIC;

        if (NormalEnable)
        {
            // 80 Normal Random Daily: 24790 / 24791
            if (questId == 24790 || questId == 24791)
                return MODE_NORMAL;

            if (!NormalLevel80Only)
            {
                // Lower level random dungeons (15-70): 24881 - 24896
                if (questId >= 24881 && questId <= 24896)
                    return MODE_NORMAL;

                // TBC Level 70 Normal / Heroic: 24922 / 24923
                if (questId == 24922 || questId == 24923)
                    return MODE_NORMAL;

                // Holiday event bosses: 25482 - 25485
                if (questId >= 25482 && questId <= 25485)
                    return MODE_NORMAL;
            }
        }

        return MODE_NONE;
    }
};

// 100% crash-safe delivery function for items, money, and emblems
inline void DeliverRoleReward(Player* player, uint32 itemId, uint32 itemCount, uint32 money, uint32 emblemId, uint32 emblemCount, std::string const& roleTitle, std::string const& colorHex, std::string const& dungeonType = "ПП гер")
{
    if (!player || !player->IsInWorld() || !player->GetSession())
        return;

    ChatHandler chat(player->GetSession());

    // 1. Money reward
    if (money > 0)
    {
        player->ModifyMoney(static_cast<int32>(money));
    }

    // 2. Main item reward (e.g. Satchel of Spoils)
    if (itemId > 0 && itemCount > 0)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (itemTemplate)
        {
            if (!player->AddItem(itemId, itemCount))
            {
                // Bags full -> safely send to mailbox via The Postmaster
                player->SendItemRetrievalMail(itemId, itemCount);
                chat.PSendSysMessage("|cffffcc00[LFG Bonus]|r Sumkangizda bo'sh joy bo'lmagani sababli mukofotingiz pochta qutingizga yuborildi!");
            }
        }
        else
        {
            LOG_ERROR("module", "[mod-lfg-role-reward] Item ID {} bazada mavjud emas! Config faylni tekshiring.", itemId);
        }
    }

    // 3. Extra Emblem reward (e.g. Emblem of Heroism)
    if (emblemId > 0 && emblemCount > 0)
    {
        ItemTemplate const* emblemTemplate = sObjectMgr->GetItemTemplate(emblemId);
        if (emblemTemplate)
        {
            if (!player->AddItem(emblemId, emblemCount))
            {
                player->SendItemRetrievalMail(emblemId, emblemCount);
            }
        }
        else
        {
            LOG_ERROR("module", "[mod-lfg-role-reward] Emblem ID {} bazada mavjud emas! Config faylni tekshiring.", emblemId);
        }
    }

    // Chat announcement to the player
    chat.PSendSysMessage("|cff{}[LFG Bonus]|r Siz guruhda |cffffffff{}|r sifatida {}ni muvaffaqiyatli yakunladingiz va maxsus rol bonusiga ega bo'ldingiz!",
        colorHex, roleTitle, dungeonType);
}

#endif
