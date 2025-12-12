#include "global.h"
#include "main.h"
#include "main_menu.h"
#include "overworld.h"
#include "string_util.h"
#include "test/overworld.h"

u32 TestRunner_ReadKeys(u32 prevKeyInput)
{
    u32 keyInput = 0;
    return keyInput;
}

void TestRunner_Overworld_SetInitialWarpDestination(void)
{
    SetWarpDestination(MAP_GROUP(MAP_OLDALE_TOWN_POKEMON_CENTER_1F), MAP_NUM(MAP_OLDALE_TOWN_POKEMON_CENTER_1F), WARP_ID_NONE, -1, -1);
}

static void OverworldTest_Run(void *data)
{
    void (*function)(void) = data;
    function();
    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, COMPOUND_STRING("PLAYER"));
    SetMainCallback2(CB2_NewGame);
}

const struct TestRunner gOverworldTestRunner =
{
    .run = OverworldTest_Run,
};

OVERWORLD_TEST("OVERWORLD")
{
}
