// Header
#include "ps2hl_item_generic.h"

// Link entity
LINK_ENTITY_TO_CLASS(item_generic, CItemGeneric);


// Methods //

// Precache handler
void CItemGeneric::Precache(void)
{
	PRECACHE_MODEL((char *)STRING(pev->model));
}

// Spawn handler
void CItemGeneric::Spawn(void)
{
	// Precache model
	Precache();

	// Set the model
	SET_MODEL(ENT(pev), STRING(pev->model));

	// Check if sequence is loaded
	if (pev->sequence == -1)
	{
		// Failed to load sequence
		ALERT(at_console, "item_generic: cant load animation sequence ...");
		pev->sequence = 0;
	}

	// Prepare sequence
	pev->frame = 0;
	m_fSequenceLoops = 1;
	ResetSequenceInfo();

	// BBox
	Vector Zero;
	Zero.x = Zero.y = Zero.z = 0;
	UTIL_SetSize(pev, Zero, Zero);
	pev->solid = SOLID_NOT;

	// Set think delay
	pev->nextthink = gpGlobals->time + ITGN_DELAY_THINK;
}

// Parse keys
void CItemGeneric::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "model"))
	{
		// Set model
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "body"))
	{
		// Set body
		pev->body = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sequencename"))
	{
		// Set sequence
		pev->sequence = LookupSequence(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

// Think handler
void CItemGeneric::Think(void)
{
	// Set delay
	pev->nextthink = gpGlobals->time + ITGN_DELAY_THINK;

	// Call animation handler
	StudioFrameAdvance(0);
}
