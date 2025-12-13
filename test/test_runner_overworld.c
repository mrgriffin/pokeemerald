#include <stdarg.h>
#include "global.h"
#include "field_message_box.h"
#include "list_menu.h"
#include "main.h"
#include "main_menu.h"
#include "menu.h"
#include "menu_helpers.h"
#include "overworld.h"
#include "palette.h"
#include "script.h"
#include "script_menu.h"
#include "shop.h"
#include "start_menu.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "window.h"
#include "test/overworld.h"
#include "constants/event_object_movement.h"

#define INVALID(fmt, ...) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__)
#define INVALID_IF(c, fmt, ...) do { if (c) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__); } while (0)

struct OverworldTestState
{
    u8 initialMapGroup;
    u8 initialMapNum;
    s8 initialMapX;
    s8 initialMapY;
    u8 commands[128];
    u16 currentCommand;
    u16 checkProgressCommand;
    union CommandState {
        struct { u8 frames; } delay;
        struct { u8 state; } pressKeys;
        struct { u8 frames; } holdKeys;
        struct { u8 state; } waitFade;
        struct { u8 state; bool8 newKey; } menuSelect;
        struct { u8 state; } walkDirection;
        struct { u8 state; } interactBegin;
        struct { u16 keys; } interactEnd;
        struct { u8 state; } startMenuBegin;
        struct { bool8 newKey; } startMenuEnd;
    } currentCommandState;
    bool8 inInteract:1;
};

EWRAM_DATA struct OverworldTestState gOverworldTestState = {0};

#define STATE gOverworldTestState

static u32 DirectionToDpad(u32 direction)
{
    switch (direction)
    {
    case DIR_SOUTH: return DPAD_DOWN;
    case DIR_NORTH: return DPAD_UP;
    case DIR_WEST:  return DPAD_LEFT;
    case DIR_EAST:  return DPAD_RIGHT;
    }

    u32 sourceLine = 0;
    INVALID("DirectionToDpad: invalid direction %d", direction);
}

static bool32 Overworld_Ready(void)
{
    return gMain.callback1 == CB1_Overworld
        && gMain.callback2 == CB2_Overworld;
}

static bool32 Overworld_PlayerInputReady(void)
{
    return gMain.callback1 == CB1_Overworld
        && gMain.callback2 == CB2_Overworld
        && !ArePlayerFieldControlsLocked();
}

enum Opcode
{
    OP_END,

    // Low-level.
    OP_DELAY,
    OP_PRESS_KEYS,
    OP_HOLD_KEYS,
    OP_WAIT_FADE_IN,

    // Menus.
    OP_MENU_SELECT,

    // OW-specific.
    OP_OW_FACE_DIRECTION,
    OP_OW_WALK_DIRECTION,
    OP_OW_INTERACT_BEGIN,
    OP_OW_INTERACT_END,
    OP_OW_START_MENU_BEGIN,
    OP_OW_START_MENU_END,
};

static void NextCommand(u32 size)
{
    STATE.currentCommand += size;
    memset(&STATE.currentCommandState, 0, sizeof(STATE.currentCommandState));
}

#define NEXT_CMD do { NextCommand(sizeof(*cmd)); } while (0)

#define CMD_ARGS(...) const struct __attribute__((packed)) { u8 opcode; RECURSIVELY(R_FOR_EACH(APPEND_SEMICOLON, __VA_ARGS__)) } *const cmd UNUSED = (const void *)&STATE.commands[STATE.currentCommand]

static u32 Cmd_End(union CommandState *s)
{
    CMD_ARGS();
    SetMainCallback2(CB2_TestRunner);
    return 0;
}

static u32 Cmd_Delay(union CommandState *s)
{
    CMD_ARGS(u16 frames);

    if (s->delay.frames++ >= cmd->frames)
        NEXT_CMD;

    return 0;
}

static u32 Cmd_PressKeys(union CommandState *s)
{
    CMD_ARGS(u16 keys);

    switch (s->pressKeys.state)
    {
    case 0:
        s->pressKeys.state++;
        return 0;
    case 1:
        NEXT_CMD;
        return cmd->keys;
    }

    return 0;
}

static u32 Cmd_HoldKeys(union CommandState *s)
{
    CMD_ARGS(u16 keys, u8 frames);

    if (s->holdKeys.frames++ >= cmd->frames)
        NEXT_CMD;

    return cmd->keys;
}

static u32 Cmd_WaitFadeIn(union CommandState *s)
{
    CMD_ARGS();

    switch (s->waitFade.state)
    {
    case 0:
        if (gPaletteFade.targetY == 0 && gPaletteFade.active)
            s->waitFade.state++;
        break;
    case 1:
        if (!gPaletteFade.active)
            NEXT_CMD;
        break;
    }

    return 0;
}

enum MenuMode
{
    MENU_NONE,
    MENU_START,
    MENU_SHOP,
    MENU_GENERIC_LISTMENU,
    MENU_GENERIC_YESNO,
    MENU_MULTICHOICE_YESNO,
};

static enum MenuMode CurrentMenuMode(void)
{
    enum MultichoiceType multichoiceType;

    if (GetStartMenuWindowId() != WINDOW_NONE)
    {
        return MENU_START;
    }

    if (InPokemartMenu())
    {
        return MENU_SHOP;
    }

    if (InYesNoMenu())
    {
        return MENU_GENERIC_YESNO;
    }

    if (InListMenu())
    {
        return MENU_GENERIC_LISTMENU;
    }

    if ((multichoiceType = ActiveMultichoiceType()) != MULTICHOICE_NONE)
    {
        switch (multichoiceType)
        {
        case MULTICHOICE_YESNO: return MENU_MULTICHOICE_YESNO;
        default: break;
        }
    }

    return MENU_NONE;
}

static u32 MenuTextIndex_StringCompare(const u8 *targetText, const u8 *(*getText)(u32))
{
    for (u32 i = 0;; i++)
    {
        u8 expandedText[32];
        const u8 *text = getText(i);
        if (!text)
            break;
        StringExpandPlaceholders(expandedText, text);
        if (StringCompare(expandedText, targetText) == 0)
            return i;
    }

    u32 sourceLine = 0;
    INVALID("SELECT(\"%S\"): not found", targetText);
}

static u32 MenuTextIndex(const u8 *targetText)
{
    switch (CurrentMenuMode())
    {
    case MENU_START:
        return MenuTextIndex_StringCompare(targetText, StartMenu_ItemText);

    case MENU_SHOP:
        return MenuTextIndex_StringCompare(targetText, PokemartMenu_ItemText);

    case MENU_GENERIC_LISTMENU:
        return MenuTextIndex_StringCompare(targetText, ListMenu_ItemText);

    case MENU_GENERIC_YESNO:
    case MENU_MULTICHOICE_YESNO:
        if (StringCompare(targetText, gText_Yes) == 0)
            return 0;
        else if (StringCompare(targetText, gText_No) == 0)
            return 1;
        break;

    case MENU_NONE:
        return 0;
    }

    u32 sourceLine = 0;
    INVALID("SELECT(\"%S\"): not found", targetText);
}

static u32 Cmd_MenuSelect(union CommandState *s)
{
    CMD_ARGS(uintptr_t argument);

    enum MenuMode mode = CurrentMenuMode();
    s->menuSelect.newKey ^= TRUE;

    u32 keys = 0;

    switch (s->menuSelect.state)
    {
    case 0:
        if (mode == MENU_NONE)
        {
            if (STATE.inInteract && Overworld_Ready())
            {
                if (!IsFieldMessageBoxHidden())
                    return s->menuSelect.newKey ? A_BUTTON : 0;
            }
            return 0;
        }

        if (s->menuSelect.newKey)
        {
            u32 cursorPos, targetPos;
            switch (cmd->argument >> 24)
            {
            // Pointers.
            case 0x02:
            case 0x03:
            case 0x08:
            case 0x09:
                // TODO: Cache this.
                targetPos = MenuTextIndex((const u8 *)cmd->argument);
                break;
            default:
                targetPos = cmd->argument;
                break;
            }

            switch (mode)
            {
            case MENU_START:
            case MENU_SHOP:
            case MENU_GENERIC_YESNO:
            case MENU_MULTICHOICE_YESNO:
                cursorPos = Menu_GetCursorPos();
                break;

            case MENU_GENERIC_LISTMENU:
                // Wait until text printers are done.
                if (AnyTextPrinterActive())
                    return 0;
                cursorPos = ListMenu_CursorPos();
                break;

            case MENU_NONE:
                cursorPos = 0;
                break;
            }

            if (cursorPos < targetPos)
            {
                keys = DPAD_DOWN;
            }
            else if (cursorPos > targetPos)
            {
                keys = DPAD_UP;
            }
            else
            {
                keys = A_BUTTON;
                s->menuSelect.state = 1;
            }
        }

        break;

    case 1:
        if (mode == MENU_NONE || mode == MENU_GENERIC_LISTMENU)
            NEXT_CMD;
        else
            s->menuSelect.state = 0;
        break;
    }

    return keys;
}

static u32 Cmd_Overworld_FaceDirection(union CommandState *s)
{
    CMD_ARGS(u8 direction);

    if (!Overworld_PlayerInputReady())
        return 0;

    const struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (objectEvent->facingDirection != cmd->direction)
    {
        return DirectionToDpad(cmd->direction);
    }
    else
    {
        NEXT_CMD;
        return 0;
    }
}

static u32 Cmd_Overworld_WalkDirection(union CommandState *s)
{
    CMD_ARGS(u8 direction);

    if (!Overworld_PlayerInputReady())
        return 0;

    const struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    switch (s->walkDirection.state)
    {
    case 0:
        if (objectEvent->playerCopyableMovement != COPY_MOVE_WALK
         && objectEvent->playerCopyableMovement != COPY_MOVE_JUMP2) // ledge
        {
            return DirectionToDpad(cmd->direction);
        }
        else
        {
            s->walkDirection.state++;
        }
        break;
    case 1:
        if (objectEvent->heldMovementFinished)
            NEXT_CMD;
        break;
    }

    return 0;
}

static u32 Cmd_Overworld_InteractBegin(union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    switch (s->interactBegin.state)
    {
    case 0:
        if (!ArePlayerFieldControlsLocked())
        {
            s->interactBegin.state = 1;
            return A_BUTTON;
        }
        break;
    case 1:
        if (!ArePlayerFieldControlsLocked())
        {
            s->interactBegin.state = 0;
        }
        else
        {
            NEXT_CMD;
            STATE.inInteract = TRUE;
        }
        break;
    }

    return 0;
}

static u32 Cmd_Overworld_InteractEnd(union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    if (ArePlayerFieldControlsLocked())
    {
        u32 sourceLine = 0;
        INVALID_IF(CurrentMenuMode() != MENU_NONE, "cannot end interaction while a menu is open");
        return s->interactEnd.keys ^= A_BUTTON;
    }
    else
    {
        NEXT_CMD;
        STATE.inInteract = FALSE;
    }

    return 0;
}

static u32 Cmd_Overworld_StartMenuBegin(union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    switch (s->startMenuBegin.state)
    {
    case 0:
        if (GetStartMenuWindowId() == WINDOW_NONE)
        {
            if (Overworld_PlayerInputReady())
                return START_BUTTON;
        }
        else
        {
            s->startMenuBegin.state++;
        }
        break;
    case 1:
        NEXT_CMD;
        break;
    }

    return 0;
}

static u32 Cmd_Overworld_StartMenuEnd(union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    s->startMenuEnd.newKey ^= TRUE;

    if (GetStartMenuWindowId() != WINDOW_NONE)
    {
        return s->startMenuEnd.newKey ? B_BUTTON : 0;
    }
    else
    {
        NEXT_CMD;
        return 0;
    }
}

static const u32 (*sCommands[])(union CommandState *) =
{
    [OP_END] = Cmd_End,
    [OP_DELAY] = Cmd_Delay,
    [OP_PRESS_KEYS] = Cmd_PressKeys,
    [OP_HOLD_KEYS] = Cmd_HoldKeys,
    [OP_WAIT_FADE_IN] = Cmd_WaitFadeIn,
    [OP_MENU_SELECT] = Cmd_MenuSelect,
    [OP_OW_FACE_DIRECTION] = Cmd_Overworld_FaceDirection,
    [OP_OW_WALK_DIRECTION] = Cmd_Overworld_WalkDirection,
    [OP_OW_INTERACT_BEGIN] = Cmd_Overworld_InteractBegin,
    [OP_OW_INTERACT_END] = Cmd_Overworld_InteractEnd,
    [OP_OW_START_MENU_BEGIN] = Cmd_Overworld_StartMenuBegin,
    [OP_OW_START_MENU_END] = Cmd_Overworld_StartMenuEnd,
};

u32 TestRunner_ReadKeys(u32 prevKeyInput)
{
    return sCommands[STATE.commands[STATE.currentCommand]](&STATE.currentCommandState);
}

void TestRunner_Overworld_SetInitialWarpDestination(void)
{
    SetWarpDestination(STATE.initialMapGroup, STATE.initialMapNum, WARP_ID_NONE, STATE.initialMapX, STATE.initialMapY);
}

static void OverworldTest_Run(void *data)
{
    memset(&STATE, 0, sizeof(STATE));
    void (*function)(void) = data;
    function();
    gOverworldTestState.currentCommand = 0;
    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, COMPOUND_STRING("PLAYER"));
    SetMainCallback2(CB2_NewGame);
}

static bool32 OverworldTest_CheckProgress(void *data)
{
    bool32 madeProgress
         = STATE.checkProgressCommand < STATE.currentCommand;
    STATE.checkProgressCommand = STATE.currentCommand;
    return madeProgress;
}

const struct TestRunner gOverworldTestRunner =
{
    .run = OverworldTest_Run,
    .checkProgress = OverworldTest_CheckProgress,
};

enum CommandArgumentType
{
    ARG_END,
    ARG_8,
    ARG_16,
    ARG_32,
};

static void PushByte(u32 sourceLine, u32 byte)
{
    INVALID_IF(gOverworldTestState.currentCommand == sizeof(gOverworldTestState.commands) - 1, "maximum test size reached");
    INVALID_IF(byte > 0xFF, "invalid byte %x", byte);
    gOverworldTestState.commands[gOverworldTestState.currentCommand++] = byte;
}

void OverworldTest_PushCommand(u32 sourceLine, enum Opcode opcode, ...)
{
    va_list va;
    va_start(va, opcode);
    PushByte(sourceLine, opcode);
    while (TRUE)
    {
        u32 arg;
        switch (va_arg(va, enum CommandArgumentType))
        {
        case ARG_END:
            return;
        case ARG_8:
            arg = va_arg(va, u32);
            PushByte(sourceLine, arg);
            break;
        case ARG_16:
            arg = va_arg(va, u32);
            PushByte(sourceLine, arg & 0xFF);
            PushByte(sourceLine, arg >> 8);
            break;
        case ARG_32:
            arg = va_arg(va, u32);
            PushByte(sourceLine, arg & 0xFF);
            PushByte(sourceLine, (arg >> 8) & 0xFF);
            PushByte(sourceLine, (arg >> 16) & 0xFF);
            PushByte(sourceLine, arg >> 24);
            break;
        }
    }
}

#define GIVEN if (1)
#define WHEN if (1)

#define ON_MAP(map, x, y) \
    do { \
        STATE.initialMapGroup = MAP_GROUP(map); \
        STATE.initialMapNum = MAP_NUM(map); \
        STATE.initialMapX = (x); \
        STATE.initialMapY = (y); \
    } while (0)

/* TODO:
 * - Smaller commands for PRESS_KEY / HOLD_KEY.
 * - HOLD_KEYS(0, frames) => DELAY(frames). */

#define DELAY(frames) OverworldTest_PushCommand(__LINE__, OP_DELAY, ARG_16, frames, ARG_END)
#define PRESS_KEYS(keys) OverworldTest_PushCommand(__LINE__, OP_PRESS_KEYS, ARG_16, keys, ARG_END)
#define HOLD_KEYS(keys, frames) OverworldTest_PushCommand(__LINE__, OP_HOLD_KEYS, ARG_16, keys, ARG_8, frames, ARG_END)
#define WAIT_FADE_IN OverworldTest_PushCommand(__LINE__, OP_WAIT_FADE_IN, ARG_END)

// TODO: Support passing a pointer.
#define SELECT(text) OverworldTest_PushCommand(__LINE__, OP_MENU_SELECT, ARG_32, (static const u8[]) _(text), ARG_END)
#define SELECT_INDEX(index) OverworldTest_PushCommand(__LINE__, OP_MENU_SELECT, ARG_32, index, ARG_END)

#define FACE_DOWN OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_SOUTH, ARG_END)
#define FACE_UP OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_NORTH, ARG_END)
#define FACE_LEFT OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_WEST, ARG_END)
#define FACE_RIGHT OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_EAST, ARG_END)

#define WALK_DOWN OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_SOUTH, ARG_END)
#define WALK_UP OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_NORTH, ARG_END)
#define WALK_LEFT OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_WEST, ARG_END)
#define WALK_RIGHT OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_EAST, ARG_END)

#define INTERACT \
    for (bool32 _once = TRUE; \
         _once && (OverworldTest_PushCommand(__LINE__, OP_OW_INTERACT_BEGIN, ARG_END), TRUE); \
         OverworldTest_PushCommand(__LINE__, OP_OW_INTERACT_END, ARG_END), _once = FALSE)

#define START_MENU \
    for (bool32 _once = TRUE; \
         _once && (OverworldTest_PushCommand(__LINE__, OP_OW_START_MENU_BEGIN, ARG_END), TRUE); \
         OverworldTest_PushCommand(__LINE__, OP_OW_START_MENU_END, ARG_END), _once = FALSE)

OVERWORLD_TEST("OVERWORLD")
{
    GIVEN {
        ON_MAP(MAP_PETALBURG_CITY_MART, -1, -1);
    } WHEN {
        WALK_UP;
        WALK_LEFT;
        WALK_LEFT;
        INTERACT {
            SELECT("BUY");
            SELECT("Potion");
            DELAY(240);
            PRESS_KEYS(A_BUTTON); // 1x
            SELECT("YES");
            DELAY(240);
            PRESS_KEYS(A_BUTTON); // "Here you go! Thank you very much."
            SELECT("CANCEL");
        }
        /*
        START_MENU {
            SELECT("BAG");
            WAIT_FADE_IN;
        }
        */
    }
}
