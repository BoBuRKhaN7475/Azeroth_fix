-- Crystallized Life (37704) Drop Rate Adjustments

-- 1. Plant corpse herbalism (skinning_loot_template): 15% chance, 4-8 items
UPDATE `skinning_loot_template` 
SET `Chance` = 15, `MinCount` = 4, `MaxCount` = 8 
WHERE `Entry` = 80007 AND `Item` = 37704;

-- 2. Northrend Herbs (gameobject_loot_template): 30% chance, 4-8 items
UPDATE `gameobject_loot_template` 
SET `MinCount` = 4, `MaxCount` = 8 
WHERE `Item` = 37704 AND `Entry` IN (24093, 24224, 24225, 24226, 24227, 24228, 25089, 25093);

-- 3. Mobs (creature_loot_template): Mature Lasher, Unbound Ancient, Living Lasher, Servant of Freya, Mossy Rampager, Ancient Watcher, Corrupted Lothalor Ancient: 3-6 items
UPDATE `creature_loot_template` 
SET `MinCount` = 3, `MaxCount` = 6 
WHERE `Item` = 37704 AND `Entry` IN (26333, 28323, 29036, 30845, 30861, 31229, 34300);
