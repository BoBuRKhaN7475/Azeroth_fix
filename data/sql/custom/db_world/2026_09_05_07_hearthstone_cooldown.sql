-- Task 18: Hearthstone Cooldown 15 Minutes (900000 ms)
DELETE FROM `spell_cooldown_overrides` WHERE `Id` = 8690;
INSERT INTO `spell_cooldown_overrides` (`Id`, `RecoveryTime`, `CategoryRecoveryTime`, `StartRecoveryTime`, `StartRecoveryCategory`, `Comment`)
VALUES (8690, 900000, 900000, 0, 0, 'Hearthstone 15 min cooldown');
