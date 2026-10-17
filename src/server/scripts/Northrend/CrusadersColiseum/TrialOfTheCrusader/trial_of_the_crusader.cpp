/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "trial_of_the_crusader.h"
#include "CreatureScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "UnitScript.h"

enum MenuTexts
{
    MSG_TESTED                      = 724001,
    MSG_NEXT_STAGE                  = 724002,
    MSG_CRUSADERS                   = 724003,
    MSG_ANUBARAK                    = 724005,
};

class npc_announcer_toc10 : public CreatureScript
{
public:
    npc_announcer_toc10() : CreatureScript("npc_announcer_toc10") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!creature->HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
            return true;

        InstanceScript* pInstance = creature->GetInstanceScript();
        if (!pInstance)
            return true;

        uint32 gossipTextId = 0;
        switch (pInstance->GetData(TYPE_INSTANCE_PROGRESS))
        {
            case INSTANCE_PROGRESS_INITIAL:
                gossipTextId = MSG_TESTED;
                break;
            case INSTANCE_PROGRESS_INTRO_DONE:
            case INSTANCE_PROGRESS_BEASTS_DEAD:
            case INSTANCE_PROGRESS_FACTION_CHAMPIONS_DEAD:
            case INSTANCE_PROGRESS_VALKYR_DEAD:
                gossipTextId = MSG_NEXT_STAGE;
                break;
            case INSTANCE_PROGRESS_JARAXXUS_DEAD:
                gossipTextId = MSG_CRUSADERS;
                break;
            case INSTANCE_PROGRESS_DONE:
                creature->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);
                return true;
            default:
                return true;
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "We are ready!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1337);
        SendGossipMenuFor(player, gossipTextId, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 uiAction) override
    {
        if (!creature->HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
            return true;

        InstanceScript* pInstance = creature->GetInstanceScript();
        if (!pInstance)
            return true;

        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1337)
        {
            pInstance->SetData(TYPE_ANNOUNCER_GOSSIP_SELECT, 0);
            creature->RemoveNpcFlag(UNIT_NPC_FLAG_GOSSIP);
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

class trial_of_the_crusader_damage_scaling : public UnitScript
{
public:
    trial_of_the_crusader_damage_scaling() : UnitScript("trial_of_the_crusader_damage_scaling", true, { UNITHOOK_MODIFY_HEAL_RECEIVED }) { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage, DamageEffectType /*damagetype*/) override
    {
        if (!attacker || !victim || damage == 0)
            return damage;

        if (attacker->GetMapId() != MAP_TRIAL_OF_THE_CRUSADER)
            return damage;

        if (!attacker->IsCreature() || (!victim->IsPlayer() && !victim->IsPet() && !victim->IsGuardian() && !victim->IsTotem()))
            return damage;

        if (!attacker->ToCreature()->IsHostileToPlayers())
            return damage;

        Map* map = attacker->GetMap();
        if (!map)
            return damage;

        Difficulty diff = map->GetDifficulty();
        if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
            return uint32(damage * 1.20f);
        else if (diff == RAID_DIFFICULTY_25MAN_NORMAL)
            return uint32(damage * 1.25f);

        return damage;
    }

    void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* /*spellInfo*/) override
    {
        if (!healer || !target || heal == 0)
            return;

        if (healer->GetMapId() != MAP_TRIAL_OF_THE_CRUSADER)
            return;

        if (!healer->IsCreature() || !healer->ToCreature()->IsHostileToPlayers())
            return;

        Map* map = healer->GetMap();
        if (!map)
            return;

        Difficulty diff = map->GetDifficulty();
        if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
            heal = uint32(heal * 1.20f);
        else if (diff == RAID_DIFFICULTY_25MAN_NORMAL)
            heal = uint32(heal * 1.25f);
    }
};

void AddSC_trial_of_the_crusader()
{
    new npc_announcer_toc10();
    new trial_of_the_crusader_damage_scaling();
}
