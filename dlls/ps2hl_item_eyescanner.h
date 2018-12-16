/*

24.08.18

"item_eyescanner" entity for ps2hl

Created by supadupaplex

*/


// Include guard: start
#ifndef PS2_EYESCANNER_H
#define PS2_EYESCANNER_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "ps2hl_dbg.h"	// Debug

// Flags
#define SCN_FLAG_LOCKED			1	// Locked for player flag

// Sequences
#define SCN_SEQ_CLOSED			0	// "idle_CLOSED"
#define SCN_SEQ_OPEN			1	// "idle_OPEN"
#define SCN_SEQ_ACTIVATE		2	// "ACTIVATE"
#define SCN_SEQ_DEACTIVATE		3	//" DEACTIVATE"

// States for scanner
enum ScanState {
	SCN_STATE_IDLE,		// Rest
	SCN_STATE_OPEN,		// Open
	SCN_STATE_ACTIVE,	// Cycle skins
	SCN_STATE_CLOSE		// Close
};

// Timings
#define SCN_DELAY_THINK			0.15f	// Delay before Think() calls. Affects skin cycle speed.
#define SCN_DELAY_START			0.01f	// Delay before startup
#define SCN_DELAY_ACTIVATE		2.0f	// 2.4 // Defines time before cycling skins
#define SCN_DELAY_DEACTIVATE	4.6f	// 5.0 // Defines time when to end cycling skins
#define SCN_USE_RADIUS			25.0f	// Defines distance at which player can use scanner

class CItemScanner : public CBaseButton
{
public:
	float ResetDelay;					// How many seconds to be inactive
	string_t LockedTarget;				// What to activate on deny
	string_t UnlockedTarget;			// What to activate on accept
private:
	// For Use()
	float LastUseTime;					// Time when last used (used to track reset delay for player)
	
	// For Think()
	ScanState CurrentState;				// Current state of the scanner
	float StartupTime;					// Time when scanner received use request (used to to track states)

	// Methods
	void Spawn(void);							// Spawn handler
	void Precache(void);						// Precache handler
	void KeyValue(KeyValueData *pkvd);			// Parse keys
	void Think(void);							// Think handler
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);	// Use handler
	void SetSequenceBox(void);					// Get BBox (this one considers rotation unlike ExtractBbox())
	void ChangeSequence(int Sequence);			// Set new animation
	void ChangeState(ScanState NewState, int NewSequence);	// Set new state

	// Save/restore
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
};

// Include guard: end
#endif