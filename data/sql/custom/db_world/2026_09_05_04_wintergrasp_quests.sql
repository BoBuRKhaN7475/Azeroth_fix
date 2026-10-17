-- Task 12: Wintergrasp / Lake Area Quest NPCs Audit
-- Allow all weekly quests to spawn in pools 384 (Horde) and 385 (Alliance)
UPDATE `pool_template` SET `max_limit` = 4 WHERE `entry` IN (384, 385);

-- Fix Primalist Mulfort (31053 - Alliance): remove horde quests (13194, 13201) and assign proper Alliance quest (13156, 13195)
DELETE FROM `creature_queststarter` WHERE `id` = 31053 AND `quest` IN (13194, 13201);
INSERT IGNORE INTO `creature_queststarter` (`id`, `quest`) VALUES
(31053, 13156),
(31053, 13195);

-- Fix Anchorite Tessa (31054 - Alliance): assign proper quests (13194, 13201)
DELETE FROM `creature_queststarter` WHERE `id` = 31054;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(31054, 13194),
(31054, 13201);

DELETE FROM `creature_questender` WHERE `id` = 31054;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(31054, 13194),
(31054, 13201);

