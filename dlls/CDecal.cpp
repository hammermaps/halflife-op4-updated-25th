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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "decals.h"

/**
 * \brief List of decals used in the game.
 *
 * This array contains the names and initial indices of various decals that can be applied to surfaces in the game.
 */
DLL_DECALLIST gDecals[] = {
    {"{shot1", 0},       // DECAL_GUNSHOT1
    {"{shot2", 0},       // DECAL_GUNSHOT2
    {"{shot3", 0},       // DECAL_GUNSHOT3
    {"{shot4", 0},       // DECAL_GUNSHOT4
    {"{shot5", 0},       // DECAL_GUNSHOT5
    {"{lambda01", 0},    // DECAL_LAMBDA1
    {"{lambda02", 0},    // DECAL_LAMBDA2
    {"{lambda03", 0},    // DECAL_LAMBDA3
    {"{lambda04", 0},    // DECAL_LAMBDA4
    {"{lambda05", 0},    // DECAL_LAMBDA5
    {"{lambda06", 0},    // DECAL_LAMBDA6
    {"{scorch1", 0},     // DECAL_SCORCH1
    {"{scorch2", 0},     // DECAL_SCORCH2
    {"{blood1", 0},      // DECAL_BLOOD1
    {"{blood2", 0},      // DECAL_BLOOD2
    {"{blood3", 0},      // DECAL_BLOOD3
    {"{blood4", 0},      // DECAL_BLOOD4
    {"{blood5", 0},      // DECAL_BLOOD5
    {"{blood6", 0},      // DECAL_BLOOD6
    {"{yblood1", 0},     // DECAL_YBLOOD1
    {"{yblood2", 0},     // DECAL_YBLOOD2
    {"{yblood3", 0},     // DECAL_YBLOOD3
    {"{yblood4", 0},     // DECAL_YBLOOD4
    {"{yblood5", 0},     // DECAL_YBLOOD5
    {"{yblood6", 0},     // DECAL_YBLOOD6
    {"{break1", 0},      // DECAL_GLASSBREAK1
    {"{break2", 0},      // DECAL_GLASSBREAK2
    {"{break3", 0},      // DECAL_GLASSBREAK3
    {"{bigshot1", 0},    // DECAL_BIGSHOT1
    {"{bigshot2", 0},    // DECAL_BIGSHOT2
    {"{bigshot3", 0},    // DECAL_BIGSHOT3
    {"{bigshot4", 0},    // DECAL_BIGSHOT4
    {"{bigshot5", 0},    // DECAL_BIGSHOT5
    {"{spit1", 0},       // DECAL_SPIT1
    {"{spit2", 0},       // DECAL_SPIT2
    {"{bproof1", 0},     // DECAL_BPROOF1
    {"{gargstomp", 0},   // DECAL_GARGSTOMP1,  // Gargantua stomp crack
    {"{smscorch1", 0},   // DECAL_SMALLSCORCH1,    // Small scorch mark
    {"{smscorch2", 0},   // DECAL_SMALLSCORCH2,    // Small scorch mark
    {"{smscorch3", 0},   // DECAL_SMALLSCORCH3,    // Small scorch mark
    {"{mommablob", 0},   // DECAL_MOMMABIRTH       // BM Birth spray
    {"{mommablob", 0},   // DECAL_MOMMASPLAT       // BM Mortar spray?? need decal
    {"{spr_splt1", 0},   // DECAL_SPR_SPLT1
    {"{spr_splt2", 0},   // DECAL_SPR_SPLT2
    {"{spr_splt3", 0},   // DECAL_SPR_SPLT3
    {"{ofscorch1", 0},   // DECAL_OFSCORCH1
    {"{ofscorch2", 0},   // DECAL_OFSCORCH2
    {"{ofscorch3", 0},   // DECAL_OFSCORCH3
    {"{ofsmscorch1", 0}, // DECAL_OFSMSCORCH1
    {"{ofsmscorch2", 0}, // DECAL_OFSMSCORCH2
    {"{ofsmscorch3", 0}, // DECAL_OFSMSCORCH3
};

#define SF_DECAL_NOTINDEATHMATCH 2048

/**
 * \brief Class representing a decal entity.
 *
 * This class handles the spawning, precaching, and triggering of decals in the game.
 */
class CDecal : public CBaseEntity
{
public:
    /**
     * \brief Spawns the decal entity.
     *
     * This function is called when the decal entity is spawned in the game. It handles the precaching of the decal
     * and sets up the appropriate think and use functions based on whether the decal has a target name.
     */
    void Spawn() override;

    /**
     * \brief Precaches the decal entity.
     *
     * This function is called to precache the decal entity, which involves loading the decal textures and setting their indices.
     */
    void Precache() override;

    /**
     * \brief Handles key-value data for the decal entity.
     *
     * This function is called to handle key-value data for the decal entity, such as setting the texture to be used for the decal.
     *
     * \param pkvd Pointer to the key-value data.
     * \return True if the key-value data was handled, otherwise false.
     */
    bool KeyValue(KeyValueData* pkvd) override;

    /**
     * \brief Applies the decal statically.
     *
     * This function is called to apply the decal statically, meaning it is applied immediately when the world is done spawning.
     */
    void EXPORT StaticDecal();

    /**
     * \brief Triggers the decal.
     *
     * This function is called to trigger the decal, meaning it is applied when the entity is triggered by another entity.
     *
     * \param pActivator Pointer to the entity that activated the decal.
     * \param pCaller Pointer to the entity that called the trigger.
     * \param useType The type of use (e.g., toggle, on, off).
     * \param value The value associated with the use.
     */
    void EXPORT TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(infodecal, CDecal);

/**
 * \brief Spawns the decal entity.
 *
 * This function is called when the decal entity is spawned in the game. It handles the precaching of the decal
 * and sets up the appropriate think and use functions based on whether the decal has a target name.
 */
void CDecal::Spawn()
{
    Precache();
    
    if (pev->skin < 0 || (0 != gpGlobals->deathmatch && FBitSet(pev->spawnflags, SF_DECAL_NOTINDEATHMATCH)))
    {
        REMOVE_ENTITY(ENT(pev));
        return;
    }

    if (FStringNull(pev->targetname))
    {
        SetThink(&CDecal::StaticDecal);
        // if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
        SetNextThink(0.0f);
    }
    else
    {
        // if there IS a targetname, the decal sprays itself on when it is triggered.
        SetThink(&CDecal::SUB_DoNothing);
        SetUse(&CDecal::TriggerDecal);
    }
}

/**
 * \brief Triggers the decal.
 *
 * This function is called to trigger the decal, meaning it is applied when the entity is triggered by another entity.
 *
 * \param pActivator Pointer to the entity that activated the decal.
 * \param pCaller Pointer to the entity that called the trigger.
 * \param useType The type of use (e.g., toggle, on, off).
 * \param value The value associated with the use.
 */
void CDecal::TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
    // this is set up as a USE function for infodecals that have targetnames, so that the
    // decal doesn't get applied until it is fired. (usually by a scripted sequence)
    TraceResult trace;

    UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);

    MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
        WRITE_BYTE(TE_BSPDECAL);
        WRITE_COORD(pev->origin.x);
        WRITE_COORD(pev->origin.y);
        WRITE_COORD(pev->origin.z);
        WRITE_SHORT(pev->skin);
        int entityIndex = (short)ENTINDEX(trace.pHit);
        WRITE_SHORT(entityIndex);
        if (0 != entityIndex)
            WRITE_SHORT(VARS(trace.pHit)->modelindex);
    MESSAGE_END();

    SetThink(&CDecal::SUB_Remove);
    SetNextThink(0.1f);
}

/**
 * \brief Precaches the decal entity.
 *
 * This function is called to precache the decal entity, which involves loading the decal textures and setting their indices.
 */
void CDecal::Precache()
{
    for (int i = 0; i < ARRAYSIZE(gDecals); i++)
        gDecals[i].index = DECAL_INDEX(gDecals[i].name);
}

/**
 * \brief Applies the decal statically.
 *
 * This function is called to apply the decal statically, meaning it is applied immediately when the world is done spawning.
 */
void CDecal::StaticDecal()
{
    TraceResult trace;

    UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);
    
    int entityIndex = (short)ENTINDEX(trace.pHit);

    int modelIndex = 0;
    if (0 != entityIndex)
        modelIndex = VARS(trace.pHit)->modelindex;

    g_engfuncs.pfnStaticDecal(pev->origin, (int)pev->skin, entityIndex, modelIndex);

    SUB_Remove();
}

/**
 * \brief Handles key-value data for the decal entity.
 *
 * This function is called to handle key-value data for the decal entity, such as setting the texture to be used for the decal.
 *
 * \param pkvd Pointer to the key-value data.
 * \return True if the key-value data was handled, otherwise false.
 */
bool CDecal::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "texture"))
    {
        pev->skin = DECAL_INDEX(pkvd->szValue);

        if (pev->skin < 0)
        {
            ALERT(at_console, "Can't find decal %s\n", pkvd->szValue);
        }

        return true;
    }

    return CBaseEntity::KeyValue(pkvd);
}