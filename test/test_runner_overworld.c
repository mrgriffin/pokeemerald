#include "global.h"
#include "main.h"
#include "main_menu.h"
#include "overworld.h"
#include "test/overworld.h"

#include "load_save.h"
#include "malloc.h"
#include "new_game.h"
#include "reload_save.h"
#include "save.h"
#include "string_util.h"

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

OVERWORLD_TEST("Overworld test")
{
}
