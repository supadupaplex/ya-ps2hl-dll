/*

20.10.18

PS2HL weapon lock sprite

Created by supadupaplex

*/

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>


//DECLARE_MESSAGE(m_HudLock, HudLockOff);

// cvar reference
extern cvar_t * cl_ps2hl_oldsights;

int CHudLock::Init(void)
{
	m_iOffX = m_iOffY = m_iActive = 0;

	//HOOK_MESSAGE(HudLockOff);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudLock::Reset(void)
{
	m_iOffX = m_iOffY = m_iActive = 0;
}

void CHudLock::SetSprite(SpriteHandle_t hspr, wrect_t rc)
{
	m_hLock = hspr;
	m_rcLock = rc;
}

void CHudLock::SetState(bool Active)
{
	m_iActive = Active;
}

//void CHudLock::SetOffsets(int OffsetX, int OffsetY)
//{
//	m_iOffX = OffsetX;
//	m_iOffY = OffsetY;
//}

//int CHudLock::MsgFunc_HudLockOff(const char *pszName, int iSize, void *pbuf)	// (doesn't work)
//{
//	BEGIN_READ(pbuf, iSize);
//
//	m_iOffX = READ_COORD();
//	m_iOffY = READ_COORD();
//
//	return 1;
//}

int CHudLock::VidInit(void)
{
	m_hLock = 0;
	m_rcLock.top = 0;
	m_rcLock.bottom = 0;
	m_rcLock.left = 0;
	m_rcLock.right = 0;

	return 1;
};

int CHudLock::Draw(float flTime)
{
	static wrect_t nullrc;

	// Do not draw if HUD is disabled
	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_ALL | HIDEHUD_WEAPONS))
	{
		SetCrosshair(0, nullrc, 0, 0, 0);	// Ghost crosshair bug fix
		return 1;
	}
	
	// Do not draw if suit isn't equipped
	if (!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
	{
		SetCrosshair(0, nullrc, 0, 0, 0);	// Ghost crosshair bug fix
		//return 1; // This is commented to match the PS2 version
	}

	// Do not draw if there are no weapons
	if (!(gHUD.m_iWeaponBits & ~(1 << (WEAPON_SUIT))))
	{
		SetCrosshair(0, nullrc, 0, 0, 0);	// Ghost crosshair bug fix
		return 1;
	}

	// Do not draw if old crosshairs are enabled
	if (cl_ps2hl_oldsights->value != 0)
		return 1;

	int r, g, b, x, y, a;

	// Set coordinates
	int Width = (m_rcLock.right - m_rcLock.left);
	int Height = (m_rcLock.bottom - m_rcLock.top);
	x = m_iOffX + ((ScreenWidth - Width) >> 1);
	y = m_iOffY + ((ScreenHeight - Height) >> 1);
	
	// Do not draw empty sprite
	if (Width == 0 || Height == 0)
		return 1;

	// Set color
	a = 225;
	if (m_iActive)
		UnpackRGB(r, g, b, RGB_REDISH);
	else
		UnpackRGB(r, g, b, RGB_YELLOWISH);
	ScaleColors(r, g, b, a);

	// Draw
	SPR_Set(m_hLock, r, g, b);
	SPR_DrawAdditive(0, x, y, &m_rcLock);

	return 1;
}
