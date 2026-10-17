-- Task 7: Set Mini Gob Manabonk respawn timer to 12 hours (43200 seconds)
UPDATE `creature` SET `spawntimesecs` = 43200 WHERE `id` = 32514;
