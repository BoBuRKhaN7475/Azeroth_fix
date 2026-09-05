-- Fix Spinestalker (37534) and Rimefang (37533) walking animation in air
UPDATE `creature_template_addon` SET `bytes1` = 50331648 WHERE `entry` IN (37533, 37534);

