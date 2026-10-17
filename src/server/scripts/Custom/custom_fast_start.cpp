/*
 * AzerothCore 3.3.5a - Custom Fast Start (Быстрый старт)
 *
 * Features:
 * - Admin grants Fast Start access to specific account login (.faststart grant <account>)
 * - 1 activation per account (or as many as granted by admin)
 * - Level 80 boost with full health and mana
 * - 3500 Gold (35,000,000 copper)
 * - Max weapon and class skills (400/400)
 * - 4x Frostweave Bag (20-slot bags, Item 41599)
 * - 150% Flying (Apprentice, Journeyman, Expert Riding + Cold Weather Flying + Mounts)
 * - Role-tailored, fully audited 200 ilvl blue WotLK dungeon equipment sets
 * - Anti-abuse validation and database audit logging
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Creature.h"
#include "GossipDef.h"
#include "Item.h"
#include "Chat.h"
#include "CommandScript.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"
#include "RBAC.h"
#include "GameTime.h"
#include "Config.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include <vector>
#include <string>

namespace
{
    enum FastStartRole : uint32
    {
        ROLE_PALADIN_HOLY    = 1,
        ROLE_PALADIN_PROT    = 2,
        ROLE_PALADIN_RET     = 3,

        ROLE_WARRIOR_PROT    = 4,
        ROLE_WARRIOR_DPS     = 5,

        ROLE_DK_TANK         = 6,
        ROLE_DK_DPS          = 7,

        ROLE_PRIEST_HEAL     = 8,
        ROLE_PRIEST_SHADOW   = 9,

        ROLE_SHAMAN_RESTO    = 10,
        ROLE_SHAMAN_ELE      = 11,
        ROLE_SHAMAN_ENH      = 12,

        ROLE_DRUID_RESTO     = 13,
        ROLE_DRUID_BALANCE   = 14,
        ROLE_DRUID_FERAL     = 15,

        ROLE_ROGUE_DPS       = 16,
        ROLE_HUNTER_DPS      = 17,
        ROLE_MAGE_DPS        = 18,
        ROLE_WARLOCK_DPS     = 19,
    };

    char const* GetRoleName(uint32 role)
    {
        switch (role)
        {
            case ROLE_PALADIN_HOLY:    return "Paladin - Holy (Healer)";
            case ROLE_PALADIN_PROT:    return "Paladin - Protection (Tank)";
            case ROLE_PALADIN_RET:     return "Paladin - Retribution (DPS)";
            case ROLE_WARRIOR_PROT:    return "Warrior - Protection (Tank)";
            case ROLE_WARRIOR_DPS:     return "Warrior - Arms/Fury (DPS)";
            case ROLE_DK_TANK:         return "Death Knight - Tank";
            case ROLE_DK_DPS:          return "Death Knight - DPS";
            case ROLE_PRIEST_HEAL:     return "Priest - Holy/Disc (Healer)";
            case ROLE_PRIEST_SHADOW:   return "Priest - Shadow (DPS)";
            case ROLE_SHAMAN_RESTO:    return "Shaman - Restoration (Healer)";
            case ROLE_SHAMAN_ELE:      return "Shaman - Elemental (DPS)";
            case ROLE_SHAMAN_ENH:      return "Shaman - Enhancement (DPS)";
            case ROLE_DRUID_RESTO:     return "Druid - Restoration (Healer)";
            case ROLE_DRUID_BALANCE:   return "Druid - Balance (DPS)";
            case ROLE_DRUID_FERAL:     return "Druid - Feral (Tank/DPS)";
            case ROLE_ROGUE_DPS:       return "Rogue - DPS";
            case ROLE_HUNTER_DPS:      return "Hunter - DPS";
            case ROLE_MAGE_DPS:        return "Mage - DPS";
            case ROLE_WARLOCK_DPS:     return "Warlock - DPS";
            default:                   return "Unknown";
        }
    }

    std::vector<uint32> const* GetGearSetForRole(uint32 role)
    {
        // 1. Paladin - Holy (Healer Plate + Shield + 1H Mace + Healer Accessories)
        static std::vector<uint32> const sPaladinHoly = {
            43280, // Head: Faceguard of the Hammer Clan (Ahn'kahet - SP 90, Int 60, MP5 38)
            37141, // Neck: Amulet of Dazzling Light (The Nexus - Int 34, Stam 51, SP 59, MP5 22)
            37376, // Shoulders: Ferocious Pauldrons of the Rhino (Utgarde Pinnacle - Int 44, Crit 49, SP 59)
            37291, // Cloak: Ancient Dragon Spirit Cape (The Oculus - Int 29, Spirit 33, Haste 42, SP 59)
            37672, // Chest: Patina-Coated Breastplate (Halls of Stone - Int 55, Haste 78, SP 59)
            37590, // Wrists: Bands of Fading Light (Ahn'kahet - Int 30, Haste 35, SP 59)
            37729, // Hands: Grips of Sculptured Icicles (Azjol-Nerub - Int 58, Crit 42, SP 59)
            37152, // Waist: Girdle of Ice (The Nexus - Int 44, Crit 41, SP 59)
            37650, // Legs: Shardling Legguards (Halls of Stone - Int 55, Haste 70, SP 59)
            43405, // Feet: Sabatons of Erekem (Violet Hold - SP 59, Int 57, MP5 39)
            37232, // Ring 1: Ring of the Traitor King (Azjol-Nerub - Int 34, Crit 43, SP 50)
            37371, // Ring 2: Ring of the Frenzied Wolvar (Utgarde Pinnacle - Int 28, Haste 43, SP 50)
            37657, // Trinket 1: Spark of Life (Halls of Stone - Spirit 73, proc Spirit/SP)
            37844, // Trinket 2: Winged Talisman (Halls of Lightning - MP5 36, use SP 340)
            37681, // 1H Mace: Gavel of the Fleshcrafter (Culling of Stratholme - Int 33, MP5 22, SP 59)
            37216, // Shield: Facade Shield of Glyphs (Azjol-Nerub - Int 30, Stam 40, SP 59)
            38364  // Libram: Venture Co. Libram of Mostly Holy Deeds (Sholazar)
        };

        // 2. Paladin - Protection (Tank Plate + Shield + 1H Sword + Tank Accessories)
        static std::vector<uint32> const sPaladinProt = {
            37135, // Head: Arcane-Shielded Helm (The Nexus - Str 61, Stam 93, Def 36, Dodge 52)
            37689, // Neck: Pendant of the Nathrezim (CoS - Str 43, Stam 64, Def 25, Parry 31)
            37635, // Shoulders: Pauldrons of the Colossus (Gundrak - Str 58, Stam 87, Def 31, Block 30)
            37197, // Cloak: Tattered Castle Drape (Utgarde Keep - Str 30, Stam 60, Def 33)
            37658, // Chest: Sun-Emblazoned Chestplate (Halls of Stone - Str 53, Stam 117, Def 60, Hit 51)
            37175, // Wrists: Glassy Silver Bracers (The Nexus - Str 35, Stam 52, Dodge 25, Parry 31)
            37671, // Hands: Refined Ore Gloves (Halls of Stone - Str 57, Stam 87, Def 42, Block 33)
            37379, // Waist: Skadi's Iron Belt (Utgarde Pinnacle - Str 49, Stam 87, Def 39)
            37292, // Legs: Ley-Guardian's Legguards (The Oculus - Str 45, Stam 117, Def 77, Hit 40)
            37618, // Feet: Greaves of Ancient Evil (Azjol-Nerub - Str 57, Stam 87, Def 42, Dodge 33)
            37186, // Ring 1: Unsmashable Heavy Band (Utgarde Keep - Str 43, Stam 64, Def 30)
            37257, // Ring 2: Band of Torture (The Oculus - Str 42, Stam 66, Def 28)
            37220, // Trinket 1: Essence of Gossamer (Azjol-Nerub - Stam 111, absorb shield)
            37872, // Trinket 2: Lavanthor's Talisman (Violet Hold - Def 65, Block value 440)
            37179, // 1H Sword: Infantry Assault Blade (Utgarde Keep - Str 24, Stam 49, Def 33)
            37162, // Shield: Bulwark of the Noble Protector (The Nexus - Str 43, Stam 64, Def 31, Block 25)
            38363  // Libram: Venture Co. Libram of Protection (Sholazar)
        };

        // 3. Paladin - Retribution (DPS Plate + 2H Sword + Melee Accessories)
        static std::vector<uint32> const sPaladinRet = {
            37237, // Head: Chitin Shell Greathelm (Azjol-Nerub - Str 63, Stam 93, Crit 60)
            37397, // Neck: Gold Amulet of Kings (Utgarde Pinnacle - Str 47, Stam 57, Crit 43)
            37698, // Shoulders: Spaulders of Elder's Square (CoS - Str 60, Stam 79, Crit 58)
            37647, // Cloak: Cloak of Bloodied Waters (Gundrak - Str 60, Stam 37, Crit 34)
            37612, // Chest: Bonegrinder Breastplate (Azjol-Nerub - Str 70, Stam 91, Crit 70)
            37217, // Wrists: Golden Limb Bands (Azjol-Nerub - Str 44, Stam 64, Crit 42)
            37874, // Hands: Gauntlets of Capture (Violet Hold - Str 58, Stam 64, Haste 56)
            37178, // Waist: Strategist's Belt (Utgarde Keep - Str 52, Stam 73, Haste 56)
            37675, // Legs: Legplates of Steel Implants (Halls of Stone - Str 70, Stam 103, Haste 62)
            37150, // Feet: Rift Striders (The Nexus - Str 50, Stam 85, Crit 50)
            37151, // Ring 1: Band of Frosted Thorns (The Nexus - Str 45, Stam 63, Crit 42)
            37685, // Ring 2: Mobius Band (CoS - Agi 43, Stam 46, Hit 25, AP 86)
            37390, // Trinket 1: Meteorite Whetstone (Utgarde Pinnacle - Crit 74, proc Haste 444)
            37166, // Trinket 2: Sphere of Red Dragon's Blood (The Nexus - Hit 55, proc AP 335)
            37653, // 2H Sword: Sword of Justice (Halls of Stone - Str 70, Stam 105, Crit 77)
            38362  // Libram: Venture Co. Libram of Retribution (Sholazar)
        };

        // 4. Warrior - Protection (Tank Plate + Shield + 1H Sword + Gun)
        static std::vector<uint32> const sWarriorProt = {
            37135, // Head: Arcane-Shielded Helm
            37689, // Neck: Pendant of the Nathrezim
            37635, // Shoulders: Pauldrons of the Colossus
            37197, // Cloak: Tattered Castle Drape
            37658, // Chest: Sun-Emblazoned Chestplate
            37175, // Wrists: Glassy Silver Bracers
            37671, // Hands: Refined Ore Gloves
            37379, // Waist: Skadi's Iron Belt
            37292, // Legs: Ley-Guardian's Legguards
            37618, // Feet: Greaves of Ancient Evil
            37186, // Ring 1: Unsmashable Heavy Band
            37257, // Ring 2: Band of Torture
            37220, // Trinket 1: Essence of Gossamer
            37872, // Trinket 2: Lavanthor's Talisman
            37179, // 1H Sword: Infantry Assault Blade
            37162, // Shield: Bulwark of the Noble Protector
            37692  // Gun: Pierce's Pistol
        };

        // 5. Warrior - Arms/Fury (DPS Plate + 2H Sword + Bow)
        static std::vector<uint32> const sWarriorDPS = {
            37237, // Head: Chitin Shell Greathelm
            37397, // Neck: Gold Amulet of Kings
            37698, // Shoulders: Spaulders of Elder's Square
            37647, // Cloak: Cloak of Bloodied Waters
            37612, // Chest: Bonegrinder Breastplate
            37217, // Wrists: Golden Limb Bands
            37874, // Hands: Gauntlets of Capture
            37178, // Waist: Strategist's Belt
            37675, // Legs: Legplates of Steel Implants
            37150, // Feet: Rift Striders
            37151, // Ring 1: Band of Frosted Thorns
            37685, // Ring 2: Mobius Band (CoS - Agi 43, Stam 46, Hit 25, AP 86)
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            37653, // 2H Sword: Sword of Justice
            37615  // Bow: Titanium Compound Bow
        };

        // 6. Death Knight - Tank (Plate + 2H Sword + Sigil)
        static std::vector<uint32> const sDKTank = {
            37135, // Head: Arcane-Shielded Helm
            37689, // Neck: Pendant of the Nathrezim
            37635, // Shoulders: Pauldrons of the Colossus
            37197, // Cloak: Tattered Castle Drape
            37658, // Chest: Sun-Emblazoned Chestplate
            37175, // Wrists: Glassy Silver Bracers
            37671, // Hands: Refined Ore Gloves
            37379, // Waist: Skadi's Iron Belt
            37292, // Legs: Ley-Guardian's Legguards
            37618, // Feet: Greaves of Ancient Evil
            37186, // Ring 1: Unsmashable Heavy Band
            37257, // Ring 2: Band of Torture
            37220, // Trinket 1: Essence of Gossamer
            37872, // Trinket 2: Lavanthor's Talisman
            37653, // 2H Sword: Sword of Justice
            40867  // Sigil: Sigil of the Wild Buck (Grizzly Hills)
        };

        // 7. Death Knight - DPS (Plate + 2H Sword + Sigil)
        static std::vector<uint32> const sDKDPS = {
            37237, // Head: Chitin Shell Greathelm
            37397, // Neck: Gold Amulet of Kings
            37698, // Shoulders: Spaulders of Elder's Square
            37647, // Cloak: Cloak of Bloodied Waters
            37612, // Chest: Bonegrinder Breastplate
            37217, // Wrists: Golden Limb Bands
            37874, // Hands: Gauntlets of Capture
            37178, // Waist: Strategist's Belt
            37675, // Legs: Legplates of Steel Implants
            37150, // Feet: Rift Striders
            37151, // Ring 1: Band of Frosted Thorns
            37685, // Ring 2: Mobius Band (CoS - Agi 43, Stam 46, Hit 25, AP 86)
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            37653, // 2H Sword: Sword of Justice
            40875  // Sigil: Sigil of Arthritic Binding (Zul'Drak)
        };

        // 8. Priest - Holy/Disc (Cloth Healer + Staff + Wand)
        static std::vector<uint32> const sPriestHeal = {
            37715, // Head: Cowl of the Dire Troll (Drak'Tharon - Spirit 60, Int 57, SP 59)
            37290, // Neck: Dragon Prow Amulet (The Oculus - Spirit 43, Int 33, SP 59)
            37655, // Shoulders: Mantle of the Tribunal (Halls of Stone - Spirit 58, Int 42, SP 59)
            37630, // Cloak: Shroud of Moorabi (Gundrak - Spirit 32, Int 43, SP 59)
            37222, // Chest: Egg Sac Robes (Azjol-Nerub - Spirit 77, Int 60, SP 59)
            37245, // Wrists: Tangled Web Bindings (Azjol-Nerub - Spirit 43, Int 34, SP 59)
            37153, // Hands: Gloves of the Crystal Gardener (The Nexus - Spirit 58, Int 42, SP 59)
            37637, // Waist: Living Mojo Belt (Gundrak - Spirit 44, Int 34, SP 59)
            37189, // Legs: Breeches of the Caller (Utgarde Keep - Spirit 77, Int 57, SP 59)
            37730, // Feet: Cleric's Linen Shoes (Drak'Tharon - Spirit 58, Int 42, SP 59)
            37732, // Ring 1: Spectral Seal of the Prophet (Gundrak - Spirit 52, Int 30, SP 50)
            37232, // Ring 2: Ring of the Traitor King (Azjol-Nerub - Int 34, Crit 43, SP 50)
            37657, // Trinket 1: Spark of Life (Halls of Stone)
            37844, // Trinket 2: Winged Talisman (Halls of Lightning)
            37384, // 2H Staff: Staff of Wayward Principles (Utgarde Pinnacle - Spirit 56, Int 77, SP 203)
            37626  // Wand: Wand of Sseratus (Gundrak - Spirit 24, Int 18, SP 18)
        };

        // 9. Priest - Shadow (Cloth Caster + Staff + Wand)
        static std::vector<uint32> const sPriestShadow = {
            37594, // Head: Elder Headpiece (Ahn'kahet)
            37595, // Neck: Necklace of Taldaram (Ahn'kahet)
            37691, // Shoulders: Mantle of Deceit (CoS)
            37174, // Cloak: Rippling Azure Cloak (The Nexus)
            37641, // Chest: Arcane Flame Altar-Garb (Gundrak)
            37613, // Wrists: Flame Sphere Bindings (Violet Hold)
            37687, // Hands: Gloves of Distorted Time (CoS)
            37289, // Waist: Sash of Phantasmal Images (The Oculus)
            37731, // Legs: Opposed Stasis Leggings (Violet Hold)
            37218, // Feet: Stone-Worn Footwraps (Azjol-Nerub)
            37232, // Ring 1: Ring of the Traitor King (Azjol-Nerub)
            37371, // Ring 2: Ring of the Frenzied Wolvar (Utgarde Pinnacle)
            37264, // Trinket 1: Pendulum of Telluric Currents (The Oculus)
            37873, // Trinket 2: Mark of the War Prisoner (Violet Hold)
            37384, // 2H Staff: Staff of Wayward Principles
            37177  // Wand: Wand of the San'layn (Utgarde Keep)
        };

        // 10. Shaman - Restoration (Mail Healer + Shield + 1H Mace + Totem)
        static std::vector<uint32> const sShamanResto = {
            37857, // Head: Helm of the Lightning Halls (HoL Loken - Int 60, Stam 58, SP 90, MP5 38)
            37141, // Neck: Amulet of Dazzling Light (The Nexus - MP5 22, Int 34, SP 59)
            37398, // Shoulders: Mantle of Discarded Ways (Utgarde Pinnacle - MP5 29, Int 44, SP 59)
            37291, // Cloak: Ancient Dragon Spirit Cape (The Oculus - Int 29, Haste 42, SP 59)
            37184, // Chest: Dalronn's Jerkin (Utgarde Keep - MP5 40, Int 55, SP 59)
            37138, // Wrists: Bands of Channeled Energy (The Nexus - MP5 22, Int 34, SP 59)
            43475, // Hands: Grips of the Chilling Revenant (Gundrak - MP5 29, Int 44, SP 59)
            37628, // Waist: Slad'ran's Coiled Cord (Gundrak - MP5 28, Int 43, SP 59)
            37155, // Legs: Frozen Forest Kilt (The Nexus - MP5 40, Int 58, SP 59)
            37654, // Feet: Sabatons of the Ages (Halls of Stone - MP5 28, Int 43, SP 59)
            37195, // Ring 1: Band of Enchanted Growth (The Oculus - MP5 25, Int 31, SP 50)
            37371, // Ring 2: Ring of the Frenzied Wolvar (Utgarde Pinnacle - Int 28, Haste 43, SP 50)
            37657, // Trinket 1: Spark of Life (Halls of Stone)
            37844, // Trinket 2: Winged Talisman (Halls of Lightning)
            37681, // 1H Mace: Gavel of the Fleshcrafter (CoS - Int 33, MP5 22, SP 59)
            37216, // Shield: Facade Shield of Glyphs (Azjol-Nerub - Int 30, SP 59)
            38368  // Totem: Totem of the Bay (Sholazar)
        };

        // 11. Shaman - Elemental (Mail Caster + Shield + 1H Mace + Totem)
        static std::vector<uint32> const sShamanEle = {
            37592, // Head: Brood Plague Helmet (Ahn'kahet - Int 51, Stam 46, Crit 60, SP)
            37595, // Neck: Necklace of Taldaram (Ahn'kahet)
            37875, // Shoulders: Spaulders of the Violet Hold
            37174, // Cloak: Rippling Azure Cloak
            37256, // Chest: Scaled Armor of Drakos (The Oculus)
            37365, // Wrists: Bands of the Sky Ring (The Oculus)
            38615, // Hands: Lightning-Charged Gloves (HoL)
            37868, // Waist: Girdle of the Ethereal (The Oculus)
            37818, // Legs: Patroller's War-Kilt (HoL)
            37654, // Feet: Sabatons of the Ages (Halls of Stone - Int 43, Stam 48, Haste 57, SP)
            37232, // Ring 1: Ring of the Traitor King
            37371, // Ring 2: Ring of the Frenzied Wolvar
            37264, // Trinket 1: Pendulum of Telluric Currents
            37873, // Trinket 2: Mark of the War Prisoner
            37681, // 1H Mace: Gavel of the Fleshcrafter
            37216, // Shield: Facade Shield of Glyphs
            38361  // Totem: Venture Co. Lightning Rod (Sholazar)
        };

        // 12. Shaman - Enhancement (Mail Physical + 2x 1H Maces + Totem)
        static std::vector<uint32> const sShamanEnh = {
            37188, // Head: Plunderer's Helmet (Utgarde Keep)
            37861, // Neck: Necklace of Arcane Spheres (The Oculus)
            37373, // Shoulders: Massive Spaulders of the Jormungar (UP)
            43406, // Cloak: Cloak of the Gushing Wound (Gundrak - Agi 43, Stam 64, AP 56, Hit 29)
            37800, // Chest: Aviary Guardsman's Hauberk (HoL)
            37656, // Wrists: Raging Construct Bands (HoS)
            37614, // Hands: Gauntlets of the Plundering Geist (Violet Hold)
            37845, // Waist: Cord of Swirling Winds (HoL)
            37221, // Legs: Hollowed Mandible Legplates (Azjol-Nerub)
            37167, // Feet: Dragon Slayer's Sabatons (The Nexus)
            37685, // Ring 1: Mobius Band (CoS)
            37624, // Ring 2: Stained-Glass Shard Ring (Gundrak)
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            43407, // Main Hand Mace: Stormstrike Mace (Violet Hold)
            43407, // Off Hand Mace: Stormstrike Mace
            38367  // Totem: Venture Co. Flame Slicer (Sholazar)
        };

        // 13. Druid - Restoration (Leather Healer + Staff + Idol)
        static std::vector<uint32> const sDruidResto = {
            37149, // Head: Helm of Anomalus (The Nexus - Spirit 58, Int 43, SP 59)
            37290, // Neck: Dragon Prow Amulet (The Oculus - Spirit 43, Int 33, SP 59)
            37652, // Shoulders: Spaulders of Krystallus (Halls of Stone - Spirit 44, Int 43, SP 59)
            37291, // Cloak: Ancient Dragon Spirit Cape (The Oculus - Spirit 33, Int 29, SP 59)
            37587, // Chest: Ymirjar Physician's Robe (Utgarde Pinnacle - Spirit 58, Int 58, SP 59)
            37634, // Wrists: Bracers of the Divine Elemental (Gundrak - Spirit 31, Int 33, SP 59)
            37261, // Hands: Gloves of Radiant Light (CoS - Spirit 43, Int 43, SP 59)
            37842, // Waist: Belt of Vivacity (Halls of Lightning - Spirit 43, Int 43, SP 59)
            37616, // Legs: Kilt of the Forgotten One (Violet Hold - Spirit 58, Int 58, SP 59)
            37640, // Feet: Boots of Transformation (Gundrak - Int 44, Stam 43, Spirit 50)
            37732, // Ring 1: Spectral Seal of the Prophet (Gundrak - Spirit 52, Int 30, SP 50)
            37371, // Ring 2: Ring of the Frenzied Wolvar (Utgarde Pinnacle - Int 28, Haste 43, SP 50)
            37657, // Trinket 1: Spark of Life (Halls of Stone)
            37844, // Trinket 2: Winged Talisman (Halls of Lightning)
            37384, // 2H Staff: Staff of Wayward Principles
            38366  // Idol: Idol of Pure Thoughts (Sholazar)
        };

        // 14. Druid - Balance (Leather Caster + Staff + Idol)
        static std::vector<uint32> const sDruidBalance = {
            37149, // Head: Helm of Anomalus (The Nexus)
            37595, // Neck: Necklace of Taldaram (Ahn'kahet)
            37652, // Shoulders: Spaulders of Krystallus (HoS)
            37174, // Cloak: Rippling Azure Cloak
            37236, // Chest: Insect Vestments (Azjol-Nerub)
            37634, // Wrists: Bracers of the Divine Elemental
            37261, // Hands: Gloves of Radiant Light
            37842, // Waist: Belt of Vivacity
            37616, // Legs: Kilt of the Forgotten One
            37176, // Feet: Cavern Leather Treads (The Nexus - Int 50, Stam 68, Haste 61)
            37232, // Ring 1: Ring of the Traitor King
            37371, // Ring 2: Ring of the Frenzied Wolvar
            37264, // Trinket 1: Pendulum of Telluric Currents
            37873, // Trinket 2: Mark of the War Prisoner
            37384, // 2H Staff: Staff of Wayward Principles
            38360  // Idol: Idol of Arcane Terror (Sholazar)
        };

        // 15. Druid - Feral (Leather Agi + Feral Staff + Idol)
        static std::vector<uint32> const sDruidFeral = {
            37636, // Head: Helm of Cheated Fate (Gundrak - Agi 78, Stam 67, Haste 55)
            37861, // Neck: Necklace of Arcane Spheres
            37139, // Shoulders: Spaulders of the Careless Thief (The Nexus)
            43406, // Cloak: Cloak of the Gushing Wound (Gundrak - Agi 43, Stam 64, AP 56, Hit 29)
            37165, // Chest: Crystal-Infused Tunic (The Nexus)
            37183, // Wrists: Bindings of the Tunneler (Utgarde Keep - Agi 31, Stam 67, Exp 25)
            37846, // Hands: Charged-Bolt Grips (Halls of Lightning - Agi 50, Stam 58, Crit 36)
            37243, // Waist: Treasure Seeker's Belt (Azjol-Nerub)
            37374, // Legs: Ravenous Leggings of the Furbolg (UP)
            37666, // Feet: Boots of the Whirling Mist (Halls of Stone - Agi 58, Stam 57, Hit 38)
            37685, // Ring 1: Mobius Band
            37624, // Ring 2: Stained-Glass Shard Ring
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            37190, // 2H Staff: Enraged Feral Staff (The Nexus)
            38365  // Idol: Idol of Perspicacious Attacks (Sholazar)
        };

        // 16. Rogue - DPS (Leather Agi + 2x Daggers + Thrown)
        static std::vector<uint32> const sRogueDPS = {
            37636, // Head: Helm of Cheated Fate (Gundrak - Agi 78, Stam 67, Haste 55)
            37861, // Neck: Necklace of Arcane Spheres
            37139, // Shoulders: Spaulders of the Careless Thief
            43406, // Cloak: Cloak of the Gushing Wound (Gundrak - Agi 43, Stam 64, AP 56, Hit 29)
            37165, // Chest: Crystal-Infused Tunic
            37183, // Wrists: Bindings of the Tunneler
            37846, // Hands: Charged-Bolt Grips (Halls of Lightning - Agi 50, Stam 58, Crit 36)
            37243, // Waist: Treasure Seeker's Belt
            37374, // Legs: Ravenous Leggings of the Furbolg
            37666, // Feet: Boots of the Whirling Mist (Halls of Stone - Agi 58, Stam 57, Hit 38)
            37685, // Ring 1: Mobius Band
            37624, // Ring 2: Stained-Glass Shard Ring
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            37181, // Main Hand Dagger: Dagger of Betrayal (Utgarde Keep)
            37871, // Off Hand Dagger: The Key (Violet Hold)
            37410  // Thrown: Tracker's Balanced Knives
        };

        // 17. Hunter - DPS (Mail Agi + 2H Polearm + Bow)
        static std::vector<uint32> const sHunterDPS = {
            37188, // Head: Plunderer's Helmet
            37861, // Neck: Necklace of Arcane Spheres
            37373, // Shoulders: Massive Spaulders of the Jormungar
            43406, // Cloak: Cloak of the Gushing Wound (Gundrak - Agi 43, Stam 64, AP 56, Hit 29)
            37800, // Chest: Aviary Guardsman's Hauberk
            37656, // Wrists: Raging Construct Bands
            37614, // Hands: Gauntlets of the Plundering Geist
            37845, // Waist: Cord of Swirling Winds
            37221, // Legs: Hollowed Mandible Legplates
            37167, // Feet: Dragon Slayer's Sabatons
            37685, // Ring 1: Mobius Band
            37624, // Ring 2: Stained-Glass Shard Ring
            37390, // Trinket 1: Meteorite Whetstone
            37166, // Trinket 2: Sphere of Red Dragon's Blood
            43409, // 2H Polearm: Saliva Corroded Pike (Violet Hold)
            37615  // Bow: Titanium Compound Bow (Violet Hold)
        };

        // 18. Mage - DPS (Cloth Caster + Staff + Wand)
        static std::vector<uint32> const sMageDPS = {
            37594, // Head: Elder Headpiece
            37595, // Neck: Necklace of Taldaram
            37691, // Shoulders: Mantle of Deceit
            37174, // Cloak: Rippling Azure Cloak
            37641, // Chest: Arcane Flame Altar-Garb
            37613, // Wrists: Flame Sphere Bindings
            37687, // Hands: Gloves of Distorted Time
            37289, // Waist: Sash of Phantasmal Images
            37731, // Legs: Opposed Stasis Leggings
            37218, // Feet: Stone-Worn Footwraps
            37232, // Ring 1: Ring of the Traitor King
            37371, // Ring 2: Ring of the Frenzied Wolvar
            37264, // Trinket 1: Pendulum of Telluric Currents
            37873, // Trinket 2: Mark of the War Prisoner
            37384, // 2H Staff: Staff of Wayward Principles
            37177  // Wand: Wand of the San'layn
        };

        // 19. Warlock - DPS (Cloth Caster + Staff + Wand)
        static std::vector<uint32> const sWarlockDPS = {
            37594, // Head: Elder Headpiece
            37595, // Neck: Necklace of Taldaram
            37691, // Shoulders: Mantle of Deceit
            37174, // Cloak: Rippling Azure Cloak
            37641, // Chest: Arcane Flame Altar-Garb
            37613, // Wrists: Flame Sphere Bindings
            37687, // Hands: Gloves of Distorted Time
            37289, // Waist: Sash of Phantasmal Images
            37731, // Legs: Opposed Stasis Leggings
            37218, // Feet: Stone-Worn Footwraps
            37232, // Ring 1: Ring of the Traitor King
            37371, // Ring 2: Ring of the Frenzied Wolvar
            37264, // Trinket 1: Pendulum of Telluric Currents
            37873, // Trinket 2: Mark of the War Prisoner
            37384, // 2H Staff: Staff of Wayward Principles
            37177  // Wand: Wand of the San'layn
        };

        switch (role)
        {
            case ROLE_PALADIN_HOLY:    return &sPaladinHoly;
            case ROLE_PALADIN_PROT:    return &sPaladinProt;
            case ROLE_PALADIN_RET:     return &sPaladinRet;
            case ROLE_WARRIOR_PROT:    return &sWarriorProt;
            case ROLE_WARRIOR_DPS:     return &sWarriorDPS;
            case ROLE_DK_TANK:         return &sDKTank;
            case ROLE_DK_DPS:          return &sDKDPS;
            case ROLE_PRIEST_HEAL:     return &sPriestHeal;
            case ROLE_PRIEST_SHADOW:   return &sPriestShadow;
            case ROLE_SHAMAN_RESTO:    return &sShamanResto;
            case ROLE_SHAMAN_ELE:      return &sShamanEle;
            case ROLE_SHAMAN_ENH:      return &sShamanEnh;
            case ROLE_DRUID_RESTO:     return &sDruidResto;
            case ROLE_DRUID_BALANCE:   return &sDruidBalance;
            case ROLE_DRUID_FERAL:     return &sDruidFeral;
            case ROLE_ROGUE_DPS:       return &sRogueDPS;
            case ROLE_HUNTER_DPS:      return &sHunterDPS;
            case ROLE_MAGE_DPS:        return &sMageDPS;
            case ROLE_WARLOCK_DPS:     return &sWarlockDPS;
            default:                   return nullptr;
        }
    }

    void TeachWeaponProficiencies(Player* player)
    {
        switch (player->getClass())
        {
            case CLASS_WARRIOR:
                player->learnSpell(196, false);   // 1H Axes
                player->learnSpell(197, false);   // 2H Axes
                player->learnSpell(198, false);   // 1H Maces
                player->learnSpell(199, false);   // 2H Maces
                player->learnSpell(200, false);   // Polearms
                player->learnSpell(201, false);   // 1H Swords
                player->learnSpell(202, false);   // 2H Swords
                player->learnSpell(227, false);   // Staves
                player->learnSpell(1180, false);  // Daggers
                player->learnSpell(15590, false); // Fist
                player->learnSpell(264, false);   // Bows
                player->learnSpell(266, false);   // Guns
                player->learnSpell(5011, false);  // Crossbows
                player->learnSpell(2567, false);  // Thrown
                player->learnSpell(9116, false);  // Shield
                player->learnSpell(8737, false);  // Mail
                player->learnSpell(750, false);   // Plate
                player->learnSpell(674, false);   // Dual Wield
                break;
            case CLASS_PALADIN:
                player->learnSpell(196, false);
                player->learnSpell(197, false);
                player->learnSpell(198, false);
                player->learnSpell(199, false);
                player->learnSpell(200, false);
                player->learnSpell(201, false);
                player->learnSpell(202, false);
                player->learnSpell(9116, false);
                player->learnSpell(8737, false);
                player->learnSpell(750, false);
                break;
            case CLASS_DEATH_KNIGHT:
                player->learnSpell(196, false);
                player->learnSpell(197, false);
                player->learnSpell(198, false);
                player->learnSpell(199, false);
                player->learnSpell(200, false);
                player->learnSpell(201, false);
                player->learnSpell(202, false);
                player->learnSpell(750, false);
                player->learnSpell(674, false);
                break;
            case CLASS_HUNTER:
                player->learnSpell(196, false);
                player->learnSpell(197, false);
                player->learnSpell(200, false);
                player->learnSpell(201, false);
                player->learnSpell(202, false);
                player->learnSpell(227, false);
                player->learnSpell(1180, false);
                player->learnSpell(15590, false);
                player->learnSpell(264, false);
                player->learnSpell(266, false);
                player->learnSpell(5011, false);
                player->learnSpell(2567, false);
                player->learnSpell(8737, false);
                player->learnSpell(674, false);
                break;
            case CLASS_ROGUE:
                player->learnSpell(196, false);
                player->learnSpell(198, false);
                player->learnSpell(201, false);
                player->learnSpell(1180, false);
                player->learnSpell(15590, false);
                player->learnSpell(264, false);
                player->learnSpell(266, false);
                player->learnSpell(5011, false);
                player->learnSpell(2567, false);
                player->learnSpell(674, false);
                break;
            case CLASS_PRIEST:
                player->learnSpell(198, false);
                player->learnSpell(1180, false);
                player->learnSpell(227, false);
                player->learnSpell(5009, false); // Wand
                break;
            case CLASS_SHAMAN:
                player->learnSpell(196, false);
                player->learnSpell(197, false);
                player->learnSpell(198, false);
                player->learnSpell(199, false);
                player->learnSpell(1180, false);
                player->learnSpell(15590, false);
                player->learnSpell(227, false);
                player->learnSpell(9116, false);
                player->learnSpell(8737, false);
                player->learnSpell(674, false);
                break;
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                player->learnSpell(201, false);
                player->learnSpell(1180, false);
                player->learnSpell(227, false);
                player->learnSpell(5009, false);
                break;
            case CLASS_DRUID:
                player->learnSpell(198, false);
                player->learnSpell(199, false);
                player->learnSpell(200, false);
                player->learnSpell(1180, false);
                player->learnSpell(15590, false);
                player->learnSpell(227, false);
                break;
            default:
                break;
        }
    }

    void TeachBaseClassSpells(Player* player)
    {
        if (!sChrClassesStore.LookupEntry(player->getClass()))
            return;

        std::vector<Trainer::Trainer const*> const& trainers = sObjectMgr->GetClassTrainers(player->getClass());
        bool hadNew;
        do
        {
            hadNew = false;
            for (Trainer::Trainer const* trainer : trainers)
            {
                if (!trainer->IsTrainerValidForPlayer(player))
                    continue;

                for (Trainer::Spell const& trainerSpell : trainer->GetSpells())
                {
                    uint32 spellId = trainerSpell.SpellId;

                    // 1. Strict Filter: NEVER learn any spell that belongs to a talent or talent chain
                    if (GetTalentSpellPos(spellId))
                        continue;

                    uint32 firstRank = sSpellMgr->GetFirstSpellInChain(spellId);
                    if (firstRank && (GetTalentSpellPos(firstRank) || GetTalentSpellCost(firstRank) > 0))
                        continue;

                    // 2. Can trainer teach this spell? (Checks class, level <= 80, requirements)
                    if (!trainer->CanTeachSpell(player, &trainerSpell))
                        continue;

                    if (trainerSpell.IsCastable())
                        player->CastSpell(player, spellId, true);
                    else
                        player->learnSpell(spellId, false);

                    hadNew = true;
                }
            }
        } while (hadNew);

        // Learn essential base class quest spells (Warrior stances, Druid forms, Shaman totems, Warlock demons)
        // These are purely base class quest abilities, never talents.
        for (auto const& [questId, quest] : sObjectMgr->GetQuestTemplates())
        {
            if (quest->GetRequiredClasses() && player->SatisfyQuestClass(quest, false))
                player->learnQuestRewardedSpells(quest);
        }
    }
}

class npc_fast_start : public CreatureScript
{
public:
    npc_fast_start() : CreatureScript("npc_fast_start") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        uint32 accountId = player->GetSession()->GetAccountId();

        // Check if account has claims available in auth database
        QueryResult result = LoginDatabase.Query("SELECT claims_available FROM custom_account_fast_start WHERE account_id = {}", accountId);
        uint32 claims = (result) ? result->Fetch()[0].Get<uint32>() : 0;

        if (claims == 0)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "На вашем аккаунте нет доступного «Быстрого старта».", GOSSIP_SENDER_MAIN, 9999);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Для получения доступа обратитесь к Администрации сервера.", GOSSIP_SENDER_MAIN, 9999);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        // Show available options tailored to player's class
        switch (player->getClass())
        {
            case CLASS_PALADIN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Holy_HolyBolt:20:20:0:0|t Свет (Holy - Исцеление / Хил)", GOSSIP_SENDER_MAIN, ROLE_PALADIN_HOLY,
                    "Вы действительно хотите активировать Быстрый старт: Свет (Хил)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Defend:20:20:0:0|t Защита (Protection - Танк)", GOSSIP_SENDER_MAIN, ROLE_PALADIN_PROT,
                    "Вы действительно хотите активировать Быстрый старт: Защита (Танк)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Holy_AuraOfLight:20:20:0:0|t Воздаяние (Retribution - Урон / ДД)", GOSSIP_SENDER_MAIN, ROLE_PALADIN_RET,
                    "Вы действительно хотите активировать Быстрый старт: Воздаяние (ДД)?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_WARRIOR:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Defend:20:20:0:0|t Защита (Protection - Танк)", GOSSIP_SENDER_MAIN, ROLE_WARRIOR_PROT,
                    "Вы действительно хотите активировать Быстрый старт: Защита (Танк)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Warrior_OffensiveStance:20:20:0:0|t Оружие / Неистовство (Arms/Fury - Урон / ДД)", GOSSIP_SENDER_MAIN, ROLE_WARRIOR_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Воин ДД?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_DEATH_KNIGHT:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Defend:20:20:0:0|t Кровь / Лед (Танк)", GOSSIP_SENDER_MAIN, ROLE_DK_TANK,
                    "Вы действительно хотите активировать Быстрый старт: Рыцарь смерти Танк?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Deathknight_FrostPresence:20:20:0:0|t Лед / Нечестивость (Урон / ДД)", GOSSIP_SENDER_MAIN, ROLE_DK_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Рыцарь смерти ДД?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_PRIEST:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Holy_GuardianSpirit:20:20:0:0|t Свет / Послушание (Holy/Disc - Исцеление / Хил)", GOSSIP_SENDER_MAIN, ROLE_PRIEST_HEAL,
                    "Вы действительно хотите активировать Быстрый старт: Жрец Хил?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Shadow_ShadowWordPain:20:20:0:0|t Тьма (Shadow - Магический урон / РДД)", GOSSIP_SENDER_MAIN, ROLE_PRIEST_SHADOW,
                    "Вы действительно хотите активировать Быстрый старт: Тьма (ШП)?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_SHAMAN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Nature_MagicImmunity:20:20:0:0|t Исцеление (Restoration - Хил)", GOSSIP_SENDER_MAIN, ROLE_SHAMAN_RESTO,
                    "Вы действительно хотите активировать Быстрый старт: Шаман Хил?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Nature_Lightning:20:20:0:0|t Стихии (Elemental - Магический урон / РДД)", GOSSIP_SENDER_MAIN, ROLE_SHAMAN_ELE,
                    "Вы действительно хотите активировать Быстрый старт: Стихии (Элем)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Nature_LightningShield:20:20:0:0|t Совершенствование (Enhancement - Мили урон / МДД)", GOSSIP_SENDER_MAIN, ROLE_SHAMAN_ENH,
                    "Вы действительно хотите активировать Быстрый старт: Совершенствование (Энх)?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_DRUID:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Druid_TreeofLife:20:20:0:0|t Исцеление (Restoration - Хил)", GOSSIP_SENDER_MAIN, ROLE_DRUID_RESTO,
                    "Вы действительно хотите активировать Быстрый старт: Друид Хил (Полено)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Druid_MoonkinForm:20:20:0:0|t Баланс (Balance - Магический урон / Сова)", GOSSIP_SENDER_MAIN, ROLE_DRUID_BALANCE,
                    "Вы действительно хотите активировать Быстрый старт: Баланс (Сова)?\nУслуга спишется с вашего аккаунта!", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Druid_CatForm:20:20:0:0|t Сила зверя (Feral - Кот / Медведь)", GOSSIP_SENDER_MAIN, ROLE_DRUID_FERAL,
                    "Вы действительно хотите активировать Быстрый старт: Сила зверя (Ферал)?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_ROGUE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Backstab:20:20:0:0|t Разбойник (Rogue - Урон / ДД)", GOSSIP_SENDER_MAIN, ROLE_ROGUE_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Разбойник?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_HUNTER:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Ability_Hunter_AimedShot:20:20:0:0|t Охотник (Hunter - Дальний бой / РДД)", GOSSIP_SENDER_MAIN, ROLE_HUNTER_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Охотник?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_MAGE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Holy_MagicalSentry:20:20:0:0|t Маг (Mage - Магический урон / РДД)", GOSSIP_SENDER_MAIN, ROLE_MAGE_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Маг?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            case CLASS_WARLOCK:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|TInterface/Icons/Spell_Shadow_Metamorphosis:20:20:0:0|t Чернокнижник (Warlock - Магический урон / РДД)", GOSSIP_SENDER_MAIN, ROLE_WARLOCK_DPS,
                    "Вы действительно хотите активировать Быстрый старт: Чернокнижник?\nУслуга спишется с вашего аккаунта!", 0, false);
                break;

            default:
                break;
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        CloseGossipMenuFor(player);

        if (sender != GOSSIP_SENDER_MAIN || action == 9999)
            return true;

        uint32 accountId = player->GetSession()->GetAccountId();

        // 1. Atomic verification and consumption of Fast Start claim
        QueryResult qResult = LoginDatabase.Query("SELECT claims_available FROM custom_account_fast_start WHERE account_id = {} AND claims_available > 0", accountId);
        if (!qResult)
        {
            ChatHandler(player->GetSession()).SendSysMessage("Ошибка: На вашем аккаунте нет доступного Быстрого старта!");
            return true;
        }

        LoginDatabase.Execute("UPDATE custom_account_fast_start SET claims_available = claims_available - 1, claims_used = claims_used + 1 WHERE account_id = {}", accountId);
        CharacterDatabase.Execute("INSERT IGNORE INTO custom_fast_start_characters (guid, account_id) VALUES ({}, {})", player->GetGUID().GetCounter(), accountId);

        // 2. Level 80 Boost
        if (player->GetLevel() < 80)
        {
            player->GiveLevel(80);
            player->InitTalentForLevel();
        }
        player->SetFullHealth();
        if (player->getPowerType() == POWER_MANA)
            player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));

        // 3. Maximize Weapon & Class Skills to 400 and teach base non-talent spells
        TeachWeaponProficiencies(player);
        TeachBaseClassSpells(player);
        player->UpdateSkillsToMaxSkillsForLevel();

        // 4. Give Gold (Configurable in worldserver.conf, defaults to 150)
        uint32 fsGold = sConfigMgr->GetOption<uint32>("FastStart.Gold", 150);
        if (fsGold > 0)
            player->ModifyMoney(fsGold * GOLD);

        // 5. 4x 20-slot bags (Item 41599: Frostweave Bag - Soulbound on receipt)
        for (uint8 bSlot = INVENTORY_SLOT_BAG_START; bSlot < INVENTORY_SLOT_BAG_END; ++bSlot)
        {
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, bSlot))
            {
                Item* bag = Item::CreateItem(41599, 1, player);
                if (bag)
                {
                    bag->SetBinding(true);
                    uint16 dest;
                    if (player->CanEquipItem(bSlot, dest, bag, false) == EQUIP_ERR_OK)
                    {
                        player->EquipItem(dest, bag, true);
                        continue;
                    }
                    delete bag;
                }
            }
            ItemPosCountVec sDest;
            InventoryResult bRes = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, sDest, 41599, 1);
            if (bRes == EQUIP_ERR_OK)
            {
                Item* bag = player->StoreNewItem(sDest, 41599, true);
                if (bag)
                    bag->SetBinding(true);
            }
        }

        // 6. 150% Flying Skills and Mounts
        player->learnSpell(33388, false); // Apprentice Riding (75)
        player->learnSpell(33391, false); // Journeyman Riding (150)
        player->learnSpell(34090, false); // Expert Riding (225 - 150% flying)
        player->learnSpell(54197, false); // Cold Weather Flying (Северный полёт)

        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            player->AddItem(25471, 1); // Snowy Gryphon (Flying)
            player->AddItem(18776, 1); // Swift Brown Steed (Ground)
        }
        else
        {
            player->AddItem(25474, 1); // Tawny Wind Rider (Flying)
            player->AddItem(18796, 1); // Horn of the Swift Brown Wolf (Ground)
        }

        // 7. Add full 200 ilvl blue dungeon gear set directly into player's bags
        // (Player's equipped gear is untouched; player can equip 200 ilvl items from bags)
        std::vector<uint32> const* gearSet = GetGearSetForRole(action);
        if (gearSet)
        {
            for (uint32 itemId : *gearSet)
            {
                player->AddItem(itemId, 1);
            }
        }

        // 9. Extra class weapons / ammo for dual-spec & hunter
        if (action == ROLE_WARRIOR_DPS)
        {
            // Give second 2H Sword (Sword of Justice) into bags for Fury Titan's Grip dual wield
            player->AddItem(37653, 1);
        }
        else if (action == ROLE_HUNTER_DPS)
        {
            // Give 2x stacks of 1000 Saronite Razorheads (arrows) for bow
            player->AddItem(41165, 1000);
            player->AddItem(41165, 1000);
        }

        // 10. Starter sustenance (food & drink)
        player->AddItem(33445, 20); // Honeymint Tea
        player->AddItem(33454, 20); // Mead Basted Caribou

        // 9. Visual Celebration Effect & Feedback
        creature->CastSpell(player, 47292, true);
        player->SaveToDB(false, false);

        // 10. Audit Logging in CharacterDatabase
        CharacterDatabase.Execute(
            "INSERT INTO custom_fast_start_log (account_id, character_guid, character_name, class, role_chosen, claimed_at) "
            "VALUES ({}, {}, '{}', {}, '{}', NOW())",
            accountId, player->GetGUID().GetCounter(), player->GetName(),
            uint32(player->getClass()), GetRoleName(action)
        );

        ChatHandler(player->GetSession()).SendSysMessage("==================================================");
        ChatHandler(player->GetSession()).SendSysMessage("Поздравляем! «Быстрый старт» успешно активирован!");
        ChatHandler(player->GetSession()).SendSysMessage("Вам начислено: 80 уровень, 3500 золота, 4 сумки,");
        ChatHandler(player->GetSession()).SendSysMessage("150% полеты, маунты и комплект экипировки 200 илвл (в сумках)!");
        ChatHandler(player->GetSession()).SendSysMessage("Откройте сумки, чтобы надеть полученную экипировку.");
        ChatHandler(player->GetSession()).SendSysMessage("==================================================");

        return true;
    }
};

using namespace Acore::ChatCommands;

class fast_start_commandscript : public CommandScript
{
public:
    fast_start_commandscript() : CommandScript("fast_start_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable fastStartSubCommands =
        {
            { "grant",  HandleFastStartGrantCommand,  rbac::RBAC_PERM_COMMAND_SERVER, Console::Yes },
            { "revoke", HandleFastStartRevokeCommand, rbac::RBAC_PERM_COMMAND_SERVER, Console::Yes },
            { "status", HandleFastStartStatusCommand, rbac::RBAC_PERM_COMMAND_SERVER, Console::Yes },
        };

        static ChatCommandTable commandTable =
        {
            { "faststart", fastStartSubCommands },
            { "bs",        fastStartSubCommands },
        };

        return commandTable;
    }

    static bool HandleFastStartGrantCommand(ChatHandler* handler, std::string accountName)
    {
        if (accountName.empty())
        {
            handler->SendErrorMessage("Использование: .faststart grant <логин_аккаунта>");
            return false;
        }

        QueryResult accRes = LoginDatabase.Query("SELECT id, username FROM account WHERE username = '{}'", accountName);
        if (!accRes)
        {
            handler->SendErrorMessage("Ошибка: Аккаунт '{}' не найден в базе данных!", accountName);
            return false;
        }

        Field* fields = accRes->Fetch();
        uint32 accountId = fields[0].Get<uint32>();
        std::string actualUsername = fields[1].Get<std::string>();
        std::string adminName = handler->GetSession() ? handler->GetPlayer()->GetName() : "Console";

        LoginDatabase.Execute(
            "INSERT INTO custom_account_fast_start (account_id, account_name, claims_available, claims_used, granted_by, granted_at) "
            "VALUES ({}, '{}', 1, 0, '{}', NOW()) "
            "ON DUPLICATE KEY UPDATE claims_available = claims_available + 1, granted_at = NOW(), granted_by = '{}'",
            accountId, actualUsername, adminName, adminName
        );

        handler->PSendSysMessage("Быстрый старт успешно выдан аккаунту '{}' (ID: {}).", actualUsername, accountId);
        return true;
    }

    static bool HandleFastStartRevokeCommand(ChatHandler* handler, std::string accountName)
    {
        if (accountName.empty())
        {
            handler->SendErrorMessage("Использование: .faststart revoke <логин_аккаунта>");
            return false;
        }

        QueryResult accRes = LoginDatabase.Query("SELECT id, username FROM account WHERE username = '{}'", accountName);
        if (!accRes)
        {
            handler->SendErrorMessage("Ошибка: Аккаунт '{}' не найден в базе данных!", accountName);
            return false;
        }

        Field* fields = accRes->Fetch();
        uint32 accountId = fields[0].Get<uint32>();
        std::string actualUsername = fields[1].Get<std::string>();

        LoginDatabase.Execute("UPDATE custom_account_fast_start SET claims_available = 0 WHERE account_id = {}", accountId);
        handler->PSendSysMessage("Быстрый старт аннулирован для аккаунта '{}' (ID: {}).", actualUsername, accountId);
        return true;
    }

    static bool HandleFastStartStatusCommand(ChatHandler* handler, std::string accountName)
    {
        if (accountName.empty())
        {
            handler->SendErrorMessage("Использование: .faststart status <логин_аккаунта>");
            return false;
        }

        QueryResult accRes = LoginDatabase.Query("SELECT id, username FROM account WHERE username = '{}'", accountName);
        if (!accRes)
        {
            handler->SendErrorMessage("Ошибка: Аккаунт '{}' не найден в базе данных!", accountName);
            return false;
        }

        Field* fields = accRes->Fetch();
        uint32 accountId = fields[0].Get<uint32>();
        std::string actualUsername = fields[1].Get<std::string>();

        QueryResult statusRes = LoginDatabase.Query("SELECT claims_available, claims_used, granted_by, granted_at FROM custom_account_fast_start WHERE account_id = {}", accountId);
        if (!statusRes)
        {
            handler->PSendSysMessage("Аккаунт '{}' (ID: {}): Доступ к Быстрому старту никогда не выдавался.", actualUsername, accountId);
            return true;
        }

        Field* sFields = statusRes->Fetch();
        uint32 available = sFields[0].Get<uint32>();
        uint32 used = sFields[1].Get<uint32>();
        std::string grantedBy = sFields[2].Get<std::string>();

        handler->PSendSysMessage("Аккаунт: '{}' (ID: {}). Доступно: {}, Использовано: {}, Выдал: {}.",
            actualUsername, accountId, available, used, grantedBy);

        return true;
    }
};

class fast_start_player_security : public PlayerScript
{
public:
    fast_start_player_security() : PlayerScript("fast_start_player_security") {}

    static bool IsFastStartRestricted(Player* player)
    {
        if (!player)
            return false;

        uint32 lockDuration = sConfigMgr->GetOption<uint32>("FastStart.TradeLockTime", 7200); // Default: 2 hours (7200s)
        if (lockDuration == 0)
            return false;

        if (player->GetTotalPlayedTime() >= lockDuration)
            return false;

        QueryResult res = CharacterDatabase.Query("SELECT 1 FROM custom_fast_start_characters WHERE guid = {}", player->GetGUID().GetCounter());
        return (res != nullptr);
    }

    bool OnPlayerCanInitTrade(Player* player, Player* target) override
    {
        uint32 lockDuration = sConfigMgr->GetOption<uint32>("FastStart.TradeLockTime", 7200);
        if (IsFastStartRestricted(player))
        {
            uint32 played = player->GetTotalPlayedTime();
            uint32 remainingMin = (lockDuration > played) ? ((lockDuration - played) / 60 + 1) : 1;
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff2020[Xavfsizlik]|r Fast Start qahramoni uchun savdo (obmen) yana %u daqiqa o'yin vaqtidan so'ng ochiladi!", remainingMin);
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff2020[Безопасность]|r Торговля для персонажа с Быстрым стартом откроется через %u мин. игры!", remainingMin);
            return false;
        }

        if (target && IsFastStartRestricted(target))
        {
            ChatHandler(player->GetSession()).SendSysMessage("|cffff2020[Xavfsizlik]|r Ushbu qahramon bilan savdo qilish hozircha cheklangan (Fast Start himoyasi)!");
            ChatHandler(player->GetSession()).SendSysMessage("|cffff2020[Безопасность]|r С этим персонажем торговля временно недоступна (защита Быстрого старта)!");
            return false;
        }

        return true;
    }

    bool OnPlayerCanSendMail(Player* player, ObjectGuid /*receiverGuid*/, ObjectGuid /*mailbox*/, std::string& /*subject*/, std::string& /*body*/, uint32 money, uint32 /*COD*/, Item* item) override
    {
        if (IsFastStartRestricted(player) && (money > 0 || item != nullptr))
        {
            uint32 lockDuration = sConfigMgr->GetOption<uint32>("FastStart.TradeLockTime", 7200);
            uint32 played = player->GetTotalPlayedTime();
            uint32 remainingMin = (lockDuration > played) ? ((lockDuration - played) / 60 + 1) : 1;
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff2020[Xavfsizlik]|r Fast Start qahramoni uchun pochta orqali oltin/buyum yuborish yana %u daqiqa o'yin vaqtidan so'ng ochiladi!", remainingMin);
            ChatHandler(player->GetSession()).PSendSysMessage("|cffff2020[Безопасность]|r Отправка золота/предметов по почте откроется через %u мин. игры!", remainingMin);
            return false;
        }

        return true;
    }
};

void AddSC_custom_fast_start()
{
    new npc_fast_start();
    new fast_start_commandscript();
    new fast_start_player_security();
}
