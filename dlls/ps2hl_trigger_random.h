/*

23.08.18

"trigger_random" entity for ps2hl. It triggers random target in specified range.
Next target: "target" + random value from 0 to "randomrange" - 1.

Example:
"target" = "branch_"
"randomrange" = "2"
One of those would be randomly triggered: "branch_0" or "branch_1"

Created by supadupaplex

*/


// Include guard: start
#ifndef PS2_TRIGGER_RANDOM_H
#define PS2_TRIGGER_RANDOM_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "ps2hl_dbg.h"	// Debug

// Spawnflags
#define	TRR_UNKNOWN 1
#define	TRR_NOREPEAT 2

// Defines
#define TRR_TRGT_CAP 10	// Target count cap, don't make it bigger than 32

class CTriggerRandom : public CBaseDelay
{
public:
	// Properties
	int Range;			// Range (from 0 to CAP-1)
private:
	int FiredFlags;		// Flags to remember what targets were fired
	int FiredCount;		// Remember how many times targets were fired

	// Methods
	void KeyValue(KeyValueData *pkvd);	// Parse keys
	void Spawn(void);					// Spawn handler
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);	// Use handler

	// Save/restore
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
};

// Include guard: end
#endif