-- Fast Start NPC (Entry: 700200)
DELETE FROM `creature_template` WHERE `entry` = 700200;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `unit_class`, `type`, `ScriptName`)
VALUES (700200, 'Мастер Быстрого Старта', 'Быстрый старт (Fast Start)', 'Interact', 0, 80, 80, 35, 1, 1, 7, 'npc_fast_start');

DELETE FROM `creature_template_model` WHERE `CreatureID` = 700200;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`)
VALUES (700200, 0, 25777, 1.0, 1.0);

DELETE FROM `creature` WHERE `id` = 700200;
-- Dalaran (Krasus Landing)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`)
VALUES (5300801, 700200, 571, 4395, 4395, 1, 1, 5780.0, 620.0, 650.2, 3.14, 300, 0, 0, 100000, 100000, 0, 0, 0, 0);

-- Orgrimmar (Valley of Strength)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`)
VALUES (5300802, 700200, 1, 1637, 1637, 1, 1, 1595.0, -4410.0, 15.8, 4.7, 300, 0, 0, 100000, 100000, 0, 0, 0, 0);

-- Stormwind (Trade District)
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`)
VALUES (5300803, 700200, 0, 1519, 1519, 1, 1, -8840.0, 625.0, 95.0, 3.7, 300, 0, 0, 100000, 100000, 0, 0, 0, 0);
