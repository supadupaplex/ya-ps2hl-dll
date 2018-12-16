/*

22.08.18

"trigger_playerfreeze" entity for ps2hl

Created by supadupaplex
Based on: http://gamer-lab.com/eng/lesson_goldsrc/Trigger_PlayerFreeez

*/

// Include guard: start
#ifndef PS2_TRIGGER_PFREEZE_H
#define PS2_TRIGGER_PFREEZE_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "player.h"		// Required for EnableControl()
#include "ps2hl_dbg.h"	// Debug

// Defines
#define TPF_DELAY_THINK	0.1f
#define TPF_DELAY_BURST	0.5f

class CTriggerPlayerFreeze : public CBaseDelay
{
public:
	// Properties
	float LastTime;		// Used for burst detection

	// Methods
	void Spawn(void);	// Spawn handler
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);	// Use handler
	void ToggleFreeze(void);	// Change freeze state
	void Unfreeze(void);		// Unfreeze

	// Save/restore
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
};

// Include guard: end
#endif