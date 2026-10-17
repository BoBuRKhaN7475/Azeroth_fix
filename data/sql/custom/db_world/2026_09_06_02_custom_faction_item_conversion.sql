-- Task 19: Cross-Faction Item Conversion World Table, NPC & Spawns

CREATE TABLE IF NOT EXISTS `custom_faction_item_conversion` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `alliance_item_id` INT UNSIGNED NOT NULL,
  `horde_item_id` INT UNSIGNED NOT NULL,
  `item_level` SMALLINT UNSIGNED NOT NULL,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `comment` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_alliance_item` (`alliance_item_id`),
  UNIQUE KEY `idx_horde_item` (`horde_item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Populate verified ToC / ToGC 637 pairs
INSERT IGNORE INTO `custom_faction_item_conversion` (`alliance_item_id`, `horde_item_id`, `item_level`, `enabled`, `comment`)
SELECT 
    f.alliance_id,
    f.horde_id,
    ita.ItemLevel,
    1,
    CONCAT(ita.name, ' <-> ', ith.name)
FROM player_factionchange_items f
JOIN item_template ita ON f.alliance_id = ita.entry
JOIN item_template ith ON f.horde_id = ith.entry
WHERE ita.ItemLevel IN (232, 245, 258, 272) AND ita.Quality = 4;

-- NPC 700100: Archmage Rhynold (Faction Item Transmuter)
DELETE FROM `creature_template` WHERE `entry` = 700100;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES 
(700100, 0, 0, 0, 0, 0, 'Archmage Rhynold', 'Faction Item Transmuter', NULL, 0, 80, 80, 0, 35, 1, 1, 1.14286, 1, 1, 20, 1, 0, 1, 2000, 2000, 1, 1, 8, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 'npc_faction_item_converter', 51831);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 700100;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES 
(700100, 0, 25619, 1, 1, 51831);

-- Spawns: Dalaran Eventide & Crusaders' Coliseum grounds
DELETE FROM `creature` WHERE `id` = 700100;
INSERT INTO `creature` (`id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `dynamicflags`, `ScriptName`, `VerifiedBuild`, `CreateObject`, `Comment`) VALUES 
(700100, 571, 4395, 4395, 1, 1, 0, 5773.5, 620.0, 650.175, 3.14, 120, 0, 0, 100000, 100000, 0, 1, 0, 0, 'npc_faction_item_converter', 51831, 0, 'Archmage Rhynold - Dalaran'),
(700100, 571, 210, 4676, 1, 1, 0, 8445.0, 705.0, 547.4, 4.2, 120, 0, 0, 100000, 100000, 0, 1, 0, 0, 'npc_faction_item_converter', 51831, 0, 'Archmage Rhynold - Crusaders Coliseum');
