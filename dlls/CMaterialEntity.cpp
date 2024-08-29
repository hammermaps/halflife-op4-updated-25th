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

#include "extdll.h"
#include "util.h"
#include "cbase.h"

TYPEDESCRIPTION CMaterialEntity::m_SaveData[] = {
    DEFINE_FIELD(CMaterialEntity, m_Material, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMaterialEntity, CBaseDelay);

const char* CMaterialEntity::pSoundsWood[] = {
    "debris/wood1.wav",
    "debris/wood2.wav",
    "debris/wood3.wav",
    "debris/wood4.wav",
};

const char* CMaterialEntity::pSoundsFlesh[] = {
    "debris/flesh1.wav",
    "debris/flesh2.wav",
    "debris/flesh3.wav",
    "debris/flesh5.wav",
    "debris/flesh6.wav",
    "debris/flesh7.wav",
};

const char* CMaterialEntity::pSoundsMetal[] = {
    "debris/metal1.wav",
    "debris/metal2.wav",
    "debris/metal3.wav",
};

const char* CMaterialEntity::pSoundsConcrete[] = {
    "debris/concrete1.wav",
    "debris/concrete2.wav",
    "debris/concrete3.wav",
};


const char* CMaterialEntity::pSoundsGlass[] = {
    "debris/glass1.wav",
    "debris/glass2.wav",
    "debris/glass3.wav",
    "debris/glass4.wav",
};

bool CMaterialEntity::KeyValue(KeyValueData* pkvd)
{
    if (FStrEq(pkvd->szKeyName, "material"))
    {
        int i = atoi(pkvd->szValue);

        // 0:glass, 1:metal, 2:flesh, 3:wood

        if ((i < 0) || (i >= matLastMaterial))
            m_Material = matWood;
        else
            m_Material = (Materials)i;

        return true;
    }

    return CBaseDelay::KeyValue(pkvd);
}

const char** CMaterialEntity::MaterialSoundList(Materials precacheMaterial, int& soundCount)
{
	const char** pSoundList = nullptr;

	switch (precacheMaterial)
	{
	case matWood:
		pSoundList = pSoundsWood;
		soundCount = ARRAYSIZE(pSoundsWood);
		break;
	case matFlesh:
		pSoundList = pSoundsFlesh;
		soundCount = ARRAYSIZE(pSoundsFlesh);
		break;
	case matComputer:
	case matUnbreakableGlass:
	case matGlass:
		pSoundList = pSoundsGlass;
		soundCount = ARRAYSIZE(pSoundsGlass);
		break;
	case matMetal:
		pSoundList = pSoundsMetal;
		soundCount = ARRAYSIZE(pSoundsMetal);
		break;
	case matCinderBlock:
	case matRocks:
		pSoundList = pSoundsConcrete;
		soundCount = ARRAYSIZE(pSoundsConcrete);
		break;
	case matCeilingTile:
	case matNone:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CMaterialEntity::Precache()
{
    PrecacheSoundArray(pSoundsWood,ARRAYSIZE(pSoundsWood));
    PrecacheSoundArray(pSoundsFlesh,ARRAYSIZE(pSoundsFlesh));
    PrecacheSoundArray(pSoundsGlass,ARRAYSIZE(pSoundsGlass));
    PrecacheSoundArray(pSoundsMetal,ARRAYSIZE(pSoundsMetal));
    PrecacheSoundArray(pSoundsConcrete,ARRAYSIZE(pSoundsConcrete));
}

void CMaterialEntity::MaterialSoundRandom(edict_t* pEdict, Materials soundMaterial, int channel, float volume, int pitch)
{
    int soundCount = 0;
	const char** pSoundList = MaterialSoundList(soundMaterial, soundCount);
	if (0 != soundCount)
	{
	    EMIT_SOUND_DYN(pEdict, channel, pSoundList[RANDOM_LONG(0, soundCount - 1)], volume, ATTN_NORM, 0, pitch);
	}
}

void CMaterialEntity::MaterialSoundRandom(edict_t* pEdict, Materials soundMaterial, float volume)
{
    int pitch;
    if (RANDOM_LONG(0, 2))
        pitch = PITCH_NORM;
    else
        pitch = 95 + RANDOM_LONG(0, 34);
	    
    MaterialSoundRandom(pEdict, soundMaterial, CHAN_BODY ,volume, pitch);
}