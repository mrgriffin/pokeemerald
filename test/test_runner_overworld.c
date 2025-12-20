#include <stdarg.h>
#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "list_menu.h"
#include "load_save.h"
#include "main.h"
#include "main_menu.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "window.h"
#include "test/overworld.h"
#include "constants/event_object_movement.h"
#include "constants/test_runner.h"

#define INVALID(fmt, ...) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__)
#define INVALID_IF(c, fmt, ...) do { if (c) Test_ExitWithResult(TEST_RESULT_INVALID, sourceLine, ":L%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__); } while (0)

struct PrintedText
{
    u8 windowId;
    u8 x;
    u8 y;
    bool8 freeText:1;
    union {
        const u8 *as_const;
        u8 *as_mut;
    } text;
    struct PrintedText *next;
};

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
    bool8 didInitialWarp:1;
    bool8 didWarp:1;
    // WARNING: currentMenuInput is lagged by a frame.
    enum MenuInputType currentMenuInputType:8;
    s32 currentMenuInputValue;
    uintptr_t currentMenuInputContext;
    struct PrintedText *printedTextHead;
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

static u32 RowOfPrintedText(const struct PrintedText *printedText)
{
    u32 row = 0;
    for (const struct PrintedText *current = STATE.printedTextHead; current; current = current->next)
    {
        if (current->windowId != printedText->windowId)
            continue;
        if (current->x != printedText->x)
            continue;
        if (current->y < printedText->y)
            row++;
    }
    return row;
}

static u32 ColumnOfPrintedText(const struct PrintedText *printedText)
{
    u32 column = 0;
    for (const struct PrintedText *current = STATE.printedTextHead; current; current = current->next)
    {
        if (current->windowId != printedText->windowId)
            continue;
        if (current->y != printedText->y)
            continue;
        // Exclude the cursor.
        if (current->text.as_const[0] == CHAR_BLACK_TRIANGLE)
            continue;
        if (current->x < printedText->x)
            column++;
    }
    return column;
}

// TODO: Define for '-1'.
static s32 MenuTextIndex(enum MenuInputType type, uintptr_t context, const u8 *text)
{
    u32 sourceLine = SourceLine(0);

    switch (type)
    {
    case MENU_INPUT_NONE:
        return -1;

    case MENU_INPUT_MENU:
    {
        for (const struct PrintedText *current = STATE.printedTextHead; current; current = current->next)
        {
            if (current->windowId != STATE.currentMenuInputContext)
                continue;
            if (StringCompareWithoutExtCtrlCodes(current->text.as_const, text) != 0)
                continue;
            return RowOfPrintedText(current);
        }
        return -1;
    }

    case MENU_INPUT_GRIDMENU:
    {
        for (const struct PrintedText *current = STATE.printedTextHead; current; current = current->next)
        {
            if (current->windowId != STATE.currentMenuInputContext)
                continue;
            if (StringCompareWithoutExtCtrlCodes(current->text.as_const, text) != 0)
                continue;
            return RowOfPrintedText(current) * GetGridMenuColumns() + ColumnOfPrintedText(current);
        }
        return -1;
    }

    case MENU_INPUT_LISTMENU:
    {
        const struct ListMenu *list = (void *)gTasks[context].data;
        INVALID_IF(list->template.isDynamic, "MENU_INPUT_LISTMENU isDynamic unimplemented");
        for (u32 i = 0; i < list->template.totalItems; i++)
        {
            // XXX: Use StringExpandPlaceholdersLength.
            u8 expandedText_[32];
            StringExpandPlaceholders(expandedText_, list->template.items[i].name);
            if (StringCompareWithoutExtCtrlCodes(expandedText_, text) == 0)
                return i;
        }
        return -1;
    }

    case MENU_INPUT_QUANTITY:
        return -1;

    case MENU_INPUT_PARTY:
        // TODO: What if the party menu isn't showing gPlayerParty, e.g.
        // in a partner battle.
        for (u32 i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE)
                break;
            u8 nickname[POKEMON_NAME_LENGTH + 1];
            GetMonData(&gPlayerParty[i], MON_DATA_NICKNAME, nickname);
            if (StringCompare(nickname, text) == 0)
                return i;
        }
        if (StringCompare(gText_Confirm2, text) == 0)
            return PARTY_SIZE;
        else if (StringCompare(gText_Cancel, text) == 0)
            return PARTY_SIZE + 1;
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

    case MENU_INPUT_PARTY:
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
        break;

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
         && objectEvent->playerCopyableMovement != COPY_MOVE_JUMP2
         && !STATE.didWarp) // ledge
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
    [OP_OW_INTERACT_BEGIN] = Cmd_Overworld_InteractBegin,
    [OP_OW_INTERACT_END] = Cmd_Overworld_InteractEnd,
    [OP_OW_START_MENU_BEGIN] = Cmd_Overworld_StartMenuBegin,
    [OP_OW_START_MENU_END] = Cmd_Overworld_StartMenuEnd,
};

u32 TestRunner_ReadKeys(u32 prevKeys)
{
    u32 currentCommand = STATE.currentCommand;
    u32 keys = sCommands[STATE.commands[currentCommand]](prevKeys, &STATE.currentCommandState);
    if (currentCommand != STATE.currentCommand)
        STATE.didWarp = FALSE;
    STATE.currentMenuInputType = MENU_INPUT_NONE;
    return keys;
}

void TestRunner_BeforeResetHeap(void)
{
    struct PrintedText *current = STATE.printedTextHead;
    while (current)
    {
        if (current->freeText)
            Free(current->text.as_mut);
        Free(current);
        // WARNING: Use-after-free.
        current = current->next;
    }
    STATE.printedTextHead = NULL;

    if (!gTestRunnerState.expectLeaks)
    {
        TestRunner_CheckMemoryLeak();
        TestRunner_CheckTaskLeak();
    }
}

static void DiscardPrintedTextsOnWindow(u32 windowId)
{
    struct PrintedText **next = &STATE.printedTextHead;
    while (*next)
    {
        if ((*next)->windowId == windowId)
        {
            if ((*next)->freeText)
                Free((*next)->text.as_mut);
            Free(*next);
            // WARNING: Use-after-free.
            *next = (*next)->next;
        }
        else
        {
            next = &(*next)->next;
        }
    }
}

static void DiscardPrintedTextOnWindowAtXY(u32 windowId, u32 x, u32 y, struct PrintedText **next)
{
    while (*next)
    {
        if ((*next)->windowId == windowId && (*next)->x == x && (*next)->y == y)
        {
            if ((*next)->freeText)
                Free((*next)->text.as_mut);
            Free(*next);
            // WARNING: Use-after-free.
            *next = (*next)->next;
        }
        else
        {
            next = &(*next)->next;
        }
    }
}

void TestRunner_MenuInputHasFocus(enum MenuInputType type, s32 value, uintptr_t context)
{
    STATE.currentMenuInputType = type;
    STATE.currentMenuInputValue = value;
    STATE.currentMenuInputContext = context;

    // Eagerly discard printed texts where possible.
    if (type == MENU_INPUT_LISTMENU)
    {
        const struct ListMenu *list = (void *)gTasks[context].data;
        DiscardPrintedTextsOnWindow(list->template.windowId);
    }
}

void TestRunner_WindowAdded(u32 windowId)
{
    DiscardPrintedTextsOnWindow(windowId);
}

void TestRunner_WindowRemoved(u32 windowId)
{
    DiscardPrintedTextsOnWindow(windowId);
}

static bool32 MustExpandOrAllocateString(const u8 *string)
{
    switch ((uintptr_t)string >> 24)
    {
    case 0x02: // EWRAM
    case 0x03: // IWRAM
        return TRUE;
    }

    enum { MODE_NORMAL, MODE_EXT_CTRL_CODE, MODE_PLACEHOLDER } mode = MODE_NORMAL;
    for (u32 i = 0; string[i] != EOS; i++)
    {
        switch (mode)
        {
        case MODE_NORMAL:
            switch (string[i])
            {
            case CHAR_DYNAMIC:
                return TRUE;
            case EXT_CTRL_CODE_BEGIN:
                mode = MODE_EXT_CTRL_CODE;
                break;
            case PLACEHOLDER_BEGIN:
                mode = MODE_PLACEHOLDER;
                break;
            // Options on multiple lines.
            case CHAR_NEWLINE:
                return TRUE;
            }
            break;

        case MODE_EXT_CTRL_CODE:
            i += GetExtCtrlCodeLength(string[i]);
            mode = MODE_NORMAL;
            break;

        case MODE_PLACEHOLDER:
            switch (string[i])
            {
            case PLACEHOLDER_ID_STRING_VAR_1:
            case PLACEHOLDER_ID_STRING_VAR_2:
            case PLACEHOLDER_ID_STRING_VAR_3:
                return TRUE;
            }
            mode = MODE_NORMAL;
            break;
        }
    }

    return FALSE;
}

void TestRunner_TextPrinterAdded(const struct TextPrinter *textPrinter)
{
    // Animated text, can't be a menu option.
    // TODO: Think of more filters to reduce memory usage.
    if (textPrinter->textSpeed != 0 && textPrinter->textSpeed != TEXT_SKIP_DRAW)
        return;

    // TODO: If the exact (windowId, x, y) already exists, replace it.
    if (MustExpandOrAllocateString(textPrinter->printerTemplate.currentChar))
    {
        u32 length = StringExpandPlaceholdersLength(textPrinter->printerTemplate.currentChar);
        u8 *text = Alloc(length + 1);
        StringExpandPlaceholders(text, textPrinter->printerTemplate.currentChar);
        struct PrintedText *printedText = Alloc(sizeof(*printedText));
        *printedText = (struct PrintedText) {
            .windowId = textPrinter->printerTemplate.windowId,
            .x = textPrinter->printerTemplate.x,
            .y = textPrinter->printerTemplate.y,
            .freeText = TRUE,
            .text.as_mut = text,
            .next = STATE.printedTextHead,
        };
        STATE.printedTextHead = printedText;
        DiscardPrintedTextOnWindowAtXY(printedText->windowId, printedText->x, printedText->y, &printedText->next);

        // Split on newlines. Needed for gText_YesNo.
        for (u32 i = 0; text[i] != EOS; i++)
        {
            if (text[i] != CHAR_NEWLINE)
                continue;

            text[i] = EOS;
            printedText = Alloc(sizeof(*printedText));
            *printedText = *STATE.printedTextHead;
            printedText->y += gFonts[textPrinter->printerTemplate.fontId].maxLetterHeight + textPrinter->printerTemplate.lineSpacing;
            printedText->freeText = FALSE;
            printedText->text.as_const = &text[i + 1];
            printedText->next = STATE.printedTextHead;
            STATE.printedTextHead = printedText;
            DiscardPrintedTextOnWindowAtXY(printedText->windowId, printedText->x, printedText->y, &printedText->next);
        }
    }
    else
    {
        struct PrintedText *printedText = Alloc(sizeof(*printedText));
        *printedText = (struct PrintedText) {
            .windowId = textPrinter->printerTemplate.windowId,
            .x = textPrinter->printerTemplate.x,
            .y = textPrinter->printerTemplate.y,
            .freeText = FALSE,
            .text.as_const = textPrinter->printerTemplate.currentChar,
            .next = STATE.printedTextHead,
        };
        STATE.printedTextHead = printedText;
    }
}

// TODO: Rather than this we could probably get away with hooking fade-
// ins?
void TestRunner_Overworld_BeforeWarp(const struct WarpData *)
{
    if (!STATE.didInitialWarp)
        STATE.didInitialWarp = TRUE;
    else
        STATE.didWarp = TRUE;
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
 * - DELAY(0) / HOLD_KEYS(_, 0): error.
 * - Movements take a parameter for how many times to repeat? */

#define DELAY(frames) OverworldTest_PushCommand(__LINE__, OP_DELAY, ARG_16, frames, ARG_END)
#define PRESS_KEYS(keys) OverworldTest_PushCommand(__LINE__, OP_PRESS_KEYS, ARG_16, keys, ARG_END)
#define HOLD_KEYS(keys, frames) OverworldTest_PushCommand(__LINE__, OP_HOLD_KEYS, ARG_16, keys, ARG_8, frames, ARG_END)
#define WAIT_FADE_IN OverworldTest_PushCommand(__LINE__, OP_WAIT_FADE_IN, ARG_END)

// https://github.com/gcc-mirror/gcc/blob/master/gcc/typeclass.h
// pointer_type_class == 5
#define MAYBE_GF_ENCODE(maybeString) __builtin_choose_expr(__builtin_constant_p(maybeString) && __builtin_classify_type(maybeString) == 5, (_cs(maybeString)), (maybeString))

#define SELECT(selector) OverworldTest_PushCommand(__LINE__, OP_MENU_SELECT, ARG_32, MAYBE_GF_ENCODE(selector), ARG_END)

#define QUANTITY(n) OverworldTest_PushCommand(__LINE__, OP_MENU_QUANTITY, ARG_16, n, ARG_END)

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

// test_test_runner.c, check that walk down only fires once.
//OVERWORLD_TEST("OVERWORLD")
//{
//    GIVEN {
//        ON_MAP(MAP_PETALBURG_CITY_MART, 4, 7);
//    } WHEN {
//        WALK_DOWN;
//    } THEN {
//        // TODO: Check that the player is where we expect.
//    }
//}

// test_test_runner.c, check that all SELECT forms work.
//OVERWORLD_TEST("OVERWORLD")
//{
//    static const u8 sText_Wobbuffet[] = _("Wobbuffet");
//    GIVEN {
//        ON_MAP(MAP_OLDALE_TOWN, -1, -1);
//        CreateMon(&gPlayerParty[0], SPECIES_WOBBUFFET, 100, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
//        CreateMon(&gPlayerParty[1], SPECIES_WOBBUFFET, 100, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
//        FlagSet(FLAG_SYS_POKEMON_GET);
//    } WHEN {
//        START_MENU {
//            SELECT("POKéMON");
//
//            SELECT("Wobbuffet");
//            PRESS_KEYS(B_BUTTON);
//
//            SELECT(0);
//            PRESS_KEYS(B_BUTTON);
//
//            SELECT(sText_Wobbuffet);
//            PRESS_KEYS(B_BUTTON);
//
//            SELECT(GetSpeciesName(SPECIES_WOBBUFFET));
//            PRESS_KEYS(B_BUTTON);
//
//            // TODO: Be able to select among duplicates. Compile-time
//            // error to do 'SELECT(integer, _)'.
//            //SELECT("Wobbuffet", 1);
//            //DELAY(30);
//            //PRESS_KEYS(B_BUTTON);
//        }
//    }
//}

OVERWORLD_TEST("OVERWORLD")
{
    GIVEN {
        ON_MAP(MAP_PETALBURG_CITY_MART, -1, -1);
        SetMoney(&gSaveBlock1Ptr->money, 3000);
        CreateMon(&gPlayerParty[0], SPECIES_WOBBUFFET, 100, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        FlagSet(FLAG_SYS_POKEMON_GET);
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
            PRESS_KEYS(A_BUTTON);
            SELECT("CLOSE BAG");
        }
        WALK_DOWN;
        WALK_DOWN;
        WALK_DOWN;
        WALK_DOWN;
        WALK_DOWN;

        WALK_DOWN;
        WALK_DOWN;
        WALK_DOWN;
        WALK_DOWN;
        WALK_LEFT;
        WALK_LEFT;
        WALK_LEFT;
        WALK_LEFT;
        WALK_LEFT;
        WALK_UP;

        WALK_UP;
        WALK_UP;
        WALK_UP;
        WALK_UP;
        INTERACT {
            SELECT("YES");
        }
        //START_MENU {
        //    SELECT("POKéMON");
        //    SELECT("Wobbuffet");
        //    SELECT("SUMMARY");
        //    WAIT_FADE_IN;
        //    PRESS_KEYS(B_BUTTON);
        //    SELECT("CANCEL");
        //    SELECT("CANCEL");
        //}
    }
}
