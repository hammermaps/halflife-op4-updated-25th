/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_dll.h

// this file is included by both the game-dll and the client-dll,

#pragma once

constexpr int MAX_PLAYERS = 32;
#define MAX_WEAPONS 64 // ???

#define MAX_WEAPON_SLOTS 7 // hud item selection slots
#define MAX_ITEM_TYPES 7   // hud item selection slots

#define MAX_ITEMS 5 // hard coded item types

#define HIDEHUD_WEAPONS (1 << 0)
#define HIDEHUD_FLASHLIGHT (1 << 1)
#define HIDEHUD_ALL (1 << 2)
#define HIDEHUD_HEALTH (1 << 3)

#define MAX_AMMO_TYPES 32 // ???
#define MAX_AMMO_SLOTS 32 // not really slots

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

enum WeaponId
{
	WEAPON_NONE = 0,
	WEAPON_CROWBAR,
	WEAPON_GLOCK,
	WEAPON_PYTHON,
	WEAPON_MP5,
	WEAPON_CHAINGUN,
	WEAPON_CROSSBOW,
	WEAPON_SHOTGUN,
	WEAPON_RPG,
	WEAPON_GAUSS,
	WEAPON_EGON,
	WEAPON_HORNETGUN,
	WEAPON_HANDGRENADE,
	WEAPON_TRIPMINE,
	WEAPON_SATCHEL,
	WEAPON_SNARK,
	WEAPON_GRAPPLE,
	WEAPON_EAGLE,
	WEAPON_PIPEWRENCH,
	WEAPON_M249,
	WEAPON_DISPLACER,
	WEAPON_SHOCKRIFLE,
	WEAPON_SPORELAUNCHER,
	WEAPON_SNIPERRIFLE,
	WEAPON_KNIFE,
	WEAPON_PENGUIN,

	WEAPON_SUIT = 31
};


// used by suit voice to indicate damage sustained and repaired type to player
// instant damage
enum DamageTypes
{
    DMG_GENERIC = 0, // generic damage was done
    DMG_CRUSH = (1 << 0), // crushed by falling or moving object
    DMG_BULLET = (1 << 1), // shot
    DMG_SLASH = (1 << 2), // cut, clawed, stabbed
    DMG_BURN = (1 << 3), // heat burned
    DMG_FREEZE = (1 << 4), // frozen
    DMG_FALL = (1 << 5), // fell too far
    DMG_BLAST = (1 << 6), // explosive blast damage
    DMG_CLUB = (1 << 7), // crowbar, punch, headbutt
    DMG_SHOCK = (1 << 8), // electric shock
    DMG_SONIC = (1 << 9), // sound pulse shockwave
    DMG_ENERGYBEAM = (1 << 10), // laser or other high energy beam
    DMG_NEVERGIB = (1 << 12), // with this bit OR'd in, no damage type will be able to gib victims upon death
    DMG_ALWAYSGIB = (1 << 13), // with this bit OR'd in, any damage type can be made to gib victims upon death.
    DMG_TIMEBASED = (~(0xff003fff)), // NOTE: MUST BE LAST TYPE. Time-based damage
    DMG_DROWN = (1 << 14), // Drowning
    DMG_FIRSTTIMEBASED = DMG_DROWN,
    DMG_PARALYZE = (1 << 15), // slows affected creature down
    DMG_NERVEGAS = (1 << 16), // nerve toxins, very bad
    DMG_POISON = (1 << 17), // blood poisioning
    DMG_RADIATION = (1 << 18), // radiation exposure
    DMG_DROWNRECOVER = (1 << 19), // drowning recovery
    DMG_ACID = (1 << 20), // toxic chemicals or acid burns
    DMG_SLOWBURN = (1 << 21), // in an oven
    DMG_SLOWFREEZE = (1 << 22), // in a subzero freezer
    DMG_MORTAR = (1 << 23), // Hit by air raid (done to distinguish grenade from mortar)
    DMG_IGNITE = (1 << 24), // Players hit by this begin to burn
    DMG_RADIUS_MAX = (1 << 25), // Radius damage with this flag doesn't decrease over distance
    DMG_RADIUS_QUAKE = (1 << 26), // Radius damage is done like Quake. 1/2 damage at 1/2 radius.
    DMG_IGNOREARMOR = (1 << 27), // Damage ignores target's armor
    DMG_AIMED = (1 << 28), // Does Hit location damage
    DMG_WALLPIERCING = (1 << 29), // Blast Damages ents through walls
    DMG_CALTROP = (1 << 30),
    DMG_HALLUC = (1 << 31)
};

// TF Healing Additions for TakeHealth
#define DMG_IGNORE_MAXHEALTH DMG_IGNITE
// TF Redefines since we never use the originals
#define DMG_NAIL DMG_SLASH
#define DMG_NOT_SELF DMG_FREEZE


#define DMG_TRANQ DMG_MORTAR
#define DMG_CONCUSS DMG_SONIC

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB)

// these are the damage types that have client hud art
#define DMG_SHOWNHUD (DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION 2 // number of 2 second intervals to take damage
#define PARALYZE_DAMAGE 1.0 // damage to take each 2 second interval

#define NERVEGAS_DURATION 2
#define NERVEGAS_DAMAGE 5.0

#define POISON_DURATION 5
#define POISON_DAMAGE 2.0

#define RADIATION_DURATION 2
#define RADIATION_DAMAGE 1.0

#define ACID_DURATION 2
#define ACID_DAMAGE 5.0

#define SLOWBURN_DURATION 2
#define SLOWBURN_DAMAGE 1.0

#define SLOWFREEZE_DURATION 2
#define SLOWFREEZE_DAMAGE 1.0


#define itbd_Paralyze 0
#define itbd_NerveGas 1
#define itbd_Poison 2
#define itbd_Radiation 3
#define itbd_DrownRecover 4
#define itbd_Acid 5
#define itbd_SlowBurn 6
#define itbd_SlowFreeze 7
#define CDMG_TIMEBASED 8

constexpr Vector VEC_HULL_MIN(-16, -16, -36);
constexpr Vector VEC_HULL_MAX(16, 16, 36);
constexpr Vector VEC_HUMAN_HULL_MIN(-16, -16, 0);
constexpr Vector VEC_HUMAN_HULL_MAX(16, 16, 72);
constexpr Vector VEC_HUMAN_HULL_DUCK(16, 16, 36);

constexpr Vector VEC_VIEW(0, 0, 28);

constexpr Vector VEC_DUCK_HULL_MIN(-16, -16, -18);
constexpr Vector VEC_DUCK_HULL_MAX(16, 16, 18);
constexpr Vector VEC_DUCK_VIEW(0, 0, 12);

constexpr Vector VEC_DEAD_VIEW(0, 0, -8);

#define MENU_NONE 0
#define MENU_DEFAULT 1
#define MENU_TEAM 2
#define MENU_CLASS 3
#define MENU_MAPBRIEFING 4
#define MENU_INTRO 5
#define MENU_CLASSHELP 6
#define MENU_CLASSHELP2 7
#define MENU_REPEATHELP 8
//#define MENU_SPECHELP				9
#define MENU_STATSMENU 9
#define MENU_SCOREBOARD 10

#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010	 //255,160,0
#define RGB_GREENISH 0x0000A000	 //0,160,0

#define RGB_HUD_COLOR RGB_GREENISH

inline void UnpackRGB(int& r, int& g, int& b, unsigned long ulRGB)
{
	r = (ulRGB & 0xFF0000) >> 16;
	g = (ulRGB & 0xFF00) >> 8;
	b = ulRGB & 0xFF;
}
