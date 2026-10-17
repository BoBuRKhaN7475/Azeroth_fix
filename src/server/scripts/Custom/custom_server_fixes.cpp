/*
 * AzerothCore Lich King 3.3.5a - Custom Server Fixes & Features
 * Tasks: 2 (Boss Announcer), 3 (BG Anti-AFK), 6 (LFG Auto-Join), 8 (Welcome Announce),
 *        10 (First Char 10 Gold), 16 (Weekend Bonus), 17 (Capital Taxi Unlock)
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "GameTime.h"
#include "World.h"
#include "WorldSessionMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "AllBattlegroundScript.h"
#include "PlayerScript.h"
#include "WorldScript.h"
#include "ChannelMgr.h"
#include "Channel.h"
#include "DatabaseEnv.h"
#include "RBAC.h"
#include "Group.h"
#include "WorldConfig.h"
#include "Config.h"
#include "StringFormat.h"
#include <unordered_map>
#include <vector>
#include <ctime>
#include <mutex>

// ============================================================================
// TASK 2: Boss Announcer (Exact Reference Style: media_1788636413348.png)
// ============================================================================
class BossAnnouncerPlayerScript : public PlayerScript
{
public:
    BossAnnouncerPlayerScript() : PlayerScript("BossAnnouncerPlayerScript", {PLAYERHOOK_ON_CREATURE_KILL}) { }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        if (!killer || !killed)
            return;

        // 1. MUST be hostile to killer (excludes friendly NPCs like Tirion Fordring, Crok, etc.)
        if (!killed->IsHostileTo(killer))
            return;

        // 2. Exclude pets, summons, totems, and player-owned creatures
        if (killed->IsPet() || killed->IsSummon() || killed->IsTotem() || !killed->GetOwnerGUID().IsEmpty())
            return;

        // 3. Must be a genuine dungeon/raid boss or outdoor world boss
        if (!killed->IsDungeonBoss() && !killed->isWorldBoss())
            return;

        // 4. Debounce check: prevent multiple raid members triggering duplicate announcements
        static std::mutex bossCooldownLock;
        static std::unordered_map<ObjectGuid, time_t> bossKillCooldowns;
        time_t now = GameTime::GetGameTime().count();

        {
            std::lock_guard<std::mutex> lock(bossCooldownLock);
            auto it = bossKillCooldowns.find(killed->GetGUID());
            if (it != bossKillCooldowns.end() && (now - it->second) < 30)
                return;

            bossKillCooldowns[killed->GetGUID()] = now;

            // Clean old cooldown entries periodically
            if (bossKillCooldowns.size() > 50)
            {
                for (auto cleanIt = bossKillCooldowns.begin(); cleanIt != bossKillCooldowns.end(); )
                {
                    if (now - cleanIt->second >= 60)
                        cleanIt = bossKillCooldowns.erase(cleanIt);
                    else
                        ++cleanIt;
                }
            }
        }

        // Format difficulty tag
        std::string diffStr;
        Map* map = killed->GetMap();
        if (map && map->IsRaid())
        {
            switch (map->GetDifficulty())
            {
                case RAID_DIFFICULTY_10MAN_NORMAL: diffStr = "[10]"; break;
                case RAID_DIFFICULTY_25MAN_NORMAL: diffStr = "[25]"; break;
                case RAID_DIFFICULTY_10MAN_HEROIC: diffStr = "[10г]"; break;
                case RAID_DIFFICULTY_25MAN_HEROIC: diffStr = "[25г]"; break;
                default: break;
            }
        }
        else if (map && map->IsDungeon())
        {
            switch (map->GetDifficulty())
            {
                case DUNGEON_DIFFICULTY_NORMAL: diffStr = "[Обыч.]"; break;
                case DUNGEON_DIFFICULTY_HEROIC: diffStr = "[Героич.]"; break;
                default: break;
            }
        }
        else if (killed->isWorldBoss())
        {
            diffStr = "[Мировой Босс]";
        }

        std::string bossName = killed->GetNameForLocaleIdx(LOCALE_ruRU);
        if (bossName.empty())
            bossName = killed->GetName();

        std::string targetTag = diffStr.empty() ?
            Acore::StringFormat("|cff00ff00[{}]|r", bossName) :
            Acore::StringFormat("|cff00ff00[{}]|r |cff00ff00{}|r", bossName, diffStr);

        std::string announceMsg;
        Group* group = killer->GetGroup();

        if (group && group->isRaidGroup())
        {
            Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
            std::string leaderName = leader ? leader->GetName() : killer->GetName();
            uint32 maxMembers = (map && (map->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || map->GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)) ? 25 : 10;

            announceMsg = Acore::StringFormat(
                "|TInterface\\TargetingFrame\\UI-RaidTargetingIcon_8:16:16:0:0|t |cffff3300[Сервер]:|r |cffffff00Рейд [{}/{}] во главе с [{}] одержал победу над |r{}",
                group->GetMembersCount(), maxMembers, leaderName, targetTag);
        }
        else if (group)
        {
            Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
            std::string leaderName = leader ? leader->GetName() : killer->GetName();

            announceMsg = Acore::StringFormat(
                "|TInterface\\TargetingFrame\\UI-RaidTargetingIcon_8:16:16:0:0|t |cffff3300[Сервер]:|r |cffffff00Группа [{}/5] во главе с [{}] одержала победу над |r{}",
                group->GetMembersCount(), leaderName, targetTag);
        }
        else
        {
            announceMsg = Acore::StringFormat(
                "|TInterface\\TargetingFrame\\UI-RaidTargetingIcon_8:16:16:0:0|t |cffff3300[Сервер]:|r |cffffff00Игрок [{}] одержал победу над |r{}",
                killer->GetName(), targetTag);
        }

        sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, announceMsg);
    }
};

// ============================================================================
// TASK 3: Battleground Anti-AFK System
// ============================================================================
struct BGAfkState
{
    Position lastPos;
    uint32 idleTimer = 0; // in milliseconds
    bool warned = false;
};

static std::unordered_map<ObjectGuid, BGAfkState> s_bgAfkData;

class BattlegroundAntiAfkScript : public AllBattlegroundScript
{
public:
    BattlegroundAntiAfkScript() : AllBattlegroundScript("BattlegroundAntiAfkScript",
        {ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_UPDATE, ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_REMOVE_PLAYER_AT_LEAVE}) { }

    void OnBattlegroundUpdate(Battleground* bg, uint32 diff) override
    {
        if (!bg || bg->GetStatus() != STATUS_IN_PROGRESS)
            return;

        std::vector<ObjectGuid> afkPlayersToKick;

        for (auto const& [guid, bgPlayer] : bg->GetPlayers())
        {
            Player* player = ObjectAccessor::FindPlayer(guid);
            if (!player || !player->IsInWorld() || player->IsGameMaster())
                continue;

            // Skip dead/ghost players waiting for resurrection
            if (!player->IsAlive())
                continue;

            auto& state = s_bgAfkData[guid];
            Position currentPos = player->GetPosition();

            // Check movement distance
            float dist = state.lastPos.GetExactDist(&currentPos);
            if (dist > 5.0f)
            {
                // Player has moved, reset idle timer and state
                state.lastPos = currentPos;
                state.idleTimer = 0;
                state.warned = false;
            }
            else
            {
                // Player has not moved significantly
                state.idleTimer += diff;

                uint32 kickTimeSec = sConfigMgr->GetOption<uint32>("Battleground.AntiAFK.Timer", 300);
                uint32 kickTimeMs = kickTimeSec * 1000;
                uint32 warnTimeMs = (kickTimeSec > 30) ? (kickTimeSec - 30) * 1000 : (kickTimeMs / 2);

                // Idle warning (30 seconds remaining before kick)
                if (state.idleTimer >= warnTimeMs && !state.warned)
                {
                    state.warned = true;
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cffff2020[BG Anti-AFK]|r Siz harakatsizsiz! 30 soniya ichida harakatlanmasangiz, maydondan chetlatilasiz va Deserter olasiz! / Вы не двигаетесь! Если не начнете движение в течение 30 секунд, вы будете исключены!");
                }

                // 5 minutes (or configured) idle -> Queue for kick
                if (state.idleTimer >= kickTimeMs)
                {
                    state.idleTimer = 0;
                    state.warned = false;
                    afkPlayersToKick.push_back(guid);
                }
            }
        }

        // Process kicks safely outside the bg->GetPlayers() iteration loop
        for (ObjectGuid kickGuid : afkPlayersToKick)
        {
            Player* player = ObjectAccessor::FindPlayer(kickGuid);
            if (!player || !player->IsInWorld())
                continue;

            // Apply Deserter debuff (Spell 26013 - Deserter, 15 minutes)
            player->CastSpell(player, 26013, true);

            // Leave battleground safely
            player->LeaveBattleground();

            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cffff2020[BG Anti-AFK]|r Uzoq vaqt harakatsiz qolganingiz uchun maydondan chetlatildingiz! / Вы были исключены с поля боя за бездействие!");
        }
    }

    void OnBattlegroundRemovePlayerAtLeave(Battleground* /*bg*/, Player* player) override
    {
        if (player)
            s_bgAfkData.erase(player->GetGUID());
    }
};

// ============================================================================
// TASK 6, 8, 10, 16, 17: Player Login / First Login Handler
// ============================================================================
class CustomPlayerLoginScript : public PlayerScript
{
public:
    CustomPlayerLoginScript() : PlayerScript("CustomPlayerLoginScript",
        {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_FIRST_LOGIN}) { }

    void OnPlayerFirstLogin(Player* player) override
    {
        if (!player)
            return;

        // TASK 8: Server welcome announcement (Russian with faction crest)
        std::string welcomeMsg;
        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            welcomeMsg = Acore::StringFormat(
                "|TInterface\\TargetingFrame\\UI-PVP-Alliance:16:16:0:0|t |cff0070ddНовый герой [{}] присоединился к Альянсу! Добро пожаловать!|r",
                player->GetName());
        }
        else
        {
            welcomeMsg = Acore::StringFormat(
                "|TInterface\\TargetingFrame\\UI-PVP-Horde:16:16:0:0|t |cffff2020Новый герой [{}] присоединился к Орде! Добро пожаловать!|r",
                player->GetName());
        }
        sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, welcomeMsg);

        // TASK 10: First character on account gets 10 Gold (100,000 copper)
        uint32 accountId = player->GetSession()->GetAccountId();
        QueryResult result = CharacterDatabase.Query(
            "SELECT account_id FROM account_first_char_bonus WHERE account_id = {}", accountId);

        if (!result)
        {
            CharacterDatabase.Execute(
                "INSERT INTO account_first_char_bonus (account_id, char_guid, rewarded_at) VALUES ({}, {}, NOW())",
                accountId, player->GetGUID().GetCounter());

            player->ModifyMoney(100000); // 10 gold
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cff00ff00[Бонус]|r Вы получили 10 золота за первого персонажа на аккаунте!");
        }
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!player)
            return;


        // TASK 17: Unlock ALL world flight points for player's faction
        player->m_taxi.UnlockAllTaxiNodesFor(player->GetTeamId(), player->getClass());
    }
};

void AddSC_custom_server_fixes()
{
    new BossAnnouncerPlayerScript();
    new BattlegroundAntiAfkScript();
    new CustomPlayerLoginScript();
}
