-- Trial of the Crusader (Map 649) Normal Modes Health Tuning
-- Option 1: 10-man Normal +25% Health, 25-man Normal +30% Health

-- =========================================================================
-- 10-man Normal (+25% Health)
-- =========================================================================

-- Beasts of Northrend
UPDATE `creature_template` SET `HealthModifier` = 200 WHERE `entry` = 34796; -- Gormok the Impaler (was 160)
UPDATE `creature_template` SET `HealthModifier` = 12.5 WHERE `entry` = 34800; -- Snobold Vassal (was 10)
UPDATE `creature_template` SET `HealthModifier` = 112.5 WHERE `entry` = 35144; -- Acidmaw (was 90)
UPDATE `creature_template` SET `HealthModifier` = 112.5 WHERE `entry` = 34799; -- Dreadscale (was 90)
UPDATE `creature_template` SET `HealthModifier` = 312.5 WHERE `entry` = 34797; -- Icehowl (was 250)

-- Lord Jaraxxus
UPDATE `creature_template` SET `HealthModifier` = 375 WHERE `entry` = 34780; -- Lord Jaraxxus (was 300)
UPDATE `creature_template` SET `HealthModifier` = 31.25 WHERE `entry` = 34826; -- Mistress of Pain (was 25)
UPDATE `creature_template` SET `HealthModifier` = 7.5 WHERE `entry` = 34815; -- Felflame Infernal (was 6)
UPDATE `creature_template` SET `HealthModifier` = 1.25 WHERE `entry` = 34825; -- Nether Portal (was 1)
UPDATE `creature_template` SET `HealthModifier` = 1.25 WHERE `entry` = 34813; -- Infernal Volcano (was 1)

-- Faction Champions (Alliance)
UPDATE `creature_template` SET `HealthModifier` = 40 WHERE `entry` IN (
    34460, -- Kavina Grovesong (Druid)
    34461, -- Tyrius Duskblade (DK)
    34463, -- Shaabad (Shaman)
    34465, -- Velanaa (Paladin)
    34466, -- Anthar Forgemender (Priest)
    34467, -- Alyssia Moonstalker (Hunter)
    34468, -- Noozle Whizzlestick (Mage)
    34469, -- Melador Valestrider (Druid)
    34470, -- Saamul (Shaman)
    34471, -- Baelnor Lightbearer (Paladin)
    34472, -- Irieth Shadowstep (Rogue)
    34473, -- Brienna Nightfell (Priest)
    34474, -- Serissa Grimdabbler (Warlock)
    34475  -- Shocuul (Warrior)
); -- was 32

-- Faction Champions (Horde)
UPDATE `creature_template` SET `HealthModifier` = 40 WHERE `entry` IN (
    34441, -- Vivienne Blackwhisper (Priest)
    34444, -- Thrakgar (Shaman)
    34445, -- Liandra Suncaller (Paladin)
    34447, -- Caiphus the Stern (Priest)
    34448, -- Ruj'kah (Hunter)
    34449, -- Ginselle Blightslinger (Mage)
    34450, -- Harkzog (Warlock)
    34451, -- Birana Stormhoof (Druid)
    34453, -- Narrhok Steelbreaker (Warrior)
    34454, -- Maz'dinah (Rogue)
    34455, -- Broln Stouthorn (Shaman)
    34456, -- Malithas Brightblade (Paladin)
    34458, -- Gorgrim Shadowcleave (DK)
    34459  -- Erin Misthoof (Druid)
); -- was 32

-- Faction Champions Pets
UPDATE `creature_template` SET `HealthModifier` = 12.5 WHERE `entry` = 35465; -- Zhaagrym (Warlock Pet, was 10)
UPDATE `creature_template` SET `HealthModifier` = 12.5 WHERE `entry` = 35610; -- Cat (Hunter Pet, was 10)

-- Twin Val'kyr
UPDATE `creature_template` SET `HealthModifier` = 543.75 WHERE `entry` = 34497; -- Fjola Lightbane (was 435)
UPDATE `creature_template` SET `HealthModifier` = 543.75 WHERE `entry` = 34496; -- Eydis Darkbane (was 435)

-- Anub'arak
UPDATE `creature_template` SET `HealthModifier` = 375 WHERE `entry` = 34564; -- Anub'arak (was 300)
UPDATE `creature_template` SET `HealthModifier` = 20.25 WHERE `entry` = 34607; -- Nerubian Burrower (was 16.2)
UPDATE `creature_template` SET `HealthModifier` = 1.875 WHERE `entry` = 34605; -- Swarm Scarab (was 1.5)


-- =========================================================================
-- 25-man Normal (+30% Health)
-- =========================================================================

-- Beasts of Northrend
UPDATE `creature_template` SET `HealthModifier` = 832 WHERE `entry` = 35438; -- Gormok the Impaler (1) (was 640)
UPDATE `creature_template` SET `HealthModifier` = 52 WHERE `entry` = 35441; -- Snobold Vassal (1) (was 40)
UPDATE `creature_template` SET `HealthModifier` = 468 WHERE `entry` = 35511; -- Acidmaw (1) (was 360)
UPDATE `creature_template` SET `HealthModifier` = 468 WHERE `entry` = 35514; -- Dreadscale (1) (was 360)
UPDATE `creature_template` SET `HealthModifier` = 1235 WHERE `entry` = 35447; -- Icehowl (1) (was 950)

-- Lord Jaraxxus
UPDATE `creature_template` SET `HealthModifier` = 1885 WHERE `entry` = 35216; -- Lord Jaraxxus (1) (was 1450)
UPDATE `creature_template` SET `HealthModifier` = 91 WHERE `entry` = 35233; -- Mistress of Pain (1) (was 70)
UPDATE `creature_template` SET `HealthModifier` = 23.4 WHERE `entry` = 35234; -- Felflame Infernal (1) (was 18)
UPDATE `creature_template` SET `HealthModifier` = 1.3 WHERE `entry` = 35278; -- Nether Portal (1) (was 1)
UPDATE `creature_template` SET `HealthModifier` = 1.3 WHERE `entry` = 35265; -- Infernal Volcano (1) (was 1)

-- Faction Champions (Alliance)
UPDATE `creature_template` SET `HealthModifier` = 130 WHERE `entry` IN (
    35702, -- Kavina Grovesong (Druid)
    35743, -- Tyrius Duskblade (DK)
    35734, -- Shaabad (Shaman)
    35746, -- Velanaa (Paladin)
    35665, -- Anthar Forgemender (Priest)
    35662, -- Alyssia Moonstalker (Hunter)
    35721, -- Noozle Whizzlestick (Mage)
    35714, -- Melador Valestrider (Druid)
    35728, -- Saamul (Shaman)
    35668, -- Baelnor Lightbearer (Paladin)
    35699, -- Irieth Shadowstep (Rogue)
    35674, -- Brienna Nightfell (Priest)
    35731, -- Serissa Grimdabbler (Warlock)
    35737  -- Shocuul (Warrior)
); -- was 100

-- Faction Champions (Horde)
UPDATE `creature_template` SET `HealthModifier` = 130 WHERE `entry` IN (
    34442, -- Vivienne Blackwhisper (Priest)
    35740, -- Thrakgar (Shaman)
    35705, -- Liandra Suncaller (Paladin)
    35683, -- Caiphus the Stern (Priest)
    35724, -- Ruj'kah (Hunter)
    35689, -- Ginselle Blightslinger (Mage)
    35695, -- Harkzog (Warlock)
    35671, -- Birana Stormhoof (Druid)
    35718, -- Narrhok Steelbreaker (Warrior)
    35711, -- Maz'dinah (Rogue)
    35680, -- Broln Stouthorn (Shaman)
    35708, -- Malithas Brightblade (Paladin)
    35692, -- Gorgrim Shadowcleave (DK)
    35686  -- Erin Misthoof (Druid)
); -- was 100

-- Faction Champions Pets
UPDATE `creature_template` SET `HealthModifier` = 45.5 WHERE `entry` = 36301; -- Zhaagrym (1) (Warlock Pet, was 35)
UPDATE `creature_template` SET `HealthModifier` = 45.5 WHERE `entry` = 35774; -- Cat (1) (Hunter Pet, was 35)

-- Twin Val'kyr
UPDATE `creature_template` SET `HealthModifier` = 2600 WHERE `entry` = 35350; -- Fjola Lightbane (1) (was 2000)
UPDATE `creature_template` SET `HealthModifier` = 2600 WHERE `entry` = 35347; -- Eydis Darkbane (1) (was 2000)

-- Anub'arak
UPDATE `creature_template` SET `HealthModifier` = 1950 WHERE `entry` = 34566; -- Anub'arak (1) (was 1500)
UPDATE `creature_template` SET `HealthModifier` = 63.18 WHERE `entry` = 34648; -- Nerubian Burrower (1) (was 48.6)
UPDATE `creature_template` SET `HealthModifier` = 11.7 WHERE `entry` = 34650; -- Swarm Scarab (1) (was 9)
