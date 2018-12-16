/*

28.08.18

"item_recharge" entity for ps2hl

Created by supadupaplex
Based on "func_recharge"

*/


// Include guard: start
#ifndef PS2_RECHARGE_H
#define PS2_RECHARGE_H

// Header files
#include "extdll.h"		// Required for KeyValueData
#include "util.h"		// Required Consts & Macros
#include "cbase.h"		// Required for CPointEntity
#include "skill.h"		// Required for getting "skill" values
#include "gamerules.h"	// Required for getting "game rules" values
#include "ps2hl_dbg.h"	// BBox render func
#include "effects.h"	// CBeam

// Defines
#define RCHG_DELAY_THINK		0.1f		// Think delay
#define RCHG_ACTIVE_RADIUS		70.0f		// Distance at which charger is deployed
#define RCHG_USE_RADIUS			50.0f		// Defines distance at which player can use charger
#define RCHG_GLASS_TRANSPARENCY	128			// Glass transparency (0 - max, 255 - min)
#define RCHG_ACTIVATE_SOUND					// Play sound on activate (like in PS2 version)
//#define RCHG_SUIT_CHECK						// If defined then charger would not deploy for HEVless players (not present in PS2 version)
#define RCHG_NO_BACK_USE					// Prevent usage from the back side (like in PS2 version)
#define	RCHG_HACKED_BBOX					// BBox hack for GoldSource

//================================
// Glass for charger
//================================

class CChargerGlass : public CBaseAnimating
{
public:
	void Precache(void);
	void Spawn(void);
	virtual int	ObjectCaps(void) { return CBaseAnimating::ObjectCaps() &~FCAP_ACROSS_TRANSITION; }
};

//================================
// Charger itself
//================================

// Sequences
#define RCHG_SEQ_IDLE		0	// "rest"
#define RCHG_SEQ_DEPLOY		1	// "deploy"
#define RCHG_SEQ_HIBERNATE	2	// "retract_arm"
#define RCHG_SEQ_EXPAND		3	// "give_charge"
#define RCHG_SEQ_RETRACT	4	// "retract_charge"
#define RCHG_SEQ_RDY		5	// "prep_charge"
#define RCHG_SEQ_USE		6	// "charge_idle"

// Controllers
#define RCHG_CTRL_CAM	0				// Camera
#define RCHG_CTRL_LCOIL	1				// Left coil
#define RCHG_CTRL_RCOIL 2				// Right coil
#define RCHG_CTRL_ARM	3				// Arm

#define RCHG_CTRL_COILS_MIN		0.0f	// Coil controller min angle
#define RCHG_CTRL_COILS_MAX		360.0f	// Coil controller max angle
#define RCHG_CTRL_COILS_SPEED	30.0f	// Coil rotation speed (depends on think delay)

// Attachments
#define RCHG_ATCH_BEAM_START	3
#define RCHG_ATCH_BEAM_END		4

// States
enum RechargeState
{
	RCHG_STATE_IDLE,		// No player near by, wait
	RCHG_STATE_ACTIVATE,	// Player is near - deploy
	RCHG_STATE_READY,		// Player is near and deployed - rotate arm to player position
	RCHG_STATE_START,		// Player hit use - expand arm
	RCHG_STATE_USE,			// Give charge
	RCHG_STATE_STOP,		// Player stopped using - retract arm, continue tracking
	RCHG_STATE_DEACTIVATE,	// PLayer went away - hibernate, reset arm position
	RCHG_STATE_EMPTY		// Charger is empty (dead end state in SP, wait for recharge in MP)
};

class CItemRecharge : public CBaseButton
{
public:
	// From "func_recharge"
	float	m_flNextCharge;
	int		m_iReactivate;				// DeathMatch Delay until reactvated
	int		m_iJuice;					// How much charge left
	int		m_iOn;						// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;
private:
	// From "func_recharge"
	void Off(void);
	void Recharge(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) &~FCAP_ACROSS_TRANSITION; }	// Needed for continous use

	// New
	BOOL IsUsed;								// Needed to track if charger is used
	BOOL IsFrontAngle;							// Needed to track if player is in front or in back side of the charger
	RechargeState CurrentState;					// Current state of the charger
	float CoilsAngle;							// Current coils angle
	CChargerGlass * pGlass;						// Ptr. for glass

	void Spawn(void);							// Spawn handler
	void Precache(void);						// Precache handler
	CBaseEntity * FindPlayer(float Radius);		// Same as UTIL_FindEntityInSphere, but returns NULL if entity is not a player
	void RotateCamArm(CBaseEntity * pPlayer);	// Rotate camera and arm to player (or to initial position if pointer is NULL)
	void RotateCoils();							// Upadate coil position
	void SetSequenceBox(void);					// Extracts BBox
	void Think(void);							// Think handler
	void ChangeSequence(int Sequence);			// Set new animation
	void ChangeState(RechargeState NewState, int NewSequence);	// Set new state
	void MakeBeam(void);						// Create beam for one think period

	// Save/restore
	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
};

// Include guard: end
#endif