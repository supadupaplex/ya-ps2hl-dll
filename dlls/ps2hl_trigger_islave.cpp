// Header
#include "ps2hl_trigger_islave.h"

// Link entity
LINK_ENTITY_TO_CLASS(trigger_player_islave, CTriggerPlayerISlave);

// Methods
void CTriggerPlayerISlave::Spawn(void)
{
	// Init
	SetThink(NULL);
}

void CTriggerPlayerISlave::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Check cvar and fire target
	if (ps2hl_islave_cheat.value != 0)
	{
		PS2HL_DEBUG(ALERT(at_console, "\ntrigger_player_islave: ok\n\n"));
		SUB_UseTargets(this, USE_TOGGLE, 0);
	}
	else
	{
		PS2HL_DEBUG(ALERT(at_console, "\ntrigger_player_islave: reject\n\n"));
	}
}
