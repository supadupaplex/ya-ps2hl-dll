// Header
#include "ps2hl_item_eyescanner.h"

// Link entity
LINK_ENTITY_TO_CLASS(item_eyescanner, CItemScanner);

// Save/restore
TYPEDESCRIPTION	CItemScanner::m_SaveData[] =
{
	DEFINE_FIELD(CItemScanner, ResetDelay, FIELD_FLOAT),
	DEFINE_FIELD(CItemScanner, LockedTarget, FIELD_STRING),
	DEFINE_FIELD(CItemScanner, UnlockedTarget, FIELD_STRING),

	DEFINE_FIELD(CItemScanner, LastUseTime, FIELD_TIME),
	DEFINE_FIELD(CItemScanner, StartupTime, FIELD_TIME),
	DEFINE_FIELD(CItemScanner, CurrentState, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CItemScanner, CBaseButton);

// Methods //

// Precache handler
void CItemScanner::Precache(void)
{
	PRECACHE_MODEL("models/eye_scanner.mdl");
}

// Spawn handler
void CItemScanner::Spawn(void)
{
	// Precache
	Precache();

	// Set the model
	SET_MODEL(ENT(pev), "models/eye_scanner.mdl");

	// Set up BBox
	pev->solid = SOLID_NOT;//SOLID_BBOX;
	SetSequenceBox();

	// Debug
	//UnlockedTarget = ALLOC_STRING("control_retinal1mm");
	//pev->spawnflags = 0;
	//ResetDelay = 0.2;

	// Check flags
	if (pev->spawnflags & SCN_FLAG_LOCKED)
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: got flag 1 >> locked for player ...\n\n\n"));

	// Reset state
	ChangeState(SCN_STATE_IDLE, SCN_SEQ_CLOSED);
	LastUseTime = gpGlobals->time - ResetDelay;
	
	// Set think
	pev->nextthink = -1;
	//pev->nextthink = gpGlobals->time + SCN_DELAY_THINK;
}

// Parse keys
void CItemScanner::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "reset_delay"))
	{
		// Reset delay
		ResetDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked_target"))
	{
		// Target to fire when deny
		LockedTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "unlocked_target"))
	{
		// Target to fire when accept
		UnlockedTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

// Think handler
void CItemScanner::Think(void)
{
	// Debug: show bounding box
	//PS2HL_DEBUG( DBG_RenderBBox(pev->origin, pev->mins, pev->maxs, ceil(SCN_DELAY_THINK / 0.1), 0xFF, 0x00, 0x00) );
	
	// Get current time
	float CurrentTime = gpGlobals->time;

	// Set think delay
	pev->nextthink = CurrentTime + SCN_DELAY_THINK;

	// Call animation handler
	StudioFrameAdvance(0);

	// State handler
	switch (CurrentState)
	{
	//case SCHED_SC_IDLE:
	//	// Do nothing
	//	break;
	case SCN_STATE_OPEN:
		// Wait for opening animation
		if (m_fSequenceFinished)
		{
			// Go to active state once opening animation is ended
			ChangeState(SCN_STATE_ACTIVE, SCN_SEQ_OPEN);
		}

		if (CurrentTime > (StartupTime + SCN_DELAY_ACTIVATE))
		{
			// Cycle skins
			pev->skin++;

			if (pev->skin > 3)
				pev->skin = 1;
		}
		break;
	case SCN_STATE_ACTIVE:
		if (CurrentTime > (StartupTime + SCN_DELAY_ACTIVATE))
		{
			// Continue to cycle skins
			pev->skin++;

			if (pev->skin > 3)
				pev->skin = 1;
		}

		if (CurrentTime > (StartupTime + SCN_DELAY_DEACTIVATE))
		{
			// Start closing once enough time is passed
			pev->skin = 0;
			ChangeState(SCN_STATE_CLOSE, SCN_SEQ_DEACTIVATE);
		}
		break;
	case SCN_STATE_CLOSE:
		if (m_fSequenceFinished)
		{
			// Go to idle state once closing animation is ended
			ChangeState(SCN_STATE_IDLE, SCN_SEQ_CLOSED);
			
			// Hibernate
			pev->nextthink = -1;
		}
		break;
	default:
		// Do nothing
		break;
	}
}

// Change sequence
void CItemScanner::ChangeSequence(int NewSequence)
{
	// Prepare sequence
	pev->sequence = NewSequence;
	pev->frame = 0;
	ResetSequenceInfo();
}

// Change state for Think()
void CItemScanner::ChangeState(ScanState NewState, int NewSequence)
{
	CurrentState = NewState;
	ChangeSequence(NewSequence);
}

// Use handler
void CItemScanner::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Get current time
	float CurrentTime = gpGlobals->time;

	// Make sure that we have an activator
	if (!pActivator)
		pActivator = this;

	// Check activator
	if (pActivator->IsPlayer())
	{
		// Player //

		// Hack to prevent false "player" call on c3a2
		Vector Dist = pev->origin - pActivator->pev->origin;
		if (Dist.Length2D() > SCN_USE_RADIUS)
		{
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: player is too far ...\n\n\n"));
			return;
		}

		// Check if busy
		if (CurrentState != SCN_STATE_IDLE)
		{
			// Busy - reject until the state is idle
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: busy ...\n\n\n"));
			return;
		}

		// Check reset delay
		if ((CurrentTime - LastUseTime) < ResetDelay)
		{
			// Hibernated - reject until after reset delay
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: hibernated ...\n\n\n"));
			return;
		}

		// Select target
		PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: started by player ...\n\n\n"));
		if (pev->spawnflags & SCN_FLAG_LOCKED)
			pev->target = LockedTarget;
		else
			pev->target = UnlockedTarget;

		// Fire target
		SUB_UseTargets(this, USE_ON, 1);
		PS2HL_DEBUG(if (pev->target) ALERT(at_console, "\n\nitem_eyescanner: fired target \"%s\"\n\n\n", STRING(pev->target)));

		// Start scanner
		ChangeState(SCN_STATE_OPEN, SCN_SEQ_ACTIVATE);
		pev->nextthink = StartupTime = CurrentTime + SCN_DELAY_START;
	}
	else
	{
		// Script //

		// Check if triggered once or in burst
		if ((CurrentTime - LastUseTime) > ResetDelay)
		{
			// Once - accept
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: started by script ...\n\n\n"));
			pev->target = UnlockedTarget;

			// Fire target
			SUB_UseTargets(this, USE_ON, 1);
			PS2HL_DEBUG(if (pev->target) ALERT(at_console, "\n\nitem_eyescanner: fired target \"%s\"\n\n\n", STRING(pev->target)));

			// Start scanner
			ChangeState(SCN_STATE_OPEN, SCN_SEQ_ACTIVATE);
			pev->nextthink = StartupTime = CurrentTime + SCN_DELAY_START;
		}
		else
		{
			// Burst - can't figure out what it does
			PS2HL_DEBUG(ALERT(at_console, "\n\nitem_eyescanner: burst detected ...\n\n\n"));
		}
	}

	// Update last use time
	LastUseTime = CurrentTime;
}

// Extract BBox from sequence (ripped from CBaseAnimating with one little change)
void CItemScanner::SetSequenceBox(void)
{
	Vector mins, maxs;

	// Get sequence bbox
	if (ExtractBbox(pev->sequence, mins, maxs))
	{
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