-- mod-progressive-wotlk
-- Progressive tables definition

CREATE TABLE IF NOT EXISTS `npc_vendor_progressive` (
    `entry` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
    `slot` SMALLINT NOT NULL DEFAULT '0',
    `item` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
    `maxcount` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
    `incrtime` INT UNSIGNED NOT NULL DEFAULT '0',
    `ExtendedCost` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
    `VerifiedBuild` SMALLINT NOT NULL DEFAULT '0',
    `min_phase` TINYINT UNSIGNED NOT NULL DEFAULT '1',
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`entry`, `item`, `ExtendedCost`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `creature_template_progressive` (
    `entry` MEDIUMINT UNSIGNED NOT NULL DEFAULT '0',
    `min_phase` TINYINT UNSIGNED NOT NULL DEFAULT '1',
    `original_npcflag` INT UNSIGNED NOT NULL DEFAULT '0',
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `creature_progressive` (
    `guid` INT UNSIGNED NOT NULL DEFAULT '0',
    `min_phase` TINYINT UNSIGNED NOT NULL DEFAULT '1',
    `comment` VARCHAR(255) NOT NULL DEFAULT '',
    PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Frozo the Renowned (40160) progressive vendor items
-- Phase 2 (Ulduar): Runed Orb (45087)
-- Phase 3 (Trial of the Crusader): Crusader Orb (47556)
-- Phase 4 (Icecrown Citadel): Pattern: Frosty Flying Carpet (54798)

INSERT IGNORE INTO `npc_vendor_progressive` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`, `VerifiedBuild`, `min_phase`, `comment`) VALUES
(40160, 0, 45087, 0, 0, 2987, 0, 2, 'Frozo the Renowned -> Runed Orb (Phase 2 - Ulduar)'),
(40160, 0, 47556, 0, 0, 2986, 0, 3, 'Frozo the Renowned -> Crusader Orb (Phase 3 - ToC)'),
(40160, 0, 54798, 0, 0, 2986, 0, 4, 'Frozo the Renowned -> Pattern: Frosty Flying Carpet (Phase 4 - ICC)');
