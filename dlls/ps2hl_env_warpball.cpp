/*

28.08.18

"env_warpball" entity for ps2hl

Combination of "monstermaker" and this code:
https://github.com/FWGS/hlsdk-xash3d/blob/blueshift/dlls/effects.cpp

*/

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "monsters.h"	// Needed for "monstermaker"
#include "effects.h"	// CBeam

//=========================================================
// env_warpball
//=========================================================
#define SF_REMOVE_ON_FIRE	0x0001
#define SF_KILL_CENTER		0x0002

// PS2HL - spawn delay
#define EWB_DELAY_SPAWN 1.0f	// PS2 HL has around 1s delay

class CEnvWarpBall : public CBaseEntity
{
public:
	// From Xash BShift
	void Precache(void);
	void Spawn(void) { Precache(); }
	void Think(void);
	void KeyValue(KeyValueData *pkvd);
	void Telefrag(bool PlayerOnly);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	Vector vecOrigin;

	// From "monstermaker"
	void MakeMonster(void);

	string_t m_iszMonsterClassname;// classname of the monster(s) that will be created.
	int	 m_cNumMonsters;// max number of monsters this ent can create
	int  m_cLiveChildren;// how many monsters made by this monster maker that are currently alive
	int	 m_iMaxLiveChildren;// max number of monsters that this maker may have out at one time.
	float m_flGround; // z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	static	TYPEDESCRIPTION m_SaveData[];
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
};

LINK_ENTITY_TO_CLASS(env_warpball, CEnvWarpBall)

// Save/restore from "monstermaker"
TYPEDESCRIPTION	CEnvWarpBall::m_SaveData[] =
{
	DEFINE_FIELD(CEnvWarpBall, m_iszMonsterClassname, FIELD_STRING),
	DEFINE_FIELD(CEnvWarpBall, m_cNumMonsters, FIELD_INTEGER),
	DEFINE_FIELD(CEnvWarpBall, m_cLiveChildren, FIELD_INTEGER),
	DEFINE_FIELD(CEnvWarpBall, m_flGround, FIELD_FLOAT),
	DEFINE_FIELD(CEnvWarpBall, m_iMaxLiveChildren, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CEnvWarpBall, CBaseEntity);

void CEnvWarpBall::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->button = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "warp_target"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damage_delay"))
	{
		pev->frags = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "monstercount"))
	{
		m_cNumMonsters = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_imaxlivechildren"))
	{
		m_iMaxLiveChildren = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "monstertype"))
	{
		m_iszMonsterClassname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CEnvWarpBall::Precache(void)
{
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_MODEL("sprites/Fexplo1.spr");
	PRECACHE_SOUND("debris/beamstart2.wav");
	PRECACHE_SOUND("debris/beamstart7.wav");

	// PS2HL: precache monster (if set)
	if (m_iszMonsterClassname)
		UTIL_PrecacheOther(STRING(m_iszMonsterClassname));
}

void CEnvWarpBall::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	int iTimes = 0;
	int iDrawn = 0;
	TraceResult tr;
	Vector vecDest;
	CBeam *pBeam;
	CBaseEntity *pEntity = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
	edict_t *pos;

	if (pEntity)//target found ?
	{
		vecOrigin = pEntity->pev->origin;
		pos = pEntity->edict();
	}
	else
	{
		//use as center
		vecOrigin = pev->origin;
		pos = edict();
	}
	EMIT_SOUND(pos, CHAN_BODY, "debris/beamstart2.wav", 1, ATTN_NORM);
	UTIL_ScreenShake(vecOrigin, 6, 160, 1.0, pev->button);
	CSprite *pSpr = CSprite::SpriteCreate("sprites/Fexplo1.spr", vecOrigin, TRUE);
	pSpr->AnimateAndDie(18);
	pSpr->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);
	EMIT_SOUND(pos, CHAN_ITEM, "debris/beamstart7.wav", 1, ATTN_NORM);
	int iBeams = RANDOM_LONG(20, 40);
	while (iDrawn < iBeams && iTimes < (iBeams * 3))
	{
		vecDest = pev->button * (Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize());
		UTIL_TraceLine(vecOrigin, vecOrigin + vecDest, ignore_monsters, NULL, &tr);
		if (tr.flFraction != 1.0)
		{
			// we hit something.
			iDrawn++;
			pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 200);
			pBeam->PointsInit(vecOrigin, tr.vecEndPos);
			pBeam->SetColor(20, 243, 20);
			pBeam->SetNoise(65);
			pBeam->SetBrightness(220);
			pBeam->SetWidth(30);
			pBeam->SetScrollRate(35);
			pBeam->SetThink(&CBeam::SUB_Remove);
			pBeam->pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.5, 1.6);
		}
		iTimes++;
	}
	
	// PS2HL: if monster isn't specified then kill all only once
	if (!m_iszMonsterClassname)
		Telefrag(false);

	pev->nextthink = gpGlobals->time + EWB_DELAY_SPAWN;//pev->frags;
}

void CEnvWarpBall::Think(void)
{
	SUB_UseTargets(this, USE_TOGGLE, 0);

	// PS2HL: if monster isn't specified then kill only the player
	if (m_iszMonsterClassname)
		Telefrag(false);
	else
		Telefrag(true);

	// PS2HL: spawn monster
	if (m_iszMonsterClassname)
		MakeMonster();

	if (pev->spawnflags & SF_REMOVE_ON_FIRE)
		UTIL_Remove(this);
}

void CEnvWarpBall::Telefrag(bool PlayerOnly)
{
	if (pev->spawnflags & SF_KILL_CENTER)
	{
		CBaseEntity *pMonster = NULL;

		while ((pMonster = UTIL_FindEntityInSphere(pMonster, vecOrigin, 72)) != NULL)
		{
			// PS2HL: kill only the player
			if (PlayerOnly && !pMonster->IsPlayer())
				continue;

			// PS2HL: increased dmg to guarantee kill
			if (FBitSet(pMonster->pev->flags, FL_MONSTER | FL_CLIENT))
				pMonster->TakeDamage(pev, pev, 1000, DMG_GENERIC);
		}
	}
}

// From "monstermaker"
void CEnvWarpBall::MakeMonster(void)
{
	edict_t	*pent;
	entvars_t		*pevCreate;

	if (m_iMaxLiveChildren > 0 && m_cLiveChildren >= m_iMaxLiveChildren)
	{// not allowed to make a new one yet. Too many live ones out right now.
		return;
	}

	if (!m_flGround)
	{
		// set altitude. Now that I'm activated, any breakables, etc should be out from under me. 
		TraceResult tr;

		UTIL_TraceLine(pev->origin, pev->origin - Vector(0, 0, 2048), ignore_monsters, ENT(pev), &tr);
		m_flGround = tr.vecEndPos.z;
	}

	Vector mins = pev->origin - Vector(34, 34, 0);
	Vector maxs = pev->origin + Vector(34, 34, 0);
	maxs.z = pev->origin.z;
	mins.z = m_flGround;

	// PS2HL: no need for this
	//CBaseEntity *pList[2];
	//int count = UTIL_EntitiesInBox(pList, 2, mins, maxs, FL_CLIENT | FL_MONSTER);
	//if (count)
	//{
	//	// don't build a stack of monsters!
	//	return;
	//}

	pent = CREATE_NAMED_ENTITY(m_iszMonsterClassname);

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in MonsterMaker!\n");
		return;
	}

	// If I have a target, fire!
	if (!FStringNull(pev->target))
	{
		// delay already overloaded for this entity, so can't call SUB_UseTargets()
		FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
	}

	pevCreate = VARS(pent);
	pevCreate->origin = pev->origin;
	pevCreate->angles = pev->angles;
	SetBits(pevCreate->spawnflags, SF_MONSTER_FALL_TO_GROUND);

	// Children hit monsterclip brushes
	//if (pev->spawnflags & SF_MONSTERMAKER_MONSTERCLIP)
	//	SetBits(pevCreate->spawnflags, SF_MONSTER_HITMONSTERCLIP);

	DispatchSpawn(ENT(pevCreate));
	pevCreate->owner = edict();

	if (!FStringNull(pev->netname))
	{
		// if I have a netname (overloaded), give the child monster that name as a targetname
		pevCreate->targetname = pev->netname;
	}

	m_cLiveChildren++;// count this monster
	m_cNumMonsters--;

	if (m_cNumMonsters == 0)
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink(NULL);
		SetUse(NULL);
	}
}
