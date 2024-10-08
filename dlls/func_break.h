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
#include "cbase.h"

typedef enum
{
	expRandom,
	expDirected
} Explosions;

#define NUM_SHARDS 6 // this many shards spawned when breakable objects break;

class CBreakable : public CMaterialEntity
{
public:
	// basic functions
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT BreakTouch(CBaseEntity* pOther);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void DamageSound();

	// breakables use an overridden takedamage
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	// To spark when hit
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	bool IsBreakable();
	bool SparkWhenHit();

	int DamageDecal(int bitsDamageType) override;

	void EXPORT Die();
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	inline bool Explodable() { return ExplosionMagnitude() > 0; }
	inline int ExplosionMagnitude() { return pev->impulse; }
	inline void ExplosionSetMagnitude(int magnitude) { pev->impulse = magnitude; }
    
	static TYPEDESCRIPTION m_SaveData[];

    static const char* pSpawnObjects[];
    
	Explosions m_Explosion;
	int m_idShard;
	float m_angle;
	int m_iszGibModel;
	int m_iszSpawnObject;
};
