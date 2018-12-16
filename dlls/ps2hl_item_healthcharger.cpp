// Includes
#include "ps2hl_item_healthcharger.h"

//================================
// Bottle for charger
//================================

// Link entity to class
LINK_ENTITY_TO_CLASS(item_healthcharger_bottle, CHealthBottle);

// Methods //

void CHealthBottle::Precache(void)
{
	PRECACHE_MODEL("models/health_charger_both.mdl");
}

void CHealthBottle::Spawn(void)
{
	// Precache
	Precache();

	// Set model
	SET_MODEL(ENT(pev), "models/health_charger_both.mdl");

	// Properties
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("item_healthcharger_bottle");
	pev->solid = SOLID_NOT;
	
	// BBox
	Vector Zero;
	Zero.x = Zero.y = Zero.z = 0;
	UTIL_SetSize(pev, Zero, Zero);

	// Visuals
	pev->rendermode = kRenderTransTexture;		// Mode with transparency
	pev->renderamt = HCHG_GLASS_TRANSPARENCY;	// Transparency amount

	// Initial animation
	pev->sequence = HBOTTLE_SEQ_IDLE;
	pev->frame = 0;

	// Set think delay
	pev->nextthink = gpGlobals->time + HCHG_DELAY_THINK;
}

void CHealthBottle::Think(void)
{
	// Set delay for next think
	pev->nextthink = gpGlobals->time + HCHG_DELAY_THINK;

	// Call animation handler
	StudioFrameAdvance(0);

	// State handler
	switch (pev->sequence)
	{
	//case HBOTTLE_SEQ_IDLE:
	//	// Idle - do nothing
	//	break;
	case HBOTTLE_SEQ_USE:
		// Reset frame counter to loop
		if (m_fSequenceFinished)
		{
			m_fSequenceFinished = false;
			pev->frame = 0;
		}
		break;
	case HBOTTLE_SEQ_STOP:
		// Just wait until animation is played
		if (m_fSequenceFinished)
			ChangeSequence(HBOTTLE_SEQ_IDLE);
		break;
	default:
		// Do nothing
		break;
	}
}

void CHealthBottle::ChangeSequence(int NewSequence)
{
	if (pev->sequence == NewSequence)
		return;

	// Set sequence
	pev->sequence = NewSequence;

	// Prepare sequence
	pev->frame = 0;
	ResetSequenceInfo();
}

void CHealthBottle::SetLevel(float Level)
{
	// Convert level to angle
	Level = HBOTTLE_CTRL_LVL_MIN + Level * (HBOTTLE_CTRL_LVL_MAX - HBOTTLE_CTRL_LVL_MIN);

	// Set goo level controller
	SetBoneController(HBOTTLE_CTRL_LVL, Level);
}

//================================
// Charger itself
//================================

// Link entity to class
LINK_ENTITY_TO_CLASS(item_healthcharger, CItemHealthCharger);

// Save/restore
TYPEDESCRIPTION CItemHealthCharger::m_SaveData[] =
{
	DEFINE_FIELD(CItemHealthCharger, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD(CItemHealthCharger, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD(CItemHealthCharger, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD(CItemHealthCharger, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD(CItemHealthCharger, m_flSoundTime, FIELD_TIME),

	DEFINE_FIELD(CItemHealthCharger, IsUsed, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemHealthCharger, IsFrontAngle, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemHealthCharger, CurrentState, FIELD_INTEGER),
	DEFINE_FIELD(CItemHealthCharger, pBottle, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemHealthCharger, CBaseButton);

// Methods //

void CItemHealthCharger::Precache(void)
{
	// Precache model
	PRECACHE_MODEL("models/health_charger_body.mdl");

	// Precache sounds
	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/medcharge4.wav");
}


void CItemHealthCharger::Spawn(void)
{
	// Precache models & sounds
	Precache();

	// Set model
	SET_MODEL(ENT(pev), "models/health_charger_body.mdl");

	// Set up BBox and origin
	pev->solid = SOLID_BBOX;//SOLID_TRIGGER;
	SetSequenceBox();
	UTIL_SetOrigin(pev, pev->origin);

	// Reset bone controllers
	InitBoneControllers();
	RotateCamArm(NULL);

	// Set move type
	//pev->movetype = MOVETYPE_PUSH;	// This was default for "func_healthcharger", but it breaks "nextthink" and bone controllers
	//pev->movetype = MOVETYPE_NONE;	// No bone controller interpolation in GoldSrc (but OK in Xash)
	pev->movetype = MOVETYPE_FLY;		// This type enables bone controller interpolation in GoldSrc

	// Get initial charge
	m_iJuice = gSkillData.healthchargerCapacity;

	// Set "on" skin
	pev->skin = 0;

	// Reset state
	ChangeState(HCHG_STATE_IDLE, HCHG_SEQ_IDLE);

	// Spawn bottle entity
	pBottle = GetClassPtr((CHealthBottle *)NULL);
	pBottle->Spawn();
	UTIL_SetOrigin(pBottle->pev, pev->origin);
	pBottle->pev->angles = pev->angles;
	pBottle->pev->owner = ENT(pev);
	pBottle->SetLevel(1.0f);

	// Set think delay
	pev->nextthink = gpGlobals->time + HCHG_DELAY_THINK;
}

void CItemHealthCharger::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseButton::KeyValue(pkvd);
}

void CItemHealthCharger::Think(void)
{
	// Debug: show bounding box
	//PS2HL_DEBUG(DBG_RenderBBox(pev->origin, pev->mins, pev->maxs, ceil(HCHG_DELAY_THINK / 0.1), 0xFF, 0x00, 0x00));

	// Get current time
	float CurrentTime = gpGlobals->time;

	// Set delay for next think
	pev->nextthink = CurrentTime + HCHG_DELAY_THINK;

	// Call animation handler
	StudioFrameAdvance(0);

	// DEBUG
	//static float LastTime2 = 0;
	//pev->nextthink = gpGlobals->time + HCHG_DELAY_THINK;
	//UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("Next: %.1f \nReal: %.1f\n", pev->nextthink - gpGlobals->time, gpGlobals->time - LastTime2)); // Message
	//LastTime2 = gpGlobals->time;

	// Update goo level
	if (IsUsed)
		pBottle->SetLevel(m_iJuice / gSkillData.healthchargerCapacity);

	// Find player
	CBaseEntity * pPlayer = FindPlayer(HCHG_ACTIVE_RADIUS);

	// Track player
	if (CurrentState != HCHG_STATE_EMPTY && CurrentState != HCHG_STATE_DEACTIVATE)
		RotateCamArm(pPlayer);
	else
		RotateCamArm(NULL);

	// State handler
	switch (CurrentState)
	{
	case HCHG_STATE_IDLE:
		// Player found?
		if (pPlayer != NULL)
		{
			// Player is near

			#ifdef HCHG_NO_BACK_USE
			// Prevent back side usage
			if (!IsFrontAngle)
				return;
			#endif

			#ifdef HCHG_SUIT_CHECK
			// Prevent deployment if player doesn't have HEV
			if (~pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			{
				RotateCamArm(NULL);
				return;
			}
			#endif

			#ifdef HCHG_ACTIVATE_SOUND
			// Emit sound
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 0.85, ATTN_NORM);
			#endif

			// Change state
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: activate\n\n\n"));
			ChangeState(HCHG_STATE_ACTIVATE, HCHG_SEQ_DEPLOY);
		}
		break;
	case HCHG_STATE_ACTIVATE:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: ready\n\n\n"));
			ChangeState(HCHG_STATE_READY, HCHG_SEQ_RDY);
		}
		break;
	case HCHG_STATE_READY:
		// Player is near?
		if (pPlayer == NULL)
		{
			// Nobody is near, hibernate
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: deactivate\n\n\n"));
			ChangeState(HCHG_STATE_DEACTIVATE, HCHG_SEQ_HIBERNATE);
		}
		else
		{
			#ifdef HCHG_NO_BACK_USE
			// Prevent back side usage
			if (!IsFrontAngle)
			{
				ChangeState(HCHG_STATE_DEACTIVATE, HCHG_SEQ_HIBERNATE);
				return;
			}
			#endif

			// Check if used
			if (IsUsed)
			{
				PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: used\n\n\n"));
				ChangeState(HCHG_STATE_START, HCHG_SEQ_EXPAND);
				pBottle->ChangeSequence(HBOTTLE_SEQ_USE);
			}
		}
		break;
	case HCHG_STATE_START:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: giving charge\n\n\n"));
			ChangeState(HCHG_STATE_USE, HCHG_SEQ_USE);
		}
		break;
	case HCHG_STATE_USE:
		if (IsUsed == false)
		{
			// No longer used - change state
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: stopping\n\n\n"));
			ChangeState(HCHG_STATE_STOP, HCHG_SEQ_RETRACT);
			pBottle->ChangeSequence(HBOTTLE_SEQ_STOP);
		}

		// Reset use flag
		IsUsed = false;
		break;
	case HCHG_STATE_STOP:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: ready\n\n\n"));
			Off();
			ChangeState(HCHG_STATE_READY, HCHG_SEQ_RDY);
		}
		break;
	case HCHG_STATE_DEACTIVATE:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: to idle\n\n\n"));
			ChangeState(HCHG_STATE_IDLE, HCHG_SEQ_IDLE);
		}
		break;
	case HCHG_STATE_EMPTY:
		// Swap animation to hibernate
		if (pev->sequence == HCHG_SEQ_RETRACT)
			if (m_fSequenceFinished)
				ChangeSequence(HCHG_SEQ_HIBERNATE);

		// Swap animation to idle
		if (pev->sequence == HCHG_SEQ_HIBERNATE)
			if (m_fSequenceFinished)
				ChangeSequence(HCHG_SEQ_IDLE);

		// Wait for next recharge (multiplayer)
		if (m_flNextCharge != -1 && CurrentTime > m_flNextCharge)
		{
			Recharge();
			ChangeState(HCHG_STATE_IDLE, HCHG_SEQ_IDLE);
		}
		break;
	default:
		// Do nothing
		break;
	}
}

void CItemHealthCharger::ChangeSequence(int NewSequence)
{
	// Set sequence
	pev->sequence = NewSequence;

	// Prepare sequence
	pev->frame = 0;
	ResetSequenceInfo();
}

void CItemHealthCharger::ChangeState(RechargeState NewState, int NewSequence)
{
	CurrentState = NewState;
	//CurrentStateTime = gpGlobals->time;	// I found no use for it so far
	ChangeSequence(NewSequence);
}

void CItemHealthCharger::RotateCamArm(CBaseEntity * pPlayer)
{
	float Angle;
	Vector ChToPl;

	if (pPlayer)
	{
		// Calculate angle
		ChToPl.x = pev->origin.x - pPlayer->pev->origin.x;
		ChToPl.y = pev->origin.y - pPlayer->pev->origin.y;
		ChToPl.z = pev->origin.z - pPlayer->pev->origin.z;
		ChToPl = UTIL_VecToAngles(ChToPl);
		Angle = ChToPl.y - pev->angles.y + 180;	// +180 - because player actually faces back side of the charger model

		// Angle correction
		if (Angle > 180)
		{
			while (Angle > 180)
				Angle -= 360;
		}
		else if (Angle < -180)
		{
			while (Angle < -180)
				Angle += 360;
		}

		#ifdef HCHG_NO_BACK_USE
		// Check if player is on front or in the back of the charger
		if (-90 > Angle || Angle > 90)
		{
			IsFrontAngle = false;
			Angle = 0;
		}
		else
			IsFrontAngle = true;
		#endif
	}
	else
	{
		Angle = 0;
	}

	// Rotate bone controllers
	SetBoneController(HCHG_CTRL_ARM, Angle);
	SetBoneController(HCHG_CTRL_CAM, Angle);
}

CBaseEntity * CItemHealthCharger::FindPlayer(float Radius)
{
	CBaseEntity * pPlayer = NULL;	// Result
	CBaseEntity * pEntity = NULL;	// Used for search

	//// Find near entities
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, HCHG_ACTIVE_RADIUS)) != NULL)
	{
		if (pEntity->IsPlayer())
		{
			// Found player - prepare result and terminate search
			pPlayer = pEntity;
			break;
		}
	}

	// Find near entities
	//while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	//{
	//	Vector Dist = pev->origin - pEntity->pev->origin;
	//	if (Dist.Length2D() < HCHG_ACTIVE_RADIUS)
	//	{
	//		// Found player - prepare result and terminate search
	//		pPlayer = pEntity;
	//		break;
	//	}
	//}

	// Return result
	return pPlayer;
}

// Based on "func_healthcharger"
void CItemHealthCharger::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;

	#ifdef HCHG_NO_BACK_USE
	// PS2HL - prevent back side usage
	if (!IsFrontAngle)
		return;
	#endif

	#ifdef HCHG_NO_BACK_USE
	// Check distance
	Vector Dist = pev->origin - pActivator->pev->origin;
	if (Dist.Length2D() > HCHG_USE_RADIUS)
	{
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: player is too far ...\n\n\n"));
		return;
	}
	#endif

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		// PS2HL
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_healthcharger: empty\n\n\n"));
		pev->skin = 1;
		Off();
		if (CurrentState != HCHG_STATE_EMPTY)
		{
			ChangeState(HCHG_STATE_EMPTY, HCHG_SEQ_RETRACT);
			pBottle->ChangeSequence(HBOTTLE_SEQ_STOP);

			// Set up recharge
			if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime()) > 0))
				m_flNextCharge = gpGlobals->time + m_iReactivate;
			else
				m_flNextCharge = -1;	// No recharge
		}
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || (!(pActivator->pev->weapons & (1 << WEAPON_SUIT))))
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshotno1.wav", 1.0, ATTN_NORM);
		}
		return;
	}

	//=============
	// PS2HL
	//=============
	// Speed up deploy if used early
	if (CurrentState != HCHG_STATE_START && CurrentState != HCHG_STATE_USE)
	{
		ChangeState(HCHG_STATE_START, HCHG_SEQ_EXPAND);
		pBottle->ChangeSequence(HBOTTLE_SEQ_USE);
	}

	// Set use flag
	IsUsed = true;
	//=============


	//pev->nextthink = pev->ltime + 0.25;
	//SetThink(&CItemHealthCharger::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM);
	}


	// charge the player
	if (pActivator->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

// Based on "func_healthcharger"
void CItemHealthCharger::Recharge(void)
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
	m_iJuice = gSkillData.healthchargerCapacity;
	pev->skin = 0;
	pBottle->SetLevel(1.0f);
	//SetThink(&CBaseEntity::SUB_DoNothing);
}

// Based on "func_healthcharger"
void CItemHealthCharger::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
		STOP_SOUND(ENT(pev), CHAN_STATIC, "items/medcharge4.wav");

	IsUsed = false;
	m_iOn = 0;

	if (m_iJuice <= 0)
		PS2HL_DEBUG(ALERT(at_console, "Next recharge in: %.0f s", m_flNextCharge - gpGlobals->time));
}

// Extract BBox from sequence (ripped from CBaseAnimating with one little change)
void CItemHealthCharger::SetSequenceBox(void)
{
	Vector mins, maxs;

	// Get sequence bbox
	if (ExtractBbox(pev->sequence, mins, maxs))
	{
		#ifdef HCHG_HACKED_BBOX
			// PS2HL - hack for GoldSource:
			// it makes no sence, but BBox should have max size of 4 in horisontal
			// axes, otherwise nearby doors and elevators can freak out
			mins.x = -4;
			maxs.x = 2;
			mins.y = -4;
			maxs.y = 4;
		#endif

		// expand box for rotation
		// find min / max for rotations
		float yaw = pev->angles.y * (M_PI / 180.0);

		Vector xvector, yvector;
		xvector.x = cos(yaw);
		xvector.y = sin(yaw);
		yvector.x = -sin(yaw);
		yvector.y = cos(yaw);
		Vector bounds[2];

		bounds[0] = mins;
		bounds[1] = maxs;

		Vector rmin(9999, 9999, 9999);
		Vector rmax(-9999, -9999, -9999);
		Vector base, transformed;

		for (int i = 0; i <= 1; i++)
		{
			base.x = bounds[i].x;
			for (int j = 0; j <= 1; j++)
			{
				base.y = bounds[j].y;
				for (int k = 0; k <= 1; k++)
				{
					base.z = bounds[k].z;

					// transform the point
					transformed.x = xvector.x*base.x + yvector.x*base.y;
					transformed.y = xvector.y*base.x + yvector.y*base.y;
					transformed.z = base.z;

					if (transformed.x < rmin.x)
						rmin.x = transformed.x;
					if (transformed.x > rmax.x)
						rmax.x = transformed.x;
					if (transformed.y < rmin.y)
						rmin.y = transformed.y;
					if (transformed.y > rmax.y)
						rmax.y = transformed.y;
					if (transformed.z < rmin.z)
						rmin.z = transformed.z;
					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}
		//rmin.z = 0;
		//rmax.z = rmin.z + 1;
		UTIL_SetSize(pev, rmin, rmax);
	}
}