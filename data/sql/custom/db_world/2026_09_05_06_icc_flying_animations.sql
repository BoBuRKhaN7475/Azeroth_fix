-- Task 14 & 15: Fix Sister Svalna (37126) and Spire Frostwyrm (37230) flying animations
UPDATE `creature_template_addon` SET `bytes1` = 50331648 WHERE `entry` IN (37126, 37230);
