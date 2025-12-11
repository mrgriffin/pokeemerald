#include "global.h"
#include "field_control_avatar.h"
#include "main.h"
#include "main_menu.h"
#include "overworld.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "test/overworld.h"
#include "constants/event_object_movement.h"

#if defined(__INTELLISENSE__)
#undef TestRunner_Overworld_PreInput
#undef TestRunner_Overworld_PostInput
#endif

enum PACKED OverworldInput
{
    OW_INP_FACE_UP,
    OW_INP_FACE_RIGHT,
    OW_INP_FACE_DOWN,
    OW_INP_FACE_LEFT,
    OW_INP_STEP_UP,
    OW_INP_STEP_RIGHT,
    OW_INP_STEP_DOWN,
    OW_INP_STEP_LEFT,
    OW_INP_INTERACT,
};

enum PACKED OverworldTestInputState
{
    OW_INP_ST_MAKE,
    OW_INP_ST_WAIT,
};

struct OverworldTestState
{
    const enum OverworldInput *inputs;
    u32 inputsCount;
    u32 inputsIndex;
    u32 checkProgressInputsIndex;
    enum OverworldTestInputState inputState;
};

static EWRAM_DATA struct OverworldTestState sOverworldTestState = {0};

#define STATE sOverworldTestState

u32 TestRunner_ReadKeys(u32 prevKeyInput)
{
    u32 keyInput = 0;
    enum OverworldInput currentInput = STATE.inputs[STATE.inputsIndex];

    if (gMain.callback1 == CB1_Overworld
     && gMain.callback2 == CB2_Overworld)
    {
        if (!ArePlayerFieldControlsLocked())
        {
            if (STATE.inputState == OW_INP_ST_MAKE)
            {
                switch (currentInput)
                {
                case OW_INP_FACE_UP:
                    keyInput = ~prevKeyInput & DPAD_UP;
                    break;
                case OW_INP_FACE_RIGHT:
                    keyInput = ~prevKeyInput & DPAD_RIGHT;
                    break;
                case OW_INP_FACE_DOWN:
                    keyInput = ~prevKeyInput & DPAD_DOWN;
                    break;
                case OW_INP_FACE_LEFT:
                    keyInput = ~prevKeyInput & DPAD_LEFT;
                    break;
                case OW_INP_STEP_UP:
                    keyInput = DPAD_UP;
                    break;
                case OW_INP_STEP_RIGHT:
                    keyInput = DPAD_RIGHT;
                    break;
                case OW_INP_STEP_DOWN:
                    keyInput = DPAD_DOWN;
                    break;
                case OW_INP_STEP_LEFT:
                    keyInput = DPAD_LEFT;
                    break;
                case OW_INP_INTERACT:
                    keyInput = ~prevKeyInput & A_BUTTON;
                    break;
                }
            }
        }
        else
        {
            if (STATE.inputState == OW_INP_ST_WAIT)
            {
                switch (currentInput)
                {
                case OW_INP_FACE_UP:
                case OW_INP_FACE_RIGHT:
                case OW_INP_FACE_DOWN:
                case OW_INP_FACE_LEFT:
                case OW_INP_STEP_UP:
                case OW_INP_STEP_RIGHT:
                case OW_INP_STEP_DOWN:
                case OW_INP_STEP_LEFT:
                    break;
                case OW_INP_INTERACT:
                    if (ActiveMultichoiceType() != MULTICHOICE_NONE)
                    {
                        u32 currentPos = Menu_GetCursorPos();
                        u32 targetPos = 1;
                        if (currentPos < targetPos)
                            keyInput = ~prevKeyInput & DPAD_DOWN;
                        else if (currentPos > targetPos)
                            keyInput = ~prevKeyInput & DPAD_UP;
                        else
                            keyInput = ~prevKeyInput & A_BUTTON;
                    }
                    else
                    {
                        keyInput = ~prevKeyInput & A_BUTTON;
                    }
                    break;
                }
            }
        }
    }

    return keyInput;
}

void TestRunner_Overworld_PostPlayerInput(const struct FieldInput *input, const struct PlayerAvatar *avatar, const struct ObjectEvent *objectEvent, bool32 playerFieldControlsWereLocked, bool32 playerFieldControlsAreLocked)
{
    enum OverworldInput currentInput = STATE.inputs[STATE.inputsIndex];

    if (STATE.inputState == OW_INP_ST_MAKE)
    {
        bool32 accepted = FALSE;
        switch (currentInput)
        {
        case OW_INP_FACE_UP:
            accepted = objectEvent->facingDirection == DIR_NORTH;
            break;
        case OW_INP_FACE_RIGHT:
            accepted = objectEvent->facingDirection == DIR_EAST;
            break;
        case OW_INP_FACE_DOWN:
            accepted = objectEvent->facingDirection == DIR_SOUTH;
            break;
        case OW_INP_FACE_LEFT:
            accepted = objectEvent->facingDirection == DIR_WEST;
            break;
        case OW_INP_STEP_UP:
        case OW_INP_STEP_RIGHT:
        case OW_INP_STEP_DOWN:
        case OW_INP_STEP_LEFT:
            accepted = objectEvent->playerCopyableMovement != COPY_MOVE_NONE && objectEvent->playerCopyableMovement != COPY_MOVE_FACE;
            break;
        case OW_INP_INTERACT:
            accepted = playerFieldControlsAreLocked;
            break;
        }

        if (accepted)
            STATE.inputState = OW_INP_ST_WAIT;
    }
    else if (STATE.inputState == OW_INP_ST_WAIT)
    {
        bool32 accepted = FALSE;
        switch (currentInput)
        {
        case OW_INP_FACE_UP:
        case OW_INP_FACE_RIGHT:
        case OW_INP_FACE_DOWN:
        case OW_INP_FACE_LEFT:
            accepted = TRUE;
            break;
        case OW_INP_STEP_UP:
        case OW_INP_STEP_RIGHT:
        case OW_INP_STEP_DOWN:
        case OW_INP_STEP_LEFT:
            accepted = input->tookStep;
            break;
        case OW_INP_INTERACT:
            accepted = !playerFieldControlsAreLocked;
            break;
        }

        if (accepted)
        {
            STATE.inputsIndex++;
            if (STATE.inputsIndex == STATE.inputsCount)
                SetMainCallback2(CB2_TestRunner);
            STATE.inputState = OW_INP_ST_MAKE;
        }
    }
}

void TestRunner_Overworld_SetInitialWarpDestination(void)
{
    SetWarpDestination(MAP_GROUP(MAP_OLDALE_TOWN_POKEMON_CENTER_1F), MAP_NUM(MAP_OLDALE_TOWN_POKEMON_CENTER_1F), WARP_ID_NONE, -1, -1);
}

static void OverworldTest_SetUp(void *)
{
    memset(&STATE, 0, sizeof(STATE));
}

static void OverworldTest_Run(void *data)
{
    void (*function)(void) = data;
    function();
    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, COMPOUND_STRING("PLAYER"));
    SetMainCallback2(CB2_NewGame);
}

static bool32 OverworldTest_CheckProgress(void *data)
{
    bool32 madeProgress
         = STATE.checkProgressInputsIndex < STATE.inputsIndex;
    STATE.checkProgressInputsIndex = STATE.inputsIndex;
    return madeProgress;
}

const struct TestRunner gOverworldTestRunner =
{
    .setUp = OverworldTest_SetUp,
    .run = OverworldTest_Run,
    .checkProgress = OverworldTest_CheckProgress,
};

/* An input checks for the callbacks, and then it checks for when to
 * fire the input, and then it checks for if the input was accepted...
 * but checking for acceptance could have a nested list of inputs (forms
 * a tree). */

struct Keys
{
    u16 new;
    u16 held;
};

static u32 DirectionToDpad(u32 direction)
{
    switch (direction)
    {
    case DIR_SOUTH: return DPAD_DOWN;
    case DIR_NORTH: return DPAD_UP;
    case DIR_WEST:  return DPAD_LEFT;
    case DIR_EAST:  return DPAD_RIGHT;
    default: Test_ExitWithResult(TEST_RESULT_INVALID, SourceLine(0), ":LDirectionToDpad: invalid direction %d", direction);
    }
}

struct KeyInput
{
    bool32 (*isReady)(uintptr_t data);
    struct Keys (*getKeys)(uintptr_t data);
    bool32 (*isAccepted)(uintptr_t data);
    uintptr_t data;
};

static bool32 Overworld_PlayerInputReady(uintptr_t)
{
    return gMain.callback1 == CB1_Overworld
        && gMain.callback2 == CB2_Overworld
        && !ArePlayerFieldControlsLocked();
}

struct Keys Overworld_GetKeys_PlayerFacing(uintptr_t direction)
{
    return (struct Keys) { .newKeys = DirectionToDpad(direction) };
}

static bool32 Overworld_Accept_PlayerFacing(uintptr_t direction)
{
    const struct ObjectEvent objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    return objectEvent->facingDirection == direction;
}

static const struct KeyInput sKeyInput_Overworld_FaceUp =
{
    .isReady = Overworld_PlayerInputReady,
    .getKeys = NULL,
    .isAccepted = Overworld_Accept_PlayerFacing,
    .data = DIR_NORTH,
};

// For a multichoice we're not going to be ready until it appears, until
// that point we should be processing keys for INTERACT. But also AFTER
// the multichoice is done we should go back to processing INTERACT.
// I think we're going to need explicit state machines.

void Test_Overworld_PlayerFace(u32 direction)
{
    WAIT_UNTIL(Overworld_PlayerInputReady());
    const struct ObjectEvent *playerObjectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    while (playerObjectEvent->facingDirection != direction)
        YIELD_KEYS(DirectionToDpad(direction), 0);
}

static bool32 PlayerCopyableMovementMaybeWalk(u32 copyableMovement)
{
    return copyableMovement == COPY_MOVE_WALK
        || copyableMovement == COPY_MOVE_JUMP2;
}

void Test_Overworld_PlayerWalk(u32 direction)
{
    WAIT_UNTIL(Overworld_PlayerInputReady());
    const struct ObjectEvent *playerObjectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    while (!PlayerCopyableMovementMaybeWalk(playerObjectEvent->playerCopyableMovement))
        YIELD_KEYS(0, DirectionToDpad(direction));
    // TODO: How do we wait until the movement is done?
}

// TODO: Be able to specify what we expect to interact with?
void Test_Overworld_Interact(void)
{
    WAIT_UNTIL(Overworld_PlayerInputReady());
    while (!ArePlayerFieldControlsLocked())
        YIELD_KEYS(A_BUTTON, 0);
    // TODO: We're in an interaction. We need to negotiate with the
    // interaction inputs. Hammer A until they're ready, then hand off
    // to them, then hammer A again. If the controls become unlocked
    // pass if there's no remaining inputs, or fail otherwise.
}

OVERWORLD_TEST("OVERWORLD")
{
    FACE_UP;
    INTERACT {
        SELECT(1);
    }
    STEP_DOWN;
}
