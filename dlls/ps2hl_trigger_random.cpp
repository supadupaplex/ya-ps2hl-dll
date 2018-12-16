// Includes
#include "ps2hl_trigger_random.h"

// Link entity
LINK_ENTITY_TO_CLASS( trigger_random, CTriggerRandom) ;

// Save/restore
TYPEDESCRIPTION CTriggerRandom::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerRandom, Range, FIELD_INTEGER),
	DEFINE_FIELD(CTriggerRandom, FiredFlags, FIELD_INTEGER),
	DEFINE_FIELD(CTriggerRandom, FiredCount, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CTriggerRandom, CBaseDelay);


// Methods //

// Parse keys
void CTriggerRandom::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "randomrange"))
	{
		// Get range
		Range = atoi(pkvd->szValue);
		if ( Range < 0 || Range > TRR_TRGT_CAP)
			Range = TRR_TRGT_CAP;
		PS2HL_DEBUG(ALERT(at_console, "\ntrigger_random: got range - %d\n\n", Range));
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

// Spawn handler
void CTriggerRandom::Spawn(void)
{
	// Get flags
	if (FBitSet(pev->spawnflags, TRR_UNKNOWN))
		ALERT(at_console, "\ntrigger_random: unknown flag (1)\n\n");

	if (FBitSet(pev->spawnflags, TRR_NOREPEAT))
	{
		PS2HL_DEBUG(ALERT(at_console, "\ntrigger_random: no_repeat flag is set\n\n"));
		FiredFlags = 0;
		FiredCount = 0;
	}
}

// Use handler
void CTriggerRandom::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	char RandomVal;
	if (pev->spawnflags & TRR_NOREPEAT)
	{
		// Without repeat //
		if (FiredCount >= Range)
		{
			// Got all variants, reset
			PS2HL_DEBUG(ALERT(at_console, "\ntrigger_random: resetting ...\n\n"));
			FiredFlags = 0;
			FiredCount = 0;
		}

		// Generate number from 0 to [Range - 1 - FireCount] to exclude fired targets from range of gen-ed numbers
		RandomVal = !Range ? 0 : RANDOM_LONG(0, Range - 1 - FiredCount);
		PS2HL_DEBUG(ALERT(at_console, "\ntrigger_random: no repeat, initial result - %d ...\n\n", RandomVal));

		// Convert generated number to actual target number
		bool Found = false;
		char Counter = 0;
		for (int BitPos = 0; BitPos < TRR_TRGT_CAP; BitPos++)
		{
			// Every bit inside FiredFlags corresponds to target number
			if (!(FiredFlags & (0x1 << BitPos)))
			{
				if (Counter == RandomVal)
				{
					Found = true;
					FiredFlags |= (0x1 << BitPos);
					FiredCount++;
					RandomVal = BitPos;
					break;
				}
				else
				{
					Counter++;
				}
			}
		}
		if (!Found)
		{
			// Not found
			ALERT(at_console, "\ntrigger_random: odd error - all targets were fired without detection ...\n\n");
			FiredCount = CHAR_MAX;
			return;
		}
	}
	else
	{
		// With repeat //
		RandomVal = !Range ? 0 : RANDOM_LONG(0, Range - 1);
	}

	// Generate full target name
	char szTargetName[128];
	sprintf_s(szTargetName, sizeof(szTargetName), "%s%d", STRING(pev->target), RandomVal);

	// Fire target
	FireTargets(szTargetName, this, this, USE_ON, 1);
	PS2HL_DEBUG(ALERT(at_console, "\ntrigger_random: fired target #%d out of 0-%d\nname: %s\n\n", RandomVal, Range == 0? 0 : Range - 1, szTargetName));
}
