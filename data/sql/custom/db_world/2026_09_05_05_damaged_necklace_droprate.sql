-- Task 13: Поврежденное ожерелье (Damaged Necklace - 43297) Drop Rate = 1%
UPDATE `reference_loot_template` SET `Chance` = 1.0 WHERE `Item` = 43297 AND `Entry` IN (1267579, 1268083);
