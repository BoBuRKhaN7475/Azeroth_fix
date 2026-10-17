-- Fix Hunter Pet Base Stats for level 70-80 to match WotLK 3.3.5 / Retail / Circle baseline
-- Level 80 baseline: STR 331, AGI 113, STA 361, INT 65, SPI 109 (Armor 9784, HP 5161)
-- This fixes Hunter Pet Strength (was 192 -> now 331) and Attack Power (was 531 -> now 836)

UPDATE `pet_levelstats` SET `str` = 179, `agi` = 126, `sta` = 312, `inte` = 60, `spi` = 100 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 71;
UPDATE `pet_levelstats` SET `str` = 196, `agi` = 125, `sta` = 318, `inte` = 61, `spi` = 101 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 72;
UPDATE `pet_levelstats` SET `str` = 213, `agi` = 123, `sta` = 323, `inte` = 61, `spi` = 102 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 73;
UPDATE `pet_levelstats` SET `str` = 230, `agi` = 122, `sta` = 329, `inte` = 62, `spi` = 103 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 74;
UPDATE `pet_levelstats` SET `str` = 247, `agi` = 120, `sta` = 334, `inte` = 62, `spi` = 104 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 75;
UPDATE `pet_levelstats` SET `str` = 264, `agi` = 119, `sta` = 340, `inte` = 63, `spi` = 105 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 76;
UPDATE `pet_levelstats` SET `str` = 280, `agi` = 117, `sta` = 345, `inte` = 63, `spi` = 106 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 77;
UPDATE `pet_levelstats` SET `str` = 297, `agi` = 116, `sta` = 350, `inte` = 64, `spi` = 107 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 78;
UPDATE `pet_levelstats` SET `str` = 314, `agi` = 114, `sta` = 356, `inte` = 64, `spi` = 108 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 79;
UPDATE `pet_levelstats` SET `str` = 331, `agi` = 113, `sta` = 361, `inte` = 65, `spi` = 109 WHERE `creature_entry` IN (1, 5058, 5766, 6250) AND `level` = 80;
