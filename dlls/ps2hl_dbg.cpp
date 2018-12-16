// Includes
#include "ps2hl_dbg.h"
#include "util.h"

// Functions

void DBG_RenderBBox(Vector origin, Vector mins, Vector maxs, int life, BYTE r, BYTE b, BYTE g)
{
	//********************Render boundarybox**************************

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BOX);
	// coord, coord, coord boxmins
	WRITE_COORD(origin[0] + mins[0]);
	WRITE_COORD(origin[1] + mins[1]);
	WRITE_COORD(origin[2] + mins[2]);

	// coord, coord, coord boxmaxs
	WRITE_COORD(origin[0] + maxs[0]);
	WRITE_COORD(origin[1] + maxs[1]);
	WRITE_COORD(origin[2] + maxs[2]);
	WRITE_SHORT(life); // short life in 0.1 s (1min)
	WRITE_BYTE(r); // r, g, b
	WRITE_BYTE(g); // r, g, b
	WRITE_BYTE(b); // r, g, b

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)
}
