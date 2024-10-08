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

#pragma once

extern void GameDLLInit();
void GameDLLShutdown();


extern cvar_t displaysoundlist;

// multiplayer server rules
extern cvar_t fragsleft;
extern cvar_t timeleft;
extern cvar_t teamplay;
extern cvar_t fraglimit;
extern cvar_t timelimit;
extern cvar_t friendlyfire;
extern cvar_t falldamage;
extern cvar_t weaponstay;
extern cvar_t forcerespawn;
extern cvar_t flashlight;
extern cvar_t aimcrosshair;
extern cvar_t decalfrequency;
extern cvar_t teamlist;
extern cvar_t teamoverride;
extern cvar_t defaultteam;
extern cvar_t allowmonsters;

extern cvar_t ctf_capture;
extern cvar_t oldweapons;
extern cvar_t multipower;
extern cvar_t ctf_autoteam;
extern cvar_t coopplay;
extern cvar_t coopweprespawn;
extern cvar_t spamdelay;

extern cvar_t sv_allowbunnyhopping;


// Engine Cvars
extern cvar_t* g_psv_gravity;
extern cvar_t* g_psv_aim;
extern cvar_t* g_footsteps;
extern cvar_t *g_psv_allow_autoaim;
extern cvar_t* g_psv_cheats;
extern cvar_t allow_spectators;
extern cvar_t mp_chattime;

