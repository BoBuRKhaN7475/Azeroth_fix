/*
 * mod-lfg-role-reward
 * AzerothCore module to grant Call to Arms style rewards to Tanks and Healers
 * completing Random Heroic / Random Dungeons via Dungeon Finder.
 */

#include "mod_lfg_role_reward.h"
#include "CommandScript.h"
#include "RBAC.h"

using namespace Acore::ChatCommands;

class LfgRoleRewardWorldScript : public WorldScript
{
public:
    LfgRoleRewardWorldScript() : WorldScript("LfgRoleRewardWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LfgRoleRewardConfig::Load();
        LOG_INFO("module", "[mod-lfg-role-reward] Modul sozlamalari yuklandi. Enabled: {}, NormalEnabled: {}",
            LfgRoleRewardConfig::Enable, LfgRoleRewardConfig::NormalEnable);
    }
};

class LfgRoleRewardPlayerScript : public PlayerScript
{
public:
    LfgRoleRewardPlayerScript() : PlayerScript("LfgRoleRewardPlayerScript") { }

    void OnPlayerLogin(Player* player) override
    {
        if (!LfgRoleRewardConfig::Enable || !LfgRoleRewardConfig::AnnounceOnLogin || !player || !player->GetSession())
            return;

        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cff00ff00[LFG Bonus]|r Serverda Tank va Xillar uchun ПП гер va ПП обычка maxsus mukofotlari faol!");
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        if (!LfgRoleRewardConfig::Enable || !player || !player->IsInWorld() || !player->GetSession() || !quest)
            return;

        DungeonMode mode = LfgRoleRewardConfig::GetQuestDungeonMode(quest->GetQuestId());
        if (mode == MODE_NONE)
            return;

        // Check random group requirement if enabled in config
        if (LfgRoleRewardConfig::RequireRandomGroup && sLFGMgr->GetRandomPlayersCount(player->GetGUID()) == 0)
            return;

        uint8 roles = sLFGMgr->GetRoles(player->GetGUID());
        std::string dungeonTypeName = (mode == MODE_HEROIC) ? "ПП гер" : "ПП обычка";

        // 1. Check Tank role
        bool tankEnabled = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankEnable : LfgRoleRewardConfig::NormalTankEnable;
        if ((roles & lfg::PLAYER_ROLE_TANK) && tankEnabled)
        {
            if (!LfgRoleRewardConfig::CheckSpecMatch || player->HasTankSpec())
            {
                uint32 itemId = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankItemId : LfgRoleRewardConfig::NormalTankItemId;
                uint32 itemCount = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankItemCount : LfgRoleRewardConfig::NormalTankItemCount;
                uint32 money = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankMoney : LfgRoleRewardConfig::NormalTankMoney;
                uint32 emblemId = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankExtraEmblemId : LfgRoleRewardConfig::NormalTankExtraEmblemId;
                uint32 emblemCount = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::TankExtraEmblemCount : LfgRoleRewardConfig::NormalTankExtraEmblemCount;

                DeliverRoleReward(player, itemId, itemCount, money, emblemId, emblemCount, "Tank", "00ccff", dungeonTypeName);
                return;
            }
        }

        // 2. Check Healer role
        bool healerEnabled = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerEnable : LfgRoleRewardConfig::NormalHealerEnable;
        if ((roles & lfg::PLAYER_ROLE_HEALER) && healerEnabled)
        {
            if (!LfgRoleRewardConfig::CheckSpecMatch || player->HasHealSpec())
            {
                uint32 itemId = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerItemId : LfgRoleRewardConfig::NormalHealerItemId;
                uint32 itemCount = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerItemCount : LfgRoleRewardConfig::NormalHealerItemCount;
                uint32 money = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerMoney : LfgRoleRewardConfig::NormalHealerMoney;
                uint32 emblemId = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerExtraEmblemId : LfgRoleRewardConfig::NormalHealerExtraEmblemId;
                uint32 emblemCount = (mode == MODE_HEROIC) ? LfgRoleRewardConfig::HealerExtraEmblemCount : LfgRoleRewardConfig::NormalHealerExtraEmblemCount;

                DeliverRoleReward(player, itemId, itemCount, money, emblemId, emblemCount, "Healer (Shifokor)", "00ff00", dungeonTypeName);
                return;
            }
        }

        // 3. Check DPS role (optional)
        if ((roles & lfg::PLAYER_ROLE_DAMAGE) && LfgRoleRewardConfig::DPSEnable)
        {
            DeliverRoleReward(player,
                LfgRoleRewardConfig::DPSItemId,
                LfgRoleRewardConfig::DPSItemCount,
                LfgRoleRewardConfig::DPSMoney,
                LfgRoleRewardConfig::DPSExtraEmblemId,
                LfgRoleRewardConfig::DPSExtraEmblemCount,
                "DPS",
                "ff9900",
                dungeonTypeName);
            return;
        }
    }
};

class LfgRoleRewardCommandScript : public CommandScript
{
public:
    LfgRoleRewardCommandScript() : CommandScript("LfgRoleRewardCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable testNormalSubCommands =
        {
            { "tank", HandleTestNormalTankCommand, rbac::RBAC_PERM_COMMAND_EVENT_START, Console::No },
            { "heal", HandleTestNormalHealCommand, rbac::RBAC_PERM_COMMAND_EVENT_START, Console::No },
        };

        static ChatCommandTable testSubCommands =
        {
            { "tank",   HandleTestTankCommand,       rbac::RBAC_PERM_COMMAND_EVENT_START, Console::No },
            { "heal",   HandleTestHealCommand,       rbac::RBAC_PERM_COMMAND_EVENT_START, Console::No },
            { "normal", testNormalSubCommands },
        };

        static ChatCommandTable lfgRewardSubCommands =
        {
            { "test",   testSubCommands },
            { "reload", HandleReloadCommand,         rbac::RBAC_PERM_COMMAND_EVENT_START, Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "lfgreward", lfgRewardSubCommands },
        };
        return commandTable;
    }

    static bool HandleReloadCommand(ChatHandler* handler)
    {
        LfgRoleRewardConfig::Load();
        handler->PSendSysMessage("|cff00ff00[mod-lfg-role-reward]|r Sozlamalar muvaffaqiyatli qayta yuklandi (Reloaded)!");
        return true;
    }

    static bool HandleTestTankCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->PSendSysMessage("[mod-lfg-role-reward] Ushbu buyruq faqat o'yin ichida o'yinchi tomonidan ishlatilishi mumkin.");
            return false;
        }

        DeliverRoleReward(player,
            LfgRoleRewardConfig::TankItemId,
            LfgRoleRewardConfig::TankItemCount,
            LfgRoleRewardConfig::TankMoney,
            LfgRoleRewardConfig::TankExtraEmblemId,
            LfgRoleRewardConfig::TankExtraEmblemCount,
            "Tank [TEST ПП ГЕР]",
            "00ccff",
            "ПП гер");

        return true;
    }

    static bool HandleTestHealCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->PSendSysMessage("[mod-lfg-role-reward] Ushbu buyruq faqat o'yin ichida o'yinchi tomonidan ishlatilishi mumkin.");
            return false;
        }

        DeliverRoleReward(player,
            LfgRoleRewardConfig::HealerItemId,
            LfgRoleRewardConfig::HealerItemCount,
            LfgRoleRewardConfig::HealerMoney,
            LfgRoleRewardConfig::HealerExtraEmblemId,
            LfgRoleRewardConfig::HealerExtraEmblemCount,
            "Healer [TEST ПП ГЕР]",
            "00ff00",
            "ПП гер");

        return true;
    }

    static bool HandleTestNormalTankCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->PSendSysMessage("[mod-lfg-role-reward] Ushbu buyruq faqat o'yin ichida o'yinchi tomonidan ishlatilishi mumkin.");
            return false;
        }

        DeliverRoleReward(player,
            LfgRoleRewardConfig::NormalTankItemId,
            LfgRoleRewardConfig::NormalTankItemCount,
            LfgRoleRewardConfig::NormalTankMoney,
            LfgRoleRewardConfig::NormalTankExtraEmblemId,
            LfgRoleRewardConfig::NormalTankExtraEmblemCount,
            "Tank [TEST ПП ОБЫЧКА]",
            "00ccff",
            "ПП обычка");

        return true;
    }

    static bool HandleTestNormalHealCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
        {
            handler->PSendSysMessage("[mod-lfg-role-reward] Ushbu buyruq faqat o'yin ichida o'yinchi tomonidan ishlatilishi mumkin.");
            return false;
        }

        DeliverRoleReward(player,
            LfgRoleRewardConfig::NormalHealerItemId,
            LfgRoleRewardConfig::NormalHealerItemCount,
            LfgRoleRewardConfig::NormalHealerMoney,
            LfgRoleRewardConfig::NormalHealerExtraEmblemId,
            LfgRoleRewardConfig::NormalHealerExtraEmblemCount,
            "Healer [TEST ПП ОБЫЧКА]",
            "00ff00",
            "ПП обычка");

        return true;
    }
};

void Addmod_lfg_role_rewardScripts()
{
    new LfgRoleRewardWorldScript();
    new LfgRoleRewardPlayerScript();
    new LfgRoleRewardCommandScript();
}
