/*

27.08.18

"item_healthcharger" entity for ps2hl

Created by supadupaplex
Based on "func_healthcharger"

*/

// Include guard: start
#ifndef PS2_HEALTHCHARGER_H
#define PS2_HEALTHCHARGER_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "skill.h"		// Required for getting "skill" values
#include "gamerules.h"	// Required for getting "game rules" values
#include "effects.h"	// CBeam
#include "ps2hl_dbg.h"	// BBox render func


// Defines
#define HCHG_DELAY_THINK		0.1f		// Think delay
#define HCHG_ACTIVE_RADIUS		70.0f		// Distance at which charger is deployed
#define HCHG_USE_RADIUS			50.0f		// Defines distance at which player can use charger
#define HCHG_GLASS_TRANSPARENCY	128			// Glass bottle transparency (0 - max, 255 - min)
#define HCHG_ACTIVATE_SOUND					// Play sound on activate (like in PS2 version)
//#define HCHG_SUIT_CHECK						// If defined then charger would not deploy for HEVless players (not present in PS2 version)
#define HCHG_NO_BACK_USE					// Prevent usage from the back side (like in PS2 version)
#define	HCHG_HACKED_BBOX					// BBox hack for GoldSource

//================================
// Bottle for charger
//================================

// Sequences
#define HBOTTLE_SEQ_IDLE	0	// "still"
#define HBOTTLE_SEQ_USE		1	// "slosh"
#define HBOTTLE_SEQ_STOP	2	// "to_rest"

// Controllers
#define HBOTTLE_CTRL_LVL		0		// Green goo level controller
#define HBOTTLE_CTRL_LVL_MIN	-11.0f	// Angle that corresponds to the min goo level
#define HBOTTLE_CTRL_LVL_MAX	0.0f	// Angle that corresponds to the max goo level

// States
//enum HBottleState
//{
//	HBOTTLE_STATE_IDLE,		// Rest
//	HBOTTLE_STATE_USE,		// Slosh
//	HBOTTLE_STATE_STOP,		// Stop slosh and go to idle
//};

class CHealthBottle : public CBaseAnimating
{
public:
	// Properties
	

	// Methods
	void Precache(void);						// Precache handler
	void Spawn(void);							// Spawn handler
	void Think(void);							// Think handler
	//void ChangeState(HBottleState NewState, int NewSequence);	// Set new state
	void ChangeSequence(int Sequence);			// Set new animation
	void SetLevel(float Level);					// Set level of a green goo
	virtual int	ObjectCaps(void) { return CBaseAnimating::ObjectCaps() &~FCAP_ACROSS_TRANSITION; }

	// Save/restore
	//virtual int		Save(CSave &save);
	//virtual int		Restore(CRestore &restore);
	//static	TYPEDESCRIPTION m_SaveData[];
};

//================================
// Charger itself
//================================

// Sequences
#define HCHG_SEQ_IDLE		0	// "still"
#define HCHG_SEQ_DEPLOY		1	// "deploy"
#define HCHG_SEQ_HIBERNATE	2	// "retract_arm"
#define HCHG_SEQ_EXPAND		3	// "give_shot"
#define HCHG_SEQ_RETRACT	4	// "retract_shot"
#define HCHG_SEQ_RDY		5	// "prep_shot"
#define HCHG_SEQ_USE		6	// "shot_idle"

// Controllers
#define HCHG_CTRL_ARM	0	// Arm
#define HCHG_CTRL_CAM	1	// Camera

// States
enum RechargeState
{
	HCHG_STATE_IDLE,		// No player near by, wait
	HCHG_STATE_ACTIVATE,	// Player is near - deploy
	HCHG_STATE_READY,		// Player is near and deployed - rotate arm to player position
	HCHG_STATE_START,		// Player hit use - expand arm
	HCHG_STATE_USE,			// Give charge
	HCHG_STATE_STOP,		// Player stopped using - retract arm, continue tracking
	HCHG_STATE_DEACTIVATE,	// PLayer went away - hibernate, reset arm position
	HCHG_STATE_EMPTY		// Charger is empty (dead end state in SP, wait for recharge in MP)
};

class CItemHealthCharger : public CBaseButton
{
public:
	// From "func_healthcharger"
	float m_flNextCharge;
	int		m_iReactivate;				// DeathMatch Delay until reactvated
	int		m_iJuice;					// How many charge left
	int		m_iOn;						// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;
private:
	// From "func_healthcharger"
	void EXPORT Off(void);
	void EXPORT Recharge(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) &~FCAP_ACROSS_TRANSITION; }	// Needed for continous use

	// New
	BOOL IsUsed;								// Needed to track if charger is used
	BOOL IsFrontAngle;							// Needed to track if player is in front or in back side of the charger
	RechargeState CurrentState;					// Current state of the charger
	CHealthBottle * pBottle;					// Ptr. for bottle

	void Spawn(void);							// Spawn handler
	void Precache(void);						// Precache handler
	CBaseEntity * FindPlayer(float Radius);		// Same as UTIL_FindEntityInSphere, but returns NULL if entity is not a player
	void RotateCamArm(CBaseEntity * pPlayer);	// Rotate camera and arm to player (or to initial position if pointer is NULL)
	//void RotateCoils();							// Upadate coil position
	void SetSequenceBox(void);					// Extracts BBox
	void Think(void);							// Think handler
	void ChangeSequence(int Sequence);			// Set new animation
	void ChangeState(RechargeState NewState, int NewSequence);	// Set new state
	//void MakeBeam(void);						// Create beam for one think period
	
	// Save/restore
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
};

// Include guard: end
#endif