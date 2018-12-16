/*

27.08.18

*/

// Include guard: start
#ifndef PS2_DBG_H
#define PS2_DBG_H

// Header files
//#include "util.h"
//#include "cbase.h"
#include "extdll.h"

// Defines
//#define PS2HL_ALLOW_DEBUG		// Comment this line to disable debug of new ps2hl entities

// References
extern cvar_t ps2hl_debug;
extern cvar_t ps2hl_precache;

// Macros //
#ifdef PS2HL_ALLOW_DEBUG

	#define PS2HL_DEBUG(code) {						\
		if (ps2hl_debug.value != 0)					\
		{											\
			code;									\
		}											\
	}

#else

	#define PS2HL_DEBUG(code) {	; }

#endif // PS2HL_ALLOW_DEBUG



// Functions //

// Source: https://www.moddb.com/games/half-life/tutorials/where-is-poppy-your-first-custom-entity-part-1
extern void DBG_RenderBBox(Vector origin, Vector mins, Vector maxs, int life, BYTE r = 0, BYTE b = 0, BYTE g = 0);

// Include guard: end
#endif