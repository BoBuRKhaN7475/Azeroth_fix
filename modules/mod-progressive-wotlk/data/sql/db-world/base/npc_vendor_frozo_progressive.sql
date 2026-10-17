-- mod-progressive-wotlk
-- Frozo the Renowned (40160) progressive vendor items
-- Phase 2 (Ulduar): Runed Orb (45087)
-- Phase 3 (Trial of the Crusader): Crusader Orb (47556)
-- Phase 4 (Icecrown Citadel): Pattern: Frosty Flying Carpet (54798)

INSERT IGNORE INTO `npc_vendor_progressive` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`, `min_phase`, `comment`) VALUES
(40160, 0, 45087, 0, 0, 2987, 0, 2, 'Frozo the Renowned -> Runed Orb (Phase 2 - Ulduar)'),
(40160, 0, 47556, 0, 0, 2986, 0, 3, 'Frozo the Renowned -> Crusader Orb (Phase 3 - ToC)'),
(40160, 0, 54798, 0, 0, 2986, 0, 4, 'Frozo the Renowned -> Pattern: Frosty Flying Carpet (Phase 4 - ICC)');
