// Includes
#include "ps2hl_item_recharge.h"

//================================
// Glass for charger
//================================

// Link entity to class
LINK_ENTITY_TO_CLASS(item_recharge_glass, CChargerGlass);

void CChargerGlass::Precache(void)
{
	PRECACHE_MODEL("models/field.mdl");
}

void CChargerGlass::Spawn(void)
{
	// Precache
	Precache();

	// Set model
	SET_MODEL(ENT(pev), "models/field.mdl");

	// Properties
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("item_recharge_glass");
	pev->solid = SOLID_NOT;

	// BBox
	Vector Zero;
	Zero.x = Zero.y = Zero.z = 0;
	UTIL_SetSize(pev, Zero, Zero);

	// Visuals
	pev->rendermode = kRenderTransTexture;		// Mode with transparency
	pev->renderamt = RCHG_GLASS_TRANSPARENCY;	// Transparency amount

	// Default animation
	pev->sequence = 0;
	pev->frame = 0;
}

//================================
// Charger itself
//================================

// Link entity to class
LINK_ENTITY_TO_CLASS(item_recharge, CItemRecharge);

// Save/restore
TYPEDESCRIPTION CItemRecharge::m_SaveData[] =
{
	DEFINE_FIELD(CItemRecharge, m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD(CItemRecharge, m_iReactivate, FIELD_INTEGER),
	DEFINE_FIELD(CItemRecharge, m_iJuice, FIELD_INTEGER),
	DEFINE_FIELD(CItemRecharge, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD(CItemRecharge, m_flSoundTime, FIELD_TIME),

	DEFINE_FIELD(CItemRecharge, IsUsed, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemRecharge, IsFrontAngle, FIELD_BOOLEAN),
	DEFINE_FIELD(CItemRecharge, CurrentState, FIELD_INTEGER),
	DEFINE_FIELD(CItemRecharge, CoilsAngle, FIELD_FLOAT),
	DEFINE_FIELD(CItemRecharge, pGlass, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemRecharge, CBaseButton);

// Methods //

void CItemRecharge::Precache(void)
{
	// Precache model
	PRECACHE_MODEL("models/hev.mdl");

	// Precache sounds
	PRECACHE_SOUND("items/suitcharge1.wav");
	PRECACHE_SOUND("items/suitchargeno1.wav");
	PRECACHE_SOUND("items/suitchargeok1.wav");

	// Precache sprite
	PRECACHE_MODEL("sprites/plasma.spr");
}

void CItemRecharge::Spawn(void)
{
	// Precache models & sounds
	Precache();

	// Set model
	SET_MODEL(ENT(pev), "models/hev.mdl");
	
	// Set up BBox and origin
	pev->solid = SOLID_BBOX;//SOLID_TRIGGER;
	SetSequenceBox();
	UTIL_SetOrigin(pev, pev->origin);

	// Reset bone controllers
	InitBoneControllers();
	RotateCamArm(NULL);
	CoilsAngle = 0;
	RotateCoils();

	// Set move type
	//pev->movetype = MOVETYPE_PUSH;	// This was default for "func_recharge", but it breaks "nextthink" and bone controllers
	//pev->movetype = MOVETYPE_NONE;	// No bone controller interpolation in GoldSrc (but OK in Xash)
	pev->movetype = MOVETYPE_FLY;		// This type enables bone controller interpolation in GoldSrc

	// Get initial charge
	m_iJuice = gSkillData.suitchargerCapacity;

	// Set "on" skin
	pev->skin = 0;

	// Reset state
	ChangeState(RCHG_STATE_IDLE, RCHG_SEQ_IDLE);

	// Spawn glass entity
	pGlass = GetClassPtr((CChargerGlass *)NULL);
	pGlass->Spawn();
	UTIL_SetOrigin(pGlass->pev, pev->origin);
	pGlass->pev->angles = pev->angles;
	pGlass->pev->owner = ENT(pev);

	// Set think delay
	pev->nextthink = gpGlobals->time + RCHG_DELAY_THINK;
}

void CItemRecharge::KeyValue(KeyValueData *pkvd)
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

void CItemRecharge::Think(void)
{
	// Debug: show bounding box
	//PS2HL_DEBUG(DBG_RenderBBox(pev->origin, pev->mins, pev->maxs, ceil(RCHG_DELAY_THINK / 0.1), 0xFF, 0x00, 0x00));

	// Get current time
	float CurrentTime = gpGlobals->time;

	// Set delay for next think
	pev->nextthink = CurrentTime + RCHG_DELAY_THINK;

	// Call animation handler
	StudioFrameAdvance(0);

	// DEBUG
	//static float LastTime = 0;
	//pev->nextthink = gpGlobals->time + RCHG_DELAY_THINK;
	//UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("Next: %.1f \nReal: %f\n", pev->nextthink - gpGlobals->time, gpGlobals->time - LastTime)); // Message
	//LastTime = gpGlobals->time;

	// Rotate coils
	if (IsUsed)
	{
		CoilsAngle += RCHG_CTRL_COILS_SPEED;
		RotateCoils();
	}

	// Find player
	CBaseEntity * pPlayer = FindPlayer(RCHG_ACTIVE_RADIUS);

	// Track player
	if (CurrentState != RCHG_STATE_EMPTY && CurrentState != RCHG_STATE_DEACTIVATE)
		RotateCamArm(pPlayer);
	else
		RotateCamArm(NULL);
	
	// State handler
	switch (CurrentState)
	{
	case RCHG_STATE_IDLE:
		// Player found?
		if (pPlayer != NULL)
		{
			// Player is near

			#ifdef RCHG_NO_BACK_USE
			// Prevent back side usage
			if (!IsFrontAngle)
				return;
			#endif

			#ifdef RCHG_SUIT_CHECK
			// Prevent deployment if player doesn't have HEV
			if (~pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			{
				RotateCamArm(NULL);
				return;
			}
			#endif

			#ifdef RCHG_ACTIVATE_SOUND
			// Emit sound
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM);
			#endif

			// Change state
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: activate\n\n\n"));
			ChangeState(RCHG_STATE_ACTIVATE, RCHG_SEQ_DEPLOY);
		}
		break;
	case RCHG_STATE_ACTIVATE:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: ready\n\n\n"));
			ChangeState(RCHG_STATE_READY, RCHG_SEQ_RDY);
		}
		break;
	case RCHG_STATE_READY:
		// Show beam
		MakeBeam();

		// Player is near?
		if (pPlayer == NULL)
		{
			// Nobody is near, hibernate
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: deactivate\n\n\n"));
			ChangeState(RCHG_STATE_DEACTIVATE, RCHG_SEQ_HIBERNATE);
		}
		else
		{
			#ifdef RCHG_NO_BACK_USE
			// Prevent back side usage
			if (!IsFrontAngle)
			{
				ChangeState(RCHG_STATE_DEACTIVATE, RCHG_SEQ_HIBERNATE);
				return;
			}
			#endif
			
			// Check if used
			if (IsUsed)
			{
				PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: used\n\n\n"));
				ChangeState(RCHG_STATE_START, RCHG_SEQ_EXPAND);
			}
		}
		break;
	case RCHG_STATE_START:
		// Show beam
		MakeBeam();

		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: giving charge\n\n\n"));
			ChangeState(RCHG_STATE_USE, RCHG_SEQ_USE);
		}
		break;
	case RCHG_STATE_USE:
		// Show beam
		MakeBeam();

		if (IsUsed == false)
		{
			// No longer used - change state
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: stopping\n\n\n"));
			ChangeState(RCHG_STATE_STOP, RCHG_SEQ_RETRACT);
		}

		// Reset use flag
		IsUsed = false;
		break;
	case RCHG_STATE_STOP:
		// Show beam
		MakeBeam();

		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: ready\n\n\n"));
			Off();
			ChangeState(RCHG_STATE_READY, RCHG_SEQ_RDY);
		}
		break;
	case RCHG_STATE_DEACTIVATE:
		// Just wait until animation is played
		if (m_fSequenceFinished)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: to idle\n\n\n"));
			ChangeState(RCHG_STATE_IDLE, RCHG_SEQ_IDLE);
		}
		break;
	case RCHG_STATE_EMPTY:
		// Swap animation to hibernate
		if (pev->sequence == RCHG_SEQ_RETRACT)
			if (m_fSequenceFinished)
				ChangeSequence(RCHG_SEQ_HIBERNATE);

		// Swap animation to idle
		if (pev->sequence == RCHG_SEQ_HIBERNATE)
			if (m_fSequenceFinished)
				ChangeSequence(RCHG_SEQ_IDLE);

		// Wait for next recharge (multiplayer)
		if (m_flNextCharge != -1 && CurrentTime > m_flNextCharge)
		{
			Recharge();
			ChangeState(RCHG_STATE_IDLE, RCHG_SEQ_IDLE);
		}
		break;
	default:
		// Do nothing
		break;
	}
}

void CItemRecharge::ChangeSequence(int NewSequence)
{
	// Set sequence
	pev->sequence = NewSequence;

	// Prepare sequence
	pev->frame = 0;
	ResetSequenceInfo();
}

void CItemRecharge::ChangeState(RechargeState NewState, int NewSequence)
{
	CurrentState = NewState;
	//CurrentStateTime = gpGlobals->time;	// I found no use for it so far
	ChangeSequence(NewSequence);
}

void CItemRecharge::RotateCamArm(CBaseEntity * pPlayer)
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

		#ifdef RCHG_NO_BACK_USE
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

	SetBoneController(RCHG_CTRL_ARM, Angle);
	SetBoneController(RCHG_CTRL_CAM, Angle);
}

void CItemRecharge::RotateCoils()
{
	if (CoilsAngle > RCHG_CTRL_COILS_MAX)
		CoilsAngle -= 360.0f;
	else if (CoilsAngle < RCHG_CTRL_COILS_MIN)
		CoilsAngle += 360.0f;//float CoilPos = (float)m_iJuice / gSkillData.suitchargerCapacity * RCHG_COIL_SPEED;
	
	SetBoneController(RCHG_CTRL_LCOIL, CoilsAngle);
	SetBoneController(RCHG_CTRL_RCOIL, RCHG_CTRL_COILS_MAX - CoilsAngle);
	//PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: coil angle: %.1f\n\n\n", CoilPos));
}

// Show beam for one Think() cycle
void CItemRecharge::MakeBeam(void)
{
	// Spawn beam
	CBeam *pBeam;
	//pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 64);
	pBeam = CBeam::BeamCreate("sprites/plasma.spr", 64);

	// Link attachments
	pBeam->EntsInit(entindex(), entindex());
	pBeam->SetStartAttachment(RCHG_ATCH_BEAM_START);
	pBeam->SetEndAttachment(RCHG_ATCH_BEAM_END);

	// Visual properties
	pBeam->SetColor(64, 255, 64);
	pBeam->SetBrightness(128);
	pBeam->SetWidth(3);
	pBeam->SetNoise(5);
	pBeam->SetScrollRate(30);
	pBeam->SetThink(&CBeam::SUB_Remove);

	// Remove on the next think cycle
	pBeam->pev->nextthink = gpGlobals->time + RCHG_DELAY_THINK;
}

CBaseEntity * CItemRecharge::FindPlayer(float Radius)
{
	CBaseEntity * pPlayer = NULL;	// Result
	CBaseEntity * pEntity = NULL;	// Used for search

	// Find near entities
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, RCHG_ACTIVE_RADIUS)) != NULL)
	{
		if (pEntity->IsPlayer())
		{
			// Found player - prepare result and terminate search
			pPlayer = pEntity;
			break;
		}
	}

	// Return result
	return pPlayer;
}

// Based on "func_recharge"
void CItemRecharge::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if (!FClassnameIs(pActivator->pev, "player"))
		return;

	#ifdef RCHG_NO_BACK_USE
	// PS2HL - prevent back side usage
	if (!IsFrontAngle)
		return;
	#endif
	
	#ifdef RCHG_NO_BACK_USE
	// Check distance
	Vector Dist = pev->origin - pActivator->pev->origin;
	if (Dist.Length2D() > RCHG_USE_RADIUS)
	{
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: player is too far ...\n\n\n"));
		return;
	}
	#endif

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		// PS2HL
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_recharge: empty\n\n\n"));
		pev->skin = 1;
		Off();
		if (CurrentState != RCHG_STATE_EMPTY)
		{
			ChangeState(RCHG_STATE_EMPTY, RCHG_SEQ_RETRACT);

			// Set up recharge
			if ((!m_iJuice) && ((m_iReactivate = g_pGameRules->FlHEVChargerRechargeTime()) > 0))
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
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/suitchargeno1.wav", 0.85, ATTN_NORM);
		}
		return;
	}

	//=============
	// PS2HL
	//=============
	// Speed up deploy if used early
	if (CurrentState != RCHG_STATE_START && CurrentState != RCHG_STATE_USE)
		ChangeState(RCHG_STATE_START, RCHG_SEQ_EXPAND);
	
	// Set use flag
	IsUsed = true;
	//=============


	//pev->nextthink = pev->ltime + 0.25;
	//SetThink(&CItemRecharge::Off);

	// Time to recharge yet?
	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Make sure that we have a caller
	if (!pActivator)
		return;

	m_hActivator = pActivator;

	//only recharge the player

	if (!m_hActivator->IsPlayer())
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM);
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "items/suitcharge1.wav", 0.85, ATTN_NORM);
	}


	// charge the player
	if (m_hActivator->pev->armorvalue < 100)
	{
		m_iJuice--;
		m_hActivator->pev->armorvalue += 1;

		if (m_hActivator->pev->armorvalue > 100)
			m_hActivator->pev->armorvalue = 100;
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

// Based on "func_recharge"
void CItemRecharge::Recharge(void)
{
	m_iJuice = gSkillData.suitchargerCapacity;
	pev->skin = 0;
	SetThink(&CBaseEntity::SUB_DoNothing);
}

// Based on "func_recharge"
void CItemRecharge::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
		STOP_SOUND(ENT(pev), CHAN_STATIC, "items/suitcharge1.wav");

	IsUsed = false;
	m_iOn = 0;

	if (m_iJuice <= 0)
		PS2HL_DEBUG(ALERT(at_console, "Next recharge in: %.0f s", m_flNextCharge - gpGlobals->time));
}

// Extract BBox from sequence (ripped from CBaseAnimating with one little change)
void CItemRecharge::SetSequenceBox(void)
{
	Vector mins, maxs;

	// Get sequence bbox
	if (ExtractBbox(pev->sequence, mins, maxs))
	{
		#ifdef RCHG_HACKED_BBOX
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
