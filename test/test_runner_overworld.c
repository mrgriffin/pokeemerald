#include <stdarg.h>
#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "item_menu.h"
#include "list_menu.h"
#include "load_save.h"
#include "main.h"
#include "main_menu.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
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
#include "constants/test_runner.h"

#define INVALID(fmt, ...) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__)
#define INVALID_IF(c, fmt, ...) do { if (c) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__); } while (0)

// NOTE: 'commands' could be a cache of the next 128 bytes, and when
// exhausted we could call the function again to refill them.
struct OverworldTestState
{
    u8 commands[128];
    u16 currentCommand;
    u16 checkProgressCommand;
    union CommandState {
        struct { u8 frames; } delay;
        struct { u8 frames; } holdKeys;
        struct { u8 state; } waitFade;
        struct { u8 waitTextPrintersState; } menuSelect;
        struct { u8 waitTextPrintersState; } menuQuantity;
        struct { u8 state; } walkDirection;
        struct { u8 state; } interactBegin;
        struct { u16 keys; } interactEnd;
        struct { u8 state; } startMenuBegin;
    } currentCommandState;
    // WARNING: currentMenuInput is lagged by a frame.
    enum MenuInputType currentMenuInputType:8;
    s32 currentMenuInputValue;
    uintptr_t currentMenuInputContext;
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

    u32 sourceLine = SourceLine(0);
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
    OP_MENU_QUANTITY,

    // OW-specific.
    OP_OW_FACE_DIRECTION,
    OP_OW_WALK_DIRECTION,
    OP_OW_RUN_DIRECTION,
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

static u32 Cmd_End(u32, union CommandState *)
{
    CMD_ARGS();
    SetMainCallback2(CB2_TestRunner);
    return 0;
}

static u32 Cmd_Delay(u32, union CommandState *s)
{
    CMD_ARGS(u16 frames);

    if (s->delay.frames++ >= cmd->frames)
        NEXT_CMD;

    return 0;
}

static u32 Cmd_PressKeys(u32 prevKeys, union CommandState *)
{
    CMD_ARGS(u16 keys);

    if (prevKeys == 0)
    {
        NEXT_CMD;
        return cmd->keys;
    }
    else
    {
        return 0;
    }
}

static u32 Cmd_HoldKeys(u32 prevKeys, union CommandState *s)
{
    CMD_ARGS(u16 keys, u8 frames);

    if (prevKeys == cmd->keys)
    {
        if (++s->holdKeys.frames >= cmd->frames)
            NEXT_CMD;
    }

    return cmd->keys;
}

static u32 Cmd_WaitFadeIn(u32, union CommandState *s)
{
    CMD_ARGS();

    switch (s->waitFade.state)
    {
    case 0: // Wait for the fade to start.
        if (gPaletteFade.targetY == 0 && gPaletteFade.active)
            s->waitFade.state++;
        break;
    case 1: // Wait for the fade to end.
        if (!gPaletteFade.active)
            NEXT_CMD;
        break;
    }

    return 0;
}

static u32 TryWaitTextPrinters(u32 prevKeys, u8 *waitTextPrintersState)
{
    if (gPaletteFade.active)
        return 0;

    switch (TextPrinterState())
    {
    case TEXT_PRINTER_INACTIVE:
        if (*waitTextPrintersState > 0)
        {
            // Try an A press if we are stuck for 30 frames.
            if (--(*waitTextPrintersState) == 0)
            {
                if (prevKeys == 0)
                    return A_BUTTON;
                else // Would be a hold, try next frame.
                    *waitTextPrintersState = 1;
            }
        }
        break;

    case TEXT_PRINTER_ACTIVE:
        *waitTextPrintersState = 30;
        // Hold A to try and speed up.
        return A_BUTTON;

    case TEXT_PRINTER_AWAIT_PRESS:
        if (prevKeys == 0)
            return A_BUTTON;
        break;
    }

    return 0;
}

static s32 MenuTextIndex(enum MenuInputType type, uintptr_t context, const u8 *text)
{
    u32 sourceLine = SourceLine(0);

    switch (type)
    {
    case MENU_INPUT_NONE:
        return -1;

    case MENU_INPUT_MENU:
    case MENU_INPUT_GRIDMENU:
    {
        const u8 *(*getText)(u32 index);
        if ((getText = IsYesNoMenuWindow(context))
         || (getText = IsPokemartMenuWindow(context))
         || (getText = IsStartMenuWindow(context))
         || (getText = IsBagMenuWindow(context)))
        {
            const u8 *text_;
            u8 expandedText_[32];
            for (u32 i = 0; (text_ = getText(i)); i++)
            {
                StringExpandPlaceholders(expandedText_, text_);
                if (StringCompare(expandedText_, text) == 0)
                    return i;
            }
        }
        return -1;
    }

    case MENU_INPUT_LISTMENU:
    {
        struct ListMenu *list = (void *) gTasks[context].data;
        INVALID_IF(list->template.isDynamic, "MENU_INPUT_LISTMENU isDynamic unimplemented");
        for (u32 i = 0; i < list->template.totalItems; i++)
        {
            u8 expandedText_[32];
            StringExpandPlaceholders(expandedText_, list->template.items[i].name);
            if (StringCompare(expandedText_, text) == 0)
                return i;
        }
        return -1;
    }

    case MENU_INPUT_QUANTITY:
        return -1;
    }

    return -1;
}

static u32 Cmd_MenuSelect(u32 prevKeys, union CommandState *s)
{
    CMD_ARGS(uintptr_t argument);

    if (STATE.currentMenuInputType == MENU_INPUT_NONE)
        return TryWaitTextPrinters(prevKeys, &s->menuQuantity.waitTextPrintersState);

    if (prevKeys)
        return 0;

    s32 index;
    switch (cmd->argument >> 24)
    {
    // Pointers.
    case 0x02:
    case 0x03:
    case 0x08:
    case 0x09:
        index = MenuTextIndex(STATE.currentMenuInputType, STATE.currentMenuInputContext, (const u8 *)cmd->argument);
        if (index < 0)
            return 0;
        break;
    default:
        index = cmd->argument;
        break;
    }

    switch (STATE.currentMenuInputType)
    {
    case MENU_INPUT_MENU:
    case MENU_INPUT_LISTMENU:
        if (STATE.currentMenuInputValue < index)
        {
            return DPAD_DOWN;
        }
        else if (STATE.currentMenuInputValue > index)
        {
            return DPAD_UP;
        }
        else
        {
            NEXT_CMD;
            return A_BUTTON;
        }

    case MENU_INPUT_GRIDMENU:
    {
        u32 columns = GetGridMenuColumns();
        if (STATE.currentMenuInputValue < index - columns)
        {
            return DPAD_RIGHT;
        }
        else if (STATE.currentMenuInputValue > index + columns)
        {
            return DPAD_LEFT;
        }
        else if (STATE.currentMenuInputValue < index)
        {
            return DPAD_DOWN;
        }
        else if (STATE.currentMenuInputValue > index)
        {
            return DPAD_UP;
        }
        else
        {
            NEXT_CMD;
            return A_BUTTON;
        }
        break;
    }

    case MENU_INPUT_NONE:
    case MENU_INPUT_QUANTITY:
    }

    return 0;
}

static u32 Cmd_MenuQuantity(u32 prevKeys, union CommandState *s)
{
    CMD_ARGS(u16 quantity);

    if (STATE.currentMenuInputType == MENU_INPUT_NONE)
        return TryWaitTextPrinters(prevKeys, &s->menuQuantity.waitTextPrintersState);

    if (prevKeys)
        return 0;

    if (STATE.currentMenuInputValue < cmd->quantity)
    {
        return DPAD_UP;
    }
    else if (STATE.currentMenuInputValue > cmd->quantity)
    {
        return DPAD_DOWN;
    }
    else
    {
        NEXT_CMD;
        return A_BUTTON;
    }
}

static u32 Cmd_Overworld_FaceDirection(u32, union CommandState *)
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

static u32 Cmd_Overworld_WalkDirection(u32, union CommandState *s)
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

static u32 Cmd_Overworld_RunDirection(u32, union CommandState *s)
{
    CMD_ARGS(u8 direction);

    u32 sourceLine = SourceLine(0);
    INVALID_IF(!FlagGet(FLAG_SYS_B_DASH), "cannot run without running shoes");

    if (!Overworld_PlayerInputReady())
        return 0;

    const struct ObjectEvent *objectEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

    switch (s->walkDirection.state)
    {
    case 0:
        if (objectEvent->playerCopyableMovement != COPY_MOVE_WALK
         && objectEvent->playerCopyableMovement != COPY_MOVE_JUMP2) // ledge
        {
            return DirectionToDpad(cmd->direction | B_BUTTON);
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

static u32 Cmd_Overworld_InteractBegin(u32, union CommandState *s)
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
            s->interactBegin.state = 0;
        else
            NEXT_CMD;
        break;
    }

    return 0;
}

static u32 Cmd_Overworld_InteractEnd(u32, union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    if (ArePlayerFieldControlsLocked())
    {
        return s->interactEnd.keys ^= B_BUTTON;
    }
    else
    {
        NEXT_CMD;
        return 0;
    }
}

static u32 Cmd_Overworld_StartMenuBegin(u32, union CommandState *s)
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

static u32 Cmd_Overworld_StartMenuEnd(u32 prevKeys, union CommandState *s)
{
    CMD_ARGS();

    if (!Overworld_Ready())
        return 0;

    if (GetStartMenuWindowId() != WINDOW_NONE)
    {
        return prevKeys ? 0 : B_BUTTON;
    }
    else
    {
        NEXT_CMD;
        return 0;
    }
}

static const u32 (*sCommands[])(u32 prevKeys, union CommandState *) =
{
    [OP_END] = Cmd_End,
    [OP_DELAY] = Cmd_Delay,
    [OP_PRESS_KEYS] = Cmd_PressKeys,
    [OP_HOLD_KEYS] = Cmd_HoldKeys,
    [OP_WAIT_FADE_IN] = Cmd_WaitFadeIn,
    [OP_MENU_SELECT] = Cmd_MenuSelect,
    [OP_MENU_QUANTITY] = Cmd_MenuQuantity,
    [OP_OW_FACE_DIRECTION] = Cmd_Overworld_FaceDirection,
    [OP_OW_WALK_DIRECTION] = Cmd_Overworld_WalkDirection,
    [OP_OW_RUN_DIRECTION] = Cmd_Overworld_RunDirection,
    [OP_OW_INTERACT_BEGIN] = Cmd_Overworld_InteractBegin,
    [OP_OW_INTERACT_END] = Cmd_Overworld_InteractEnd,
    [OP_OW_START_MENU_BEGIN] = Cmd_Overworld_StartMenuBegin,
    [OP_OW_START_MENU_END] = Cmd_Overworld_StartMenuEnd,
};

u32 TestRunner_ReadKeys(u32 prevKeys)
{
    u32 keys = sCommands[STATE.commands[STATE.currentCommand]](prevKeys, &STATE.currentCommandState);
    STATE.currentMenuInputType = MENU_INPUT_NONE;
    return keys;
}

void TestRunner_Overworld_MenuInputHasFocus(enum MenuInputType type, s32 value, uintptr_t context)
{
    STATE.currentMenuInputType = type;
    STATE.currentMenuInputValue = value;
    STATE.currentMenuInputContext = context;
}

static void OverworldTest_Run(void *data)
{
    memset(&STATE, 0, sizeof(STATE));
    void (*function)(void) = data;
    function();
    gOverworldTestState.currentCommand = 0;
    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy_PlayerName(gSaveBlock2Ptr->playerName, COMPOUND_STRING("PLAYER"));
    SetContinueGameWarpStatus();
    SetMainCallback2(CB2_ContinueSavedGame);
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

#define ON_MAP(map, x, y) SetContinueGameWarp(MAP_GROUP(map), MAP_NUM(map), -1, (x), (y))

/* TODO:
 * - Smaller commands for PRESS_KEY / HOLD_KEY.
 * - HOLD_KEYS(0, frames) => DELAY(frames).
 * - DELAY(0) / HOLD_KEYS(_, 0): error. */

#define DELAY(frames) OverworldTest_PushCommand(__LINE__, OP_DELAY, ARG_16, frames, ARG_END)
#define PRESS_KEYS(keys) OverworldTest_PushCommand(__LINE__, OP_PRESS_KEYS, ARG_16, keys, ARG_END)
#define HOLD_KEYS(keys, frames) OverworldTest_PushCommand(__LINE__, OP_HOLD_KEYS, ARG_16, keys, ARG_8, frames, ARG_END)
#define WAIT_FADE_IN OverworldTest_PushCommand(__LINE__, OP_WAIT_FADE_IN, ARG_END)

// TODO: Support passing a pointer.
#define SELECT(text) OverworldTest_PushCommand(__LINE__, OP_MENU_SELECT, ARG_32, (static const u8[]) _(text), ARG_END)
#define SELECT_INDEX(index) OverworldTest_PushCommand(__LINE__, OP_MENU_SELECT, ARG_32, index, ARG_END)
#define QUANTITY(n) OverworldTest_PushCommand(__LINE__, OP_MENU_QUANTITY, ARG_16, n, ARG_END)

#define FACE_DOWN OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_SOUTH, ARG_END)
#define FACE_UP OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_NORTH, ARG_END)
#define FACE_LEFT OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_WEST, ARG_END)
#define FACE_RIGHT OverworldTest_PushCommand(__LINE__, OP_OW_FACE_DIRECTION, ARG_8, DIR_EAST, ARG_END)

#define WALK_DOWN OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_SOUTH, ARG_END)
#define WALK_UP OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_NORTH, ARG_END)
#define WALK_LEFT OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_WEST, ARG_END)
#define WALK_RIGHT OverworldTest_PushCommand(__LINE__, OP_OW_WALK_DIRECTION, ARG_8, DIR_EAST, ARG_END)

#define RUN_DOWN OverworldTest_PushCommand(__LINE__, OP_OW_RUN_DIRECTION, ARG_8, DIR_SOUTH, ARG_END)
#define RUN_UP OverworldTest_PushCommand(__LINE__, OP_OW_RUN_DIRECTION, ARG_8, DIR_NORTH, ARG_END)
#define RUN_LEFT OverworldTest_PushCommand(__LINE__, OP_OW_RUN_DIRECTION, ARG_8, DIR_WEST, ARG_END)
#define RUN_RIGHT OverworldTest_PushCommand(__LINE__, OP_OW_RUN_DIRECTION, ARG_8, DIR_EAST, ARG_END)

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
        SetMoney(&gSaveBlock1Ptr->money, 3000);
    } WHEN {
        WALK_UP;
        WALK_LEFT;
        WALK_LEFT;
        INTERACT {
            SELECT("BUY");
            SELECT("Potion");
            QUANTITY(3);
            SELECT("YES");
            SELECT("CANCEL");
            SELECT("QUIT");
        }
        START_MENU {
            SELECT("BAG");
            SELECT("Potion");
            SELECT("TOSS");
            QUANTITY(1);
            SELECT("YES");
        }
    }
}
