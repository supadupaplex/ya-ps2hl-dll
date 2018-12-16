// Header
#include "ps2hl_trigger_playerfreeze.h"

// Link entity
LINK_ENTITY_TO_CLASS(trigger_playerfreeze, CTriggerPlayerFreeze);

// Save/restore
TYPEDESCRIPTION CTriggerPlayerFreeze::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerPlayerFreeze, LastTime, FIELD_TIME)
};
IMPLEMENT_SAVERESTORE(CTriggerPlayerFreeze, CBaseDelay);

// Methods
void CTriggerPlayerFreeze::Spawn(void)
{
	// Init fields
	float LastTime = 0;
	SetThink(NULL);
}

void CTriggerPlayerFreeze::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Check if triggered once or in burst
	float CurrentTime = gpGlobals->time;
	if ((CurrentTime - LastTime) > TPF_DELAY_BURST)
	{
		// Once - change state
		SetThink(& CTriggerPlayerFreeze :: ToggleFreeze);
		pev->nextthink = gpGlobals->time + TPF_DELAY_THINK;
	}
	else
	{
		// Burst - guarantee unfreeze
		SetThink(& CTriggerPlayerFreeze :: Unfreeze);
		pev->nextthink = gpGlobals->time + TPF_DELAY_THINK;
	}
	
	// Update last time
	LastTime = CurrentTime;
}

void CTriggerPlayerFreeze::ToggleFreeze()
{
	// Find player
	CBaseEntity * pPlayer = UTIL_FindEntityByClassname(NULL, "player");

	// Wierd WON version glitch
	if (!pPlayer)
	{
		PS2HL_DEBUG(ALERT(at_console, "\n\ntrigger_playerfreeze: can't find player!\n\n\n"));
		pev->nextthink = gpGlobals->time + TPF_DELAY_THINK;
		return;
	}

	// Change state
	PS2HL_DEBUG(ALERT(at_console, "\n\ntrigger_playerfreeze: changing state to "));
	if (pPlayer->pev->flags & FL_FROZEN)
	{
		PS2HL_DEBUG(ALERT(at_console, "normal ...\n\n\n"));
		((CBasePlayer *)((CBaseEntity *)pPlayer))->EnableControl(TRUE);
	}
	else
	{
		PS2HL_DEBUG(ALERT(at_console, "frozen ...\n\n\n"));
		((CBasePlayer *)((CBaseEntity *)pPlayer))->EnableControl(FALSE);
	}

	SetThink(NULL);
}

void CTriggerPlayerFreeze::Unfreeze()
{
	// Find player
	CBaseEntity * pPlayer = UTIL_FindEntityByClassname(NULL, "player");

	// Wierd WON version glitch
	if (!pPlayer)
	{
		PS2HL_DEBUG(ALERT(at_console, "\n\ntrigger_playerfreeze: can't find player!\n\n\n"));
		pev->nextthink = gpGlobals->time + TPF_DELAY_THINK;
		return;
	}

	// Unfreeze
	PS2HL_DEBUG(ALERT(at_console, "\n\ntrigger_playerfreeze: burst detected - unfreezing player ...\n\n\n"));
	((CBasePlayer *)((CBaseEntity *)pPlayer))->EnableControl(TRUE);

	SetThink(NULL);
}