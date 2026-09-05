/*
 * mod-progressive-wotlk
 * AzerothCore Progressive WotLK Phase System
 */

#ifndef PROGRESSIVE_WOTLK_H
#define PROGRESSIVE_WOTLK_H

#include "Common.h"
#include "Config.h"
#include "DisableMgr.h"
#include "Player.h"
#include "World.h"

// Cached Configuration
struct PWConfig
{
    static inline bool Enable = true;
    static inline bool GMBypass = true;
    static inline bool AnnouncePhaseChange = true;

    static void Load(bool /*reload*/ = false)
    {
        Enable = sConfigMgr->GetOption<bool>("ProgressiveWotlk.Enable", true, false);
        GMBypass = sConfigMgr->GetOption<bool>("ProgressiveWotlk.GMBypass", true, false);
        AnnouncePhaseChange = sConfigMgr->GetOption<bool>("ProgressiveWotlk.AnnouncePhaseChange", true, false);
    }
};

// Item IDs for WotLK Emblems
enum WotLKEmblems : uint32
{
    ITEM_EMBLEM_OF_HEROISM   = 40752, // Phase 1 (5-man HC, 10-man Raids)
    ITEM_EMBLEM_OF_VALOR     = 40753, // Phase 1 (25-man Raids, Daily HC)
    ITEM_EMBLEM_OF_CONQUEST  = 45624, // Phase 2 (25-man Ulduar, Daily HC)
    ITEM_EMBLEM_OF_TRIUMPH   = 47241, // Phase 3 (ToC, 5-man HC catchup)
    ITEM_EMBLEM_OF_FROST     = 49426, // Phase 4 (ICC, Daily HC)
};

// Map gating entries with proper difficulty bitmasks
struct PhaseMapEntry
{
    uint8       unlockPhase;
    uint32      mapId;
    uint8       diffMask; // Bitmask: 3 for 5-man & 10/25 normal; 15 for 10N/25N/10H/25H
    const char* mapName;
};

static const PhaseMapEntry PHASE_MAP_TABLE[] =
{
    // Phase 2: Ulduar & Onyxia
    { 2, 249, 3,  "Onyxia's Lair"         },
    { 2, 603, 3,  "Ulduar"                },

    // Phase 3: Trial of the Crusader, Trial of the Champion, Isle of Conquest
    { 3, 628, 1,  "Isle of Conquest"      },
    { 3, 649, 15, "Trial of the Crusader" },
    { 3, 650, 3,  "Trial of the Champion" },

    // Phase 4: Icecrown Citadel & ICC Dungeons
    { 4, 631, 15, "Icecrown Citadel"      },
    { 4, 632, 3,  "Forge of Souls"        },
    { 4, 658, 3,  "Pit of Saron"          },
    { 4, 668, 3,  "Halls of Reflection"   },

    // Phase 5: Ruby Sanctum
    { 5, 724, 15, "Ruby Sanctum"          },
};

static const uint8 PHASE_MAP_TABLE_SIZE = sizeof(PHASE_MAP_TABLE) / sizeof(PHASE_MAP_TABLE[0]);

// Dalaran Emblem Quartermasters, VoA Bosses, & Arena / Honor Vendors
struct PhaseCreatureEntry
{
    uint8       unlockPhase;
    uint32      creatureId;
    const char* creatureName;
};

static const PhaseCreatureEntry PHASE_CREATURE_TABLE[] =
{
    // ========================================================
    // Phase 2: Ulduar & Arena Season 6 (Furious Gladiator)
    // ========================================================
    // PvE Quartermasters & VoA Bosses
    { 2, 33963, "Magister Sarien (Conquest Quartermaster)"          },
    { 2, 33964, "Arcanist Firael (Conquest Quartermaster)"          },
    { 2, 33993, "Emalon the Storm Watcher (10-man)"                },
    { 2, 33994, "Emalon the Storm Watcher (25-man)"                },

    // Dalaran Sewers (Kloaka) A6 Furious Arena Armor Vendors
    { 2, 33915, "Argex Irongut (A6 Furious Veteran Vendor)"         },
    { 2, 33918, "Kezzik the Striker (A6 Furious Veteran Vendor)"    },
    { 2, 33921, "Nargle Lashcord (A6 Furious Veteran Vendor)"       },
    { 2, 33926, "Xazi Smolderpipe (A6 Furious Arena Vendor)"        },
    { 2, 33928, "Evee Copperspring (A6 Furious Arena Vendor)"       },
    { 2, 33934, "Ecton Brasstumbler (A6 Furious Apprentice Vendor)" },
    { 2, 33938, "Zom Bocom (A6 Furious Apprentice Vendor)"          },
    { 2, 33941, "Leeni 'Smiley' Smalls (A6 Furious Apprentice Vendor)" },

    // Dalaran Sewers A6 Furious Arena Weapons
    { 2, 34087, "Trapjaw Rix (A6 Furious Weapons)"                  },
    { 2, 34088, "Blazzek the Biter (A6 Furious Weapons)"            },
    { 2, 34089, "Grex Brainboiler (A6 Furious Weapons)"             },

    // Capital Cities / Dalaran A6 Furious Quartermasters
    { 2, 34037, "Sergeant Thunderhorn (A6 Apprentice Quartermaster)" },
    { 2, 34058, "Doris Volanthius (A6 Veteran Quartermaster)"        },
    { 2, 34063, "Blood Guard Zar'shi (A6 Northrend Quartermaster)"   },
    { 2, 34074, "Captain Dirgehammer (A6 Apprentice Quartermaster)"  },
    { 2, 34076, "Lieutenant Tristia (A6 Veteran Quartermaster)"      },
    { 2, 34084, "Knight-Lieutenant Moonstrike (A6 Northrend Quartermaster)" },
    { 2, 40607, "Knight-Lieutenant T'Maire Sydes (A6 Northrend Quartermaster)" },

    // ========================================================
    // Phase 3: Trial of the Crusader & Arena Season 7 (Relentless Gladiator)
    // ========================================================
    // PvE Quartermasters & VoA Bosses
    { 3, 35494, "Arcanist Miluria (Triumph Quartermaster)"         },
    { 3, 35495, "Magistrix Vesara (Triumph Quartermaster)"         },
    { 3, 35573, "Arcanist Asarina (Triumph Quartermaster)"         },
    { 3, 35574, "Magistrix Iruvia (Triumph Quartermaster)"         },
    { 3, 35013, "Koralon the Flame Watcher (10-man)"               },
    { 3, 35360, "Koralon the Flame Watcher (25-man)"               },

    // Dalaran Sewers (Kloaka) A7 Relentless Arena Armor Vendors
    { 3, 33924, "Argex Irongut (A7 Relentless Veteran Vendor)"       },
    { 3, 33927, "Nargle Lashcord (A7 Relentless Veteran Vendor)"     },
    { 3, 33931, "Kezzik the Striker (A7 Relentless Veteran Vendor)"  },
    { 3, 33933, "Big Zokk Torquewrench (A7 Relentless Arena Vendor)" },
    { 3, 33935, "Evee Copperspring (A7 Relentless Arena Vendor)"     },
    { 3, 33937, "Xazi Smolderpipe (A7 Relentless Arena Vendor)"      },

    // Dalaran Sewers A7 Relentless Arena Weapons
    { 3, 34090, "Blazzek the Biter (A7 Relentless Weapons)"          },
    { 3, 34091, "Grex Brainboiler (A7 Relentless Weapons)"           },
    { 3, 34092, "Trapjaw Rix (A7 Relentless Weapons)"                },

    // Capital Cities A7 Relentless Quartermasters
    { 3, 34038, "Sergeant Thunderhorn (A7 Apprentice Quartermaster)" },
    { 3, 34059, "Doris Volanthius (A7 Veteran Quartermaster)"        },
    { 3, 34075, "Captain Dirgehammer (A7 Apprentice Quartermaster)"  },
    { 3, 34077, "Lieutenant Tristia (A7 Veteran Quartermaster)"      },

    // ========================================================
    // Phase 4: Icecrown Citadel & Arena Season 8 (Wrathful Gladiator)
    // ========================================================
    // PvE Quartermasters & VoA Bosses
    { 4, 37941, "Magister Arlan (Frost Quartermaster)"             },
    { 4, 37942, "Arcanist Uovril (Frost Quartermaster)"            },
    { 4, 38858, "Goodman the 'Closer' (Frost Quartermaster)"       },
    { 4, 38433, "Toravon the Ice Watcher (10-man)"                 },
    { 4, 38462, "Toravon the Ice Watcher (25-man)"                 },

    // Dalaran Sewers (Kloaka) A8 Wrathful Arena Armor Vendors
    { 4, 33936, "Nargle Lashcord (A8 Wrathful Veteran Vendor)"     },
    { 4, 33939, "Argex Irongut (A8 Wrathful Veteran Vendor)"       },
    { 4, 33940, "Kezzik the Striker (A8 Wrathful Veteran Vendor)"  },

    // Dalaran Sewers A8 Wrathful Arena Weapons
    { 4, 34093, "Blazzek the Biter (A8 Wrathful Weapons)"          },
    { 4, 34094, "Grex Brainboiler (A8 Wrathful Weapons)"           },
    { 4, 34095, "Trapjaw Rix (A8 Wrathful Weapons)"                },

    // Capital Cities A8 Wrathful Quartermasters
    { 4, 34060, "Doris Volanthius (A8 Veteran Quartermaster)"      },
    { 4, 34078, "Lieutenant Tristia (A8 Veteran Quartermaster)"    },
};

static const uint32 PHASE_CREATURE_TABLE_SIZE = sizeof(PHASE_CREATURE_TABLE) / sizeof(PHASE_CREATURE_TABLE[0]);

static const uint8 MAX_PHASE = 5;
static const uint8 MIN_PHASE = 1;

// Function declarations
uint8       PW_GetCurrentPhase();
void        PW_ApplyPhaseDisables(uint8 currentPhase);
void        PW_ApplyPhaseQuestRewards(uint8 currentPhase);
void        PW_ApplyPhaseLoot(uint8 currentPhase);
void        PW_ApplyPhaseArenaSeason(uint8 currentPhase);
void        PW_ApplyPhaseEpicGems(uint8 currentPhase);
bool        PW_SetPhase(uint8 newPhase, std::string& outMessage);
std::string PW_GetStatusText(uint8 currentPhase);

#endif // PROGRESSIVE_WOTLK_H
