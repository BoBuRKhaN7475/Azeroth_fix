/*
 * mod-progressive-wotlk
 * GM Buyruqlari:
 *
 *   .phase set <1-5>   — Fazani o'zgartirish (GM level 3+ kerak)
 *   .phase status      — Joriy faza va ochiq/yopiq raidlar ro'yxati
 */

#include "ProgressiveWotlk.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Player.h"
#include "RBAC.h"
#include "WorldSessionMgr.h"

using namespace Acore::ChatCommands;

class ProgressiveWotlkCommandScript : public CommandScript
{
public:
    ProgressiveWotlkCommandScript() : CommandScript("ProgressiveWotlkCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable phaseCommandTable =
        {
            // .phase set <N>
            // rbac::RBAC_PERM_COMMAND_EVENT_START (369) — GM level 3 talab qiladi
            { "set",    HandlePhaseSetCommand,    rbac::RBAC_PERM_COMMAND_EVENT_START, Console::Yes },

            // .phase status
            // rbac::RBAC_PERM_COMMAND_EVENT_INFO (367) — GM level 2+ ko'ra oladi
            { "status", HandlePhaseStatusCommand, rbac::RBAC_PERM_COMMAND_EVENT_INFO,  Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "phase", phaseCommandTable },
        };

        return commandTable;
    }

    // --------------------------------------------------------
    // .phase set <1-5>
    // --------------------------------------------------------
    static bool HandlePhaseSetCommand(ChatHandler* handler, uint8 newPhase)
    {
        if (!PWConfig::Enable)
        {
            handler->PSendSysMessage("[ProgressiveWotlk] Modul o'chirilgan (ProgressiveWotlk.Enable = 0).");
            handler->SetSentErrorMessage(true);
            return false;
        }

        std::string message;
        bool success = PW_SetPhase(newPhase, message);

        if (!success)
        {
            handler->SendSysMessage(message);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Server ga e'lon qilish
        if (PWConfig::AnnouncePhaseChange)
            sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, message);
        else
            handler->SendSysMessage(message);

        // Konsolga log
        LOG_INFO("module", "[ProgressiveWotlk] Faza {} ga o'zgartirildi. Buyruq bergan: {}",
                 newPhase,
                 handler->GetSession() ? handler->GetSession()->GetPlayerName() : "Console");

        return true;
    }

    // --------------------------------------------------------
    // .phase status
    // --------------------------------------------------------
    static bool HandlePhaseStatusCommand(ChatHandler* handler)
    {
        if (!PWConfig::Enable)
        {
            handler->PSendSysMessage("[ProgressiveWotlk] Modul o'chirilgan.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint8 currentPhase = PW_GetCurrentPhase();
        std::string statusText = PW_GetStatusText(currentPhase);
        handler->SendSysMessage(statusText);
        return true;
    }
};

void AddSC_ProgressiveWotlkCommands()
{
    new ProgressiveWotlkCommandScript();
}
