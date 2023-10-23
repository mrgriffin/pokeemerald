#include "global.h"
#include "main.h"
#include "main_menu.h"
#include "overworld.h"
#include "string_util.h"
#include "test/overworld.h"

static u32 OverworldTest_EstimateCost(void *data)
{
    return 1;
}

static void OverworldTest_SetUp(void *data)
{
}

static void OverworldTest_Run(void *data)
{
    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, COMPOUND_STRING("PLAYER"));
    SetMainCallback2(CB2_NewGame);
}

static void OverworldTest_TearDown(void *data)
{
}

static bool32 OverworldTest_CheckProgress(void *data)
{
    return FALSE;
}

const struct TestRunner gOverworldTestRunner =
{
    .estimateCost = OverworldTest_EstimateCost,
    .setUp = OverworldTest_SetUp,
    .run = OverworldTest_Run,
    .tearDown = OverworldTest_TearDown,
    .checkProgress = OverworldTest_CheckProgress,
};

static EWRAM_DATA bool8 sKeysAccept = FALSE;
static EWRAM_DATA u32 sKeysIndex = 0;

// TODO: If a button was accepted on the previous frame, we should press
// nothing on the next frame.
// TODO: Press nothing if the palette fade is active?
u32 GetKeys(void)
{
    if (sKeysAccept)
    {
        sKeysIndex++;
        sKeysAccept = FALSE;
    }

    switch (sKeysIndex)
    {
    case 0:  return DPAD_RIGHT;
    case 1:  return DPAD_DOWN;
    case 2:  return START_BUTTON;
    case 3:  return DPAD_DOWN;
    case 4:  return DPAD_DOWN;
    case 5:  return A_BUTTON;
    case 6:  return DPAD_DOWN;
    case 7:  return A_BUTTON;
    case 8:  return DPAD_UP;
    case 9:  return A_BUTTON;
    case 10: return B_BUTTON;
    default: return 0;
    }
}

void AcceptKeys(void)
{
    MgbaPrintf_("Accept %d", sKeysIndex);
    sKeysAccept = TRUE;
}

OVERWORLD_TEST("Overworld test")
{
}
