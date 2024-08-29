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
#include "monsters.h"
#include "saverestore.h"
#include "client.h"
#include "decals.h"
#include "FileSystem.h"
#include "filesystem_utils.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd);

void OnFreeEntPrivateData(edict_s* pEdict);

extern Vector VecBModelOrigin(entvars_t* pevBModel);

static DLL_FUNCTIONS gFunctionTable = {
		GameDLLInit,			   //pfnGameInit
		DispatchSpawn,			   //pfnSpawn
		DispatchThink,			   //pfnThink
		DispatchUse,			   //pfnUse
		DispatchTouch,			   //pfnTouch
		DispatchBlocked,		   //pfnBlocked
		DispatchKeyValue,		   //pfnKeyValue
		DispatchSave,			   //pfnSave
		DispatchRestore,		   //pfnRestore
		DispatchObjectCollsionBox, //pfnAbsBox

		SaveWriteFields, //pfnSaveWriteFields
		SaveReadFields,	 //pfnSaveReadFields

		SaveGlobalState,	//pfnSaveGlobalState
		RestoreGlobalState, //pfnRestoreGlobalState
		ResetGlobalState,	//pfnResetGlobalState

		ClientConnect,		   //pfnClientConnect
		ClientDisconnect,	   //pfnClientDisconnect
		ClientKill,			   //pfnClientKill
		ClientPutInServer,	   //pfnClientPutInServer
		ClientCommand,		   //pfnClientCommand
		ClientUserInfoChanged, //pfnClientUserInfoChanged
		ServerActivate,		   //pfnServerActivate
		ServerDeactivate,	   //pfnServerDeactivate

		PlayerPreThink,	 //pfnPlayerPreThink
		PlayerPostThink, //pfnPlayerPostThink

		StartFrame,		  //pfnStartFrame
		ParmsNewLevel,	  //pfnParmsNewLevel
		ParmsChangeLevel, //pfnParmsChangeLevel

		GetGameDescription,	 //pfnGetGameDescription    Returns string describing current .dll game.
		PlayerCustomization, //pfnPlayerCustomization   Notifies .dll of new customization for player.

		SpectatorConnect,	 //pfnSpectatorConnect      Called when spectator joins server
		SpectatorDisconnect, //pfnSpectatorDisconnect   Called when spectator leaves the server
		SpectatorThink,		 //pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

		Sys_Error, //pfnSys_Error				Called when engine has encountered an error

		PM_Move,			//pfnPM_Move
		PM_Init,			//pfnPM_Init				Server version of player movement initialization
		PM_FindTextureType, //pfnPM_FindTextureType

		SetupVisibility,		  //pfnSetupVisibility        Set up PVS and PAS for networking for this client
		UpdateClientData,		  //pfnUpdateClientData       Set up data sent only to specific client
		AddToFullPack,			  //pfnAddToFullPack
		CreateBaseline,			  //pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
		RegisterEncoders,		  //pfnRegisterEncoders		Callbacks for network encoding
		GetWeaponData,			  //pfnGetWeaponData
		CmdStart,				  //pfnCmdStart
		CmdEnd,					  //pfnCmdEnd
		ConnectionlessPacket,	  //pfnConnectionlessPacket
		GetHullBounds,			  //pfnGetHullBounds
		CreateInstancedBaselines, //pfnCreateInstancedBaselines
		InconsistentFile,		  //pfnInconsistentFile
		AllowLagCompensation,	  //pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS gNewDLLFunctions = {
		OnFreeEntPrivateData, //pfnOnFreeEntPrivateData
		GameDLLShutdown,
};

static void SetObjectCollisionBox(entvars_t* pev);

extern "C" {
    int GetEntityAPI(DLL_FUNCTIONS* pFunctionTable, int interfaceVersion)
    {
	    if (!pFunctionTable || interfaceVersion != INTERFACE_VERSION)
	    {
		    return 0;
	    }

	    memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	    return 1;
    }

    int GetEntityAPI2(DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
    {
	    if (!pFunctionTable || *interfaceVersion != INTERFACE_VERSION)
	    {
		    // Tell engine what version we had, so it can figure out who is out of date.
		    *interfaceVersion = INTERFACE_VERSION;
		    return 0;
	    }

	    memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	    return 1;
    }

    int GetNewDLLFunctions(NEW_DLL_FUNCTIONS* pFunctionTable, int* interfaceVersion)
    {
	    if (!pFunctionTable || *interfaceVersion != NEW_DLL_FUNCTIONS_VERSION)
	    {
		    *interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		    return 0;
	    }

	    memcpy(pFunctionTable, &gNewDLLFunctions, sizeof(gNewDLLFunctions));
	    return 1;
    }
}


int DispatchSpawn(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity)
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->SetAbsMin(pEntity->GetOrigin() - Vector(1, 1, 1));
		pEntity->SetAbsMax(pEntity->GetOrigin() + Vector(1, 1, 1));
		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// UNDONE: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = (CBaseEntity*)GET_PRIVATE(pent);

		if (pEntity)
		{
			if (g_pGameRules && !g_pGameRules->IsAllowedToSpawn(pEntity))
				return -1; // return that this entity should be deleted
		    
			if ((pEntity->pev->flags & FL_KILLME) != 0)
				return -1;
		}


		// Handle global stuff here
		if (pEntity && !FStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;

			    if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
											// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
				//				ALERT( at_console, "Added global entity %s (%s)\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->globalname) );
			}
		}
	}

	return 0;
}

void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd)
{
	if (!pkvd || !pentKeyvalue)
		return;

	EntvarsKeyvalue(VARS(pentKeyvalue), pkvd);

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if (0 != pkvd->fHandled || pkvd->szClassName == NULL)
		return;

	// Get the actualy entity object
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentKeyvalue);

	if (!pEntity)
		return;

	pkvd->fHandled = static_cast<int32>(pEntity->KeyValue(pkvd));
}

void DispatchTouch(edict_t* pentTouched, edict_t* pentOther)
{
	if (gTouchDisabled)
		return;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentTouched);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity && pOther && ((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME) == 0)
		pEntity->Touch(pOther);
}


void DispatchUse(edict_t* pentUsed, edict_t* pentOther)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentUsed);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity && (pEntity->pev->flags & FL_KILLME) == 0)
		pEntity->Use(pOther, pOther, USE_TOGGLE, 0);
}

void DispatchThink(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
	if (pEntity)
	{
		if (FBitSet(pEntity->pev->flags, FL_DORMANT))
			ALERT(at_error, "Dormant entity %s is thinking!!\n", STRING(pEntity->pev->classname));

		pEntity->Think();
	}
}

void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pentBlocked);
	CBaseEntity* pOther = (CBaseEntity*)GET_PRIVATE(pentOther);

	if (pEntity)
		pEntity->Blocked(pOther);
}

void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity && CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		ENTITYTABLE* pTable = &pSaveData->pTable[pSaveData->currentIndex];

		if (pTable->pent != pent)
			ALERT(at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n");

		if ((pEntity->ObjectCaps() & FCAP_DONT_SAVE) != 0)
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if (pEntity->pev->movetype == MOVETYPE_PUSH)
		{
			float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
			pEntity->pev->ltime = gpGlobals->time;
			pEntity->pev->nextthink = pEntity->pev->ltime + delta;
		}

		pTable->location = pSaveData->size;			 // Remember entity position for file I/O
		pTable->classname = pEntity->pev->classname; // Remember entity class for respawn

		CSave saveHelper(*pSaveData);
		pEntity->Save(saveHelper);

		pTable->size = pSaveData->size - pTable->location; // Size of entity block is data size written to block
	}
}

void OnFreeEntPrivateData(edict_s* pEdict)
{
	if (pEdict && pEdict->pvPrivateData)
	{
		auto entity = reinterpret_cast<CBaseEntity*>(pEdict->pvPrivateData);

		delete entity;

		//Zero this out so the engine doesn't try to free it again.
		pEdict->pvPrivateData = nullptr;
	}
}

// Find the matching global entity.  Spit out an error if the designer made entities of
// different classes with the same global name
CBaseEntity* FindGlobalEntity(string_t classname, string_t globalname)
{
	edict_t* pent = FIND_ENTITY_BY_STRING(NULL, "globalname", STRING(globalname));
	CBaseEntity* pReturn = CBaseEntity::Instance(pent);
	if (pReturn)
	{
		if (!FClassnameIs(pReturn->pev, STRING(classname)))
		{
			ALERT(at_console, "Global entity found %s, wrong class %s\n", STRING(globalname), STRING(pReturn->pev->classname));
			pReturn = NULL;
		}
	}

	return pReturn;
}


int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity)
{
	gpGlobals->time = pSaveData->time;

	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);

	if (pEntity && CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper(*pSaveData);
		if (0 != globalEntity)
		{
			CRestore tmpRestore(*pSaveData);
			tmpRestore.PrecacheMode(false);
			tmpRestore.ReadEntVars("ENTVARS", &tmpVars);

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			// -------------------


			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(tmpVars.globalname);

			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if (!FStrEq(pSaveData->szCurrentMapName, pGlobal->levelName))
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity* pNewEntity = FindGlobalEntity(tmpVars.classname, tmpVars.globalname);
			if (pNewEntity)
			{
				//				ALERT( at_console, "Overlay %s with %s\n", STRING(pNewEntity->pev->classname), STRING(tmpVars.classname) );
				// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode(true); // Don't overwrite global fields
				pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
				pEntity = pNewEntity; // we're going to restore this data OVER the old entity
				pent = ENT(pEntity->pev);
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate(pEntity->pev->globalname, gpGlobals->mapname);
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}
		}

		if ((pEntity->ObjectCaps() & FCAP_MUST_SPAWN) != 0)
		{
			pEntity->Restore(restoreHelper);
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore(restoreHelper);
			pEntity->Precache();
		}

		// Again, could be deleted, get the pointer again.
		pEntity = (CBaseEntity*)GET_PRIVATE(pent);

#if 0
		if ( pEntity && !FStringNull(pEntity->pev->globalname) && 0 != globalEntity ) 
		{
			ALERT( at_console, "Global %s is %s\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model) );
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if (0 != globalEntity)
		{
			//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->pev->origin.x, pEntity->pev->origin.y, pEntity->pev->origin.z, STRING(pEntity->pev->model) );
			pSaveData->vecLandmarkOffset = oldOffset;
			if (pEntity)
			{
				UTIL_SetOrigin(pEntity->pev, pEntity->pev->origin);
				pEntity->OverrideReset();
			}
		}
		else if (pEntity && !FStringNull(pEntity->pev->globalname))
		{
			const globalentity_t* pGlobal = gGlobalState.EntityFromTable(pEntity->pev->globalname);
			if (pGlobal)
			{
				// Already dead? delete
				if (pGlobal->state == GLOBAL_DEAD)
					return -1;
				else if (!FStrEq(STRING(gpGlobals->mapname), pGlobal->levelName))
				{
					pEntity->MakeDormant(); // Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				ALERT(at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname));
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd(pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON);
			}
		}
	}
	return 0;
}


void DispatchObjectCollsionBox(edict_t* pent)
{
	CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(pent);
	if (pEntity)
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox(&pent->v);
}


void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	CSave saveHelper(*pSaveData);
	saveHelper.WriteFields(pname, pBaseData, pFields, fieldCount);
}


void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount)
{
	if (!CSaveRestoreBuffer::IsValidSaveRestoreData(pSaveData))
	{
		return;
	}

	// Always check if the player is stuck when loading a save game.
	g_CheckForPlayerStuck = true;

	CRestore restoreHelper(*pSaveData);
	restoreHelper.ReadFields(pname, pBaseData, pFields, fieldCount);
}

/**
 * \brief Heals the entity by the specified amount of health.
 *
 * This function increases the entity's health by the specified amount, up to the maximum health.
 * It will not heal the entity if it is set to not take damage or if the health is already at or above the maximum.
 *
 * \param flHealth The amount of health to add.
 * \param bitsDamageType An integer representing the type of damage (not used in this function).
 * \return True if the health was successfully added, otherwise false.
 */
bool CBaseEntity::TakeHealth(float flHealth, int bitsDamageType)
{
	if (GetTakeDamage() == DAMAGE_NO)
		return false;

	// heal
	if (GetHealth() >= GetMaxHealth())
		return false;

    float newHealth = GetHealth() + flHealth;
    if(newHealth > GetMaxHealth())
        SetHealth(GetMaxHealth());
    
    SetHealth(newHealth);
	return true;
}

/**
 * \brief Inflicts damage on this entity.
 *
 * This function handles the process of inflicting damage on the entity. It calculates the direction of the attack,
 * applies momentum based on the damage, and reduces the entity's health. If the entity's health drops to zero or below,
 * it calls the Killed function to handle the entity's death.
 *
 * \param pevInflictor Pointer to the entity variables of the inflictor (the source of the damage).
 * \param pevAttacker Pointer to the entity variables of the attacker (the entity causing the damage).
 * \param flDamage The amount of damage to inflict.
 * \param bitsDamageType An integer representing the type of damage inflicted (e.g., DMG_CRUSH).
 * \return True if the damage was successfully inflicted, otherwise false.
 */
bool CBaseEntity::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
    Vector vecTemp;

    if (!pev->takedamage)
        return false;

    // if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
    // (that is, no actual entity projectile was involved in the attack so use the shooter's origin).
    if (pevAttacker == pevInflictor) {
        vecTemp = pevInflictor->origin - VecBModelOrigin(pev);
    } else {
        // an actual missile was involved.
        vecTemp = pevInflictor->origin - VecBModelOrigin(pev);
    }

    // this global is still used for glass and other non-monster killables, along with decals.
    g_vecAttackDir = vecTemp.Normalize();

    // save damage based on the target's armor level

    // figure momentum add (don't let hurt brushes or other triggers move player)
    if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER)) {
        Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
        vecDir = vecDir.Normalize();

        float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

        if (flForce > 1000.0)
            flForce = 1000.0;
        pev->velocity = pev->velocity + vecDir * flForce;
    }

    // do the damage
    pev->health -= flDamage;
    if (pev->health <= 0) {
        Killed(pevAttacker, GIB_NORMAL);
        return false;
    }

    return true;
}

/**
 * \brief Handles the death of the entity.
 *
 * This function is called when the entity is killed. It sets the entity's damage mode to `DAMAGE_NO`,
 * marks the entity as dead by setting the dead flag to `DEAD_DEAD`, and then removes the entity from the game.
 *
 * \param pevAttacker Pointer to the entity variables of the attacker.
 * \param iGib Integer representing the gibbing mode (e.g., whether the entity should gib upon death).
 */
void CBaseEntity::Killed(entvars_t* pevAttacker, int iGib)
{
    SetTakeDamage(DAMAGE_NO);
    SetDeadFlag(DEAD_DEAD);
    UTIL_Remove(this);
}

/**
 * \brief Retrieves the next target entity based on the current entity's target field.
 *
 * This function searches for the next entity in the sequence based on the current entity's target field.
 * If the target field is null or the target entity is not found, it returns nullptr.
 *
 * \return A pointer to the next target entity, or nullptr if no target is found.
 */
CBaseEntity* CBaseEntity::GetNextTarget()
{
	if (FStringNull(pev->target))
		return nullptr;
    
	edict_t* pTarget = FIND_ENTITY_BY_TARGETNAME(nullptr, STRING(pev->target));
	if (FNullEnt(pTarget))
		return nullptr;

	return Instance(pTarget);
}

/**
 * \brief Sets the model for the entity.
 *
 * This function sets the model for the entity based on the provided model name.
 * If the model name is null or empty, a default null model is set.
 * If the model name starts with '*', it is treated as a brush model.
 * If the model file exists, it is set as the entity's model.
 * If the model file does not exist, an error model or sprite is set based on the file extension.
 *
 * \param pszModelName The name of the model to be set.
 */
void CBaseEntity::SetModel(const char* pszModelName)
{
    if (!FStringNull(pev->model))
        pszModelName = STRING(pev->model);

    if (!pszModelName || !(*pszModelName)) {
        g_engfuncs.pfnSetModel(edict(), "models/null.mdl");
        return;
    }

    // is this brush model?
    if (pszModelName[0] == '*') {
        g_engfuncs.pfnSetModel(edict(), pszModelName);
        return;
    }

    // verify file exists
    if (g_pFileSystem->FileExists(pszModelName)) {
        FileHandle_t file = g_pFileSystem->Open(pszModelName, "r");
        if (file != FILESYSTEM_INVALID_HANDLE && g_pFileSystem->Size(file) > 0) {
            g_pFileSystem->Close(file);
            g_engfuncs.pfnSetModel(edict(), pszModelName);
            return;
        }
        g_pFileSystem->Close(file);
    }

    const char* ext = UTIL_FileExtension(pszModelName);
    if (FStrEq(ext, "mdl")) {
        g_engfuncs.pfnSetModel(edict(), "models/error.mdl");
        return;
    }

    if (FStrEq(ext, "spr")) {
        g_engfuncs.pfnSetModel(edict(), "sprites/error.spr");
        return;
    }

    // set null model
    g_engfuncs.pfnSetModel(edict(), "models/null.mdl");
}

/**
 * \brief Precaches a model if it exists.
 *
 * This function precaches a model based on the provided model name.
 * If the model name is null or empty, a warning is logged and a null model index is returned.
 * If the model name starts with '*', it is treated as a brush model and no precaching is done.
 * If the model file exists, it is precached.
 * If the model file does not exist, an error model or sprite index is returned based on the file extension.
 *
 * \param pszModelName The name of the model to be precached.
 * \return The index of the precached model.
 */
auto CBaseEntity::PrecacheModel(const char* pszModelName) -> int {
    if (!pszModelName || !*pszModelName) {
        ALERT(at_console, "Warning: modelname not specified\n");
        return g_sModelIndexNullModel; // set null model
    }

    // no need to precache brush
    if (pszModelName[0] == '*') return 0;

    // verify file exists
    if (g_pFileSystem->FileExists(pszModelName)) {
        FileHandle_t file = g_pFileSystem->Open(pszModelName, "r");
        if (file != FILESYSTEM_INVALID_HANDLE && g_pFileSystem->Size(file) > 0) {
            g_pFileSystem->Close(file);
            return g_engfuncs.pfnPrecacheModel(pszModelName);
        }
        g_pFileSystem->Close(file);
    }

    const char* ext = UTIL_FileExtension(pszModelName);
    if (FStrEq(ext, "mdl")) {
        ALERT(at_console, "Warning: model \"%s\" not found!\n", pszModelName);
        return g_sModelIndexErrorModel;
    }

    if (FStrEq(ext, "spr")) {
        ALERT(at_console, "Warning: sprite \"%s\" not found!\n", pszModelName);
        return g_sModelIndexErrorSprite;
    }

    ALERT(at_console, "Warning: invalid name \"%s\"!\n", pszModelName);
    return g_sModelIndexNullModel; // set null model
}

/**
 * \brief Precaches a sound if it exists.
 *
 * This function precaches a sound based on the provided sound name.
 * If the sound name is null or empty, a null sound index is returned.
 * The function constructs the full path to the sound file and verifies its existence.
 * If the sound file exists, it is precached.
 * If the sound file does not exist, a warning is logged and a null sound index is returned.
 *
 * \param pszSoundName The name of the sound to be precached.
 * \return The index of the precached sound.
 */
auto CBaseEntity::PrecacheSound(const char* pszSoundName) -> int {
    if (!pszSoundName || !*pszSoundName)
        return g_sSoundIndexNullSound; // set null sound

    // Construct the full path to the sound file
    char path[256];
    const char* sound = (pszSoundName[0] == '*') ? pszSoundName + 1 : pszSoundName;
    sprintf(path, "sound/%s", sound);

    // Verify file exists
    if (g_pFileSystem->FileExists(path))
    {
        FileHandle_t file = g_pFileSystem->Open(path, "r");
        if (file != FILESYSTEM_INVALID_HANDLE && g_pFileSystem->Size(file) > 0)
        {
            g_pFileSystem->Close(file);
            return g_engfuncs.pfnPrecacheSound(pszSoundName);
        }
        g_pFileSystem->Close(file);
    }

    // Handle unknown formats
    if (FStrEq(UTIL_FileExtension(pszSoundName), "wav"))
    {
        ALERT(at_console, "Warning: sound \"%s\" not found!\n", path);
        return g_sSoundIndexNullSound; // set null sound
    }

    ALERT(at_console, "Warning: invalid name \"%s\"!\n", pszSoundName);
    return g_sSoundIndexNullSound; // set null sound
}

/**
 * \brief Preloads an array of sound files for the entity.
 *
 * This function iterates through an array of sound file names and preloads each one.
 * If any sound file is not found, a warning is logged to the console.
 *
 * \param soundFiles An array of sound file names to preload.
 * \param arraySize The number of elements in the soundFiles array.
 * \return True if all sounds were preloaded successfully, otherwise false.
 */
void CBaseEntity::PrecacheSoundArray(const char* soundFiles[],int arraySize) {
    for(int i = 0; i < arraySize; ++i) {
        PrecacheSound(soundFiles[i]);
    }
}

/**
 * \brief Precaches an event if it exists.
 *
 * This function attempts to precache an event based on the provided event name.
 * If the event file exists and is valid, it is precached.
 * If the event file does not exist, a warning is logged and a null event index is returned.
 *
 * \param type The type of the event to be precached.
 * \param psz The name of the event to be precached.
 * \return The index of the precached event, or a null event index if the event file does not exist.
 */
auto CBaseEntity::PrecacheEvent(int type, const char* psz) const -> unsigned short {
    if (g_pFileSystem->FileExists(psz)) {
        FileHandle_t file = g_pFileSystem->Open(psz, "r");
        if (file != FILESYSTEM_INVALID_HANDLE && g_pFileSystem->Size(file) > 0) {
            g_pFileSystem->Close(file);
            return g_engfuncs.pfnPrecacheEvent(type, psz);
        }
        g_pFileSystem->Close(file);
    }

    ALERT(at_console, "Warning: event \"%s\" not found!\n", psz);
    return g_sEventIndexNullEvent;
}

// Global Savedata for Delay
TYPEDESCRIPTION CBaseEntity::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR),
		DEFINE_FIELD(CBaseEntity, m_EFlags, FIELD_CHARACTER),

		DEFINE_FIELD(CBaseEntity, m_pfnThink, FIELD_FUNCTION), // UNDONE: Build table of these!!!
		DEFINE_FIELD(CBaseEntity, m_pfnTouch, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnUse, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseEntity, m_pfnBlocked, FIELD_FUNCTION),
};


bool CBaseEntity::Save(CSave& save)
{
	if (save.WriteEntVars("ENTVARS", pev))
		return save.WriteFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	return false;
}

bool CBaseEntity::Restore(CRestore& restore)
{

    bool status = restore.ReadEntVars("ENTVARS", pev);
	if (status)
		status = restore.ReadFields("BASE", this, m_SaveData, ARRAYSIZE(m_SaveData));

	if (pev->modelindex != 0 && !FStringNull(pev->model))
	{
        Vector mins = pev->mins; // Set model is about to destroy these
		Vector maxs = pev->maxs;
	    
		PrecacheModel(STRING(pev->model));
		SetModel(STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs); // Reset them
	}

	return status;
}

/**
 * \brief Initializes the absolute minimum and maximum bounds of the entity.
 *
 * This function sets the absolute minimum (absmin) and absolute maximum (absmax) bounds of the entity
 * based on its current position, size, and rotation. If the entity is a BSP model and has non-zero angles,
 * the bounds are expanded to account for rotation.
 *
 * \param pev Pointer to the entity variables structure.
 */
void SetObjectCollisionBox(entvars_t* pev)
{
    constexpr float EXPANSION_VALUE = 1.0f;
    
    if ((pev->solid == SOLID_BSP) && (pev->angles != g_vecZero))
    { // expand for rotation
        int i;
        float max = 0;
        for (i = 0; i < 3; i++)
        {
            float v = fabs(pev->mins[i]);
            if (v > max)
                max = v;
            v = fabs(pev->maxs[i]);
            if (v > max)
                max = v;
        }
        
        for (i = 0; i < 3; i++)
        {
            pev->absmin[i] = pev->origin[i] - max;
            pev->absmax[i] = pev->origin[i] + max;
        }
    }
    else
    {
        pev->absmin = pev->origin + pev->mins;
        pev->absmax = pev->origin + pev->maxs;
    }
    
    pev->absmin -= Vector(EXPANSION_VALUE, EXPANSION_VALUE, EXPANSION_VALUE);
    pev->absmax += Vector(EXPANSION_VALUE, EXPANSION_VALUE, EXPANSION_VALUE);
}

/**
 * \brief Sets the collision box for the entity.
 *
 * This function sets the collision box for the entity by calling the global 
 * SetObjectCollisionBox function with the entity's pev (pointer to entity variables).
 */
void CBaseEntity::SetObjectCollisionBox()
{
    ::SetObjectCollisionBox(pev);
}

/**
 * \brief Checks if this entity intersects with another entity.
 *
 * This function determines if the bounding boxes of this entity and another entity intersect.
 *
 * \param pOther The other entity to check for intersection.
 * \return True if the entities intersect, otherwise false.
 */
bool CBaseEntity::Intersects(const CBaseEntity* pOther) const
{
    return !(pOther->GetAbsMin().x > GetAbsMax().x ||
             pOther->GetAbsMin().y > GetAbsMax().y ||
             pOther->GetAbsMin().z > GetAbsMax().z ||
             pOther->GetAbsMax().x < GetAbsMin().x ||
             pOther->GetAbsMax().y < GetAbsMin().y ||
             pOther->GetAbsMax().z < GetAbsMin().z);
}

/**
 * \brief Makes the entity dormant.
 *
 * This function sets the entity to a dormant state by modifying its flags, solidity, movement type, effects, and think time.
 * It also updates the entity's origin to ensure it is properly linked in the game world.
 */
void CBaseEntity::MakeDormant()
{
    SetFlag(FL_DORMANT); // Set the dormant flag
    SetSolid(SOLID_NOT); // Don't touch
    SetMovetype(MOVETYPE_NONE); // Don't move
    SetEffect(EF_NODRAW); // Don't draw
    DontThink(); // Don't think
    UTIL_SetOrigin(pev, GetOrigin()); // Relink
}

/**
 * \brief Checks if the entity is within the world boundaries and speed limits.
 *
 * This function verifies that the entity's position and velocity are within acceptable limits.
 *
 * \return True if the entity is within the world boundaries and speed limits, otherwise false.
 */
bool CBaseEntity::IsInWorld()
{
    // Check position boundaries
    if (fabs(pev->origin.x) >= 4096 || fabs(pev->origin.y) >= 4096 || fabs(pev->origin.z) >= 4096)
        return false;

    // Check speed limits
    if (fabs(pev->velocity.x) >= 2000 || fabs(pev->velocity.y) >= 2000 || fabs(pev->velocity.z) >= 2000)
        return false;

    return true;
}

/**
 * \brief Determines if the entity should toggle its state based on the use type and current state.
 *
 * This function checks the use type and current state of the entity to decide if the state should be toggled.
 * It returns true if the state should be toggled, otherwise false.
 *
 * \param useType The type of use action being performed (e.g., USE_TOGGLE, USE_ON, USE_OFF).
 * \param currentState The current state of the entity (true if on, false if off).
 * \return True if the state should be toggled, otherwise false.
 */
bool CBaseEntity::ShouldToggle(USE_TYPE useType, bool currentState)
{
	if (useType != USE_TOGGLE && useType != USE_SET)
	{
		if ((currentState && useType == USE_ON) || (!currentState && useType == USE_OFF))
			return false;
	}
	return true;
}

/**
 * \brief Determines the type of damage decal to apply based on the damage type.
 *
 * This function returns the appropriate decal index to use for the given damage type.
 * It considers the entity's render mode to decide which decal to apply.
 *
 * \param bitsDamageType The type of damage inflicted, represented as a bitmask.
 * \return The index of the decal to apply, or -1 if no decal should be applied.
 */
int CBaseEntity::DamageDecal(int bitsDamageType)
{
    if (GetRenderMode() == kRenderTransAlpha)
        return -1;

    if (GetRenderMode() != kRenderNormal)
        return DECAL_BPROOF1;

    return DECAL_GUNSHOT1 + RANDOM_LONG(0, 4);
}

/**
 * \brief Preloads essential models, sounds, and events for the entity.
 *
 * This function preloads a set of default models, sounds, and events that are 
 * essential for the entity's operation. If any of these resources are not found, 
 * a warning is logged to the console, and a flag indicating a pre-cache error is set.
 */
void CBaseEntity::Precache()
{
    // Preload the null model
    g_sModelIndexNullModel = g_engfuncs.pfnPrecacheModel("models/null.mdl");
    // Preload the error model
    g_sModelIndexErrorModel = g_engfuncs.pfnPrecacheModel("models/error.mdl");
    // Preload the error sprite
    g_sModelIndexErrorSprite = g_engfuncs.pfnPrecacheModel("sprites/error.spr");
    // Preload the null sound
    g_sSoundIndexNullSound = g_engfuncs.pfnPrecacheSound("null.wav");
    // Preload the null event
    g_sEventIndexNullEvent = g_engfuncs.pfnPrecacheEvent(1, "events/null.sc");

    // Check if the null event was not found
    if(g_sEventIndexNullEvent == 0)
    {
        ALERT(at_console, "Warning: event \"events/null.sc\" not found!\n");
        m_bHasPreacheError = true;
        return;
    }

    // Check if the null model was not found
    if(g_sModelIndexNullModel == 0)
    {
        ALERT(at_console, "Warning: model \"models/null.mdl\" not found!\n");
        m_bHasPreacheError = true;
        return;
    }

    // Check if the error model was not found
    if(g_sModelIndexErrorModel == 0)
    {
        ALERT(at_console, "Warning: model \"models/error.mdl\" not found!\n");
        m_bHasPreacheError = true;
        return;
    }

    // Check if the error sprite was not found
    if(g_sModelIndexErrorSprite == 0)
    {
        ALERT(at_console, "Warning: sprite \"sprites/error.spr\" not found!\n");
        m_bHasPreacheError = true;
        return;
    }

    // Check if the null sound was not found
    if(g_sSoundIndexNullSound == 0)
    {
        ALERT(at_console, "Warning: sound \"sound/null.wav\" not found!\n");
        m_bHasPreacheError = true;
    }
}

/**
 * \brief Creates a new entity of the specified class name at the given origin and angles.
 *
 * This function creates a new entity in the game world by its class name, sets its origin and angles,
 * assigns an owner if provided, and then spawns the entity.
 *
 * \param szName The class name of the entity to create. Must be a pointer to constant memory.
 * \param vecOrigin The origin (position) where the entity should be created.
 * \param vecAngles The angles (rotation) for the entity.
 * \param pentOwner The owner of the entity, if any. Can be nullptr.
 * \return A pointer to the created entity, or nullptr if the entity could not be created.
 */
CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner)
{
    // Create a new entity by its class name
    edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING(szName));
    
    // Check if the entity creation failed
    if (FNullEnt(pent))
    {
        ALERT(at_console, "NULL Ent in Create!\n");
        return nullptr;
    }
    
    // Get the instance of the created entity
    CBaseEntity* pEntity = Instance(pent);
    
    // Set the owner, origin, and angles of the entity
    pEntity->SetOwner(pentOwner);
    pEntity->SetOrigin(vecOrigin);
    pEntity->SetAngles(vecAngles);
    
    // Spawn the entity in the game world
    DispatchSpawn(pEntity->edict());
    return pEntity;
}
