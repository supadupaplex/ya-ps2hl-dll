/*

10.11.18

"trigger_player_islave" entity for ps2hl

Created by supadupaplex

*/

// Include guard: start
#ifndef PS2_TRIGGER_ISLAVE_H
#define PS2_TRIGGER_ISLAVE_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "player.h"		// Required for EnableControl()
#include "ps2hl_dbg.h"	// Debug

// References
extern cvar_t ps2hl_islave_cheat;

class CTriggerPlayerISlave : public CBaseDelay
{
public:
	void Spawn(void);	// Spawn handler
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);	// Use handler
};

// Include guard: end
#endif