-- Task 4: Disable 40v40 Battlegrounds (Alterac Valley and Isle of Conquest) from queue and random BG
DELETE FROM `disables` WHERE `sourceType` = 3 AND `entry` IN (1, 30);
INSERT INTO `disables` (`sourceType`, `entry`, `flags`, `params_0`, `params_1`, `comment`) VALUES
(3, 1, 0, '', '', 'Disable Battleground: Alterac Valley (40v40)'),
(3, 30, 0, '', '', 'Disable Battleground: Isle of Conquest (40v40)');
