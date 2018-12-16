/*

20.09.18

Annoying little man HUD icon for ps2hl

Created by supadupaplex

*/

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#define HUD_IDLE_WALK	0
#define HUD_RUN			1
#define HUD_CROUCH		2

DECLARE_MESSAGE(m_HudMode, HudMode);


int CHudMode::Init(void)
{
	m_iMode = 0;

	HOOK_MESSAGE(HudMode);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudMode::Reset(void)
{
	m_iMode = 0;
}

int CHudMode::VidInit(void)
{
	int HUD_crouch_on = gHUD.GetSpriteIndex("crouch_on");
	int HUD_walk = gHUD.GetSpriteIndex("walk");
	int HUD_run = gHUD.GetSpriteIndex("run");

	m_hCrouch = gHUD.GetSprite(HUD_crouch_on);
	m_hWalk = gHUD.GetSprite(HUD_walk);
	m_hRun = gHUD.GetSprite(HUD_run);

	m_prcCrouch = &gHUD.GetSpriteRect(HUD_crouch_on);
	m_prcWalk = &gHUD.GetSpriteRect(HUD_walk);
	m_prcRun = &gHUD.GetSpriteRect(HUD_run);
	
	return 1;
};

int CHudMode::MsgFunc_HudMode(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	
	m_iMode = READ_BYTE();
	
	return 1;
}

int CHudMode::Draw(float flTime)
{
	// Do not draw if HUD is disabled
	if (gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)
		return 1;

	// Do not draw if suit isn't equipped
	if (!(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)))
		return 1;

	int r, g, b, x, y, a;
	wrect_t rc;

	a = 225;
	UnpackRGB(r, g, b, RGB_YELLOWISH);
	ScaleColors(r, g, b, a);

	y = (m_prcWalk->bottom - m_prcWalk->top) * 2;
	int Width = (m_prcWalk->right - m_prcWalk->left);
	x = ScreenWidth - Width - Width / 2;

	switch (m_iMode)
	{
	case HUD_IDLE_WALK:
		SPR_Set(m_hWalk, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prcWalk);
		break;
	case HUD_RUN:
		SPR_Set(m_hRun, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prcRun);
		break;
	case HUD_CROUCH:
		SPR_Set(m_hCrouch, r, g, b);
		SPR_DrawAdditive(0, x, y, m_prcCrouch);
		break;
	default:
		break;
	}

	return 1;
}