/* Simple party select screen, similar to ChoosePartyMon.
 *
 * 1. Loads the graphics in `graphics/example_screen`.
 * 2. Calls InitScreen.
 * 3. Fades in.
 * 4. Calls UpdateScreen once each frame until it returns FALSE.
 * 5. Fades out into the overworld.
 *
 * USAGE:
 *     setvar VAR_0x8004, 0 @ Start on the first party member.
 *     callnative ShowExampleScreen
 *     compare VAR_0x8004, 0xFF
 *     goto_if_eq EventScript_NoPokemon
 *     @ VAR_0x8004 contains the chosen Pokémon. */
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "overworld.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define STD_FRAME_PALETTE 0xC
#define DLG_FRAME_PALETTE 0xD
#define TEXT_PALETTE 0xE
#define HEADER_PALETTE 0xF

#define STD_FRAME_TILE 0x001
#define DLG_FRAME_TILE 0x00A
#define WINDOW_TILE 0x018

/* Note that the rest of pokeemerald manipulates tilemap entries as u16. */
struct __attribute__((packed, aligned(2))) TilemapEntry
{
    u16 tile:10,
        hFlip:1,
        vFlip:1,
        palette:4;
};

/* Note that the rest of pokeemerald uses 0, 1, and 7 directly. */
#define FONT_SMALL 0
#define FONT_NORMAL 1
#define FONT_NARROW 7

/* Note that the rest of pokeemerald computes the size and offsets directly. */
#define IMG_FRAME(gfx, width, height, frame) { .data = ((gfx) + (frame)*IMG_SIZE(width, height)), .size = IMG_SIZE(width, height) }
#define IMG_SIZE(width, height) ((width)*(height)/64*TILE_SIZE_4BPP)

/*
 * Boilerplate code above. You probably don't need to edit it, except to
 * add #includes for other functions you want to call.
 */

// NOTE: 192 colors maximum.
// 0xC0-0xFF are overwritten by the window and header palettes.
static const u16 sScreen_BackgroundPalette[] = INCBIN_U16("graphics/example_screen/tiles.gbapal");

// NOTE: 512 tiles maximum.
static const u32 sScreen_BackgroundTiles[] = INCBIN_U32("graphics/example_screen/tiles.4bpp.lz");

static const u32 sScreen_Background2Tilemap[] = INCBIN_U32("graphics/example_screen/top_tilemap.bin.lz");
static const u32 sScreen_Background3Tilemap[] = INCBIN_U32("graphics/example_screen/bottom_tilemap.bin.lz");

/* Each unique sprite palette needs a unique tag. If two sprites share a
 * palette, they should use the same tag. Up to 16 sprite palettes can
 * be used at the same time. */
enum
{
    PALTAG_CURSOR,
};

static const struct SpritePalette sSpritePalette_Cursor =
{
    .data = (u16 []) INCBIN_U16("graphics/example_screen/cursor.gbapal"),
    .tag = PALTAG_CURSOR,
};

/* A 32x32 cursor sprite.
 * OamData describes the size and shape of the sprite.
 * Anims describes the animations. The first animation automatically plays.
 * Graphics contains the actual sprite.
 * Template connects these parts together. */

static const struct OamData sOamData_Cursor =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = 0,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const union AnimCmd *const sAnims_Cursor[] =
{
    (union AnimCmd [])
    {
        ANIMCMD_FRAME(0, 8),
        ANIMCMD_FRAME(1, 8),
        ANIMCMD_JUMP(0),
    },
};

static const u8 sGraphics_Cursor[] = INCBIN_U8("graphics/example_screen/cursor.4bpp");

static const struct SpriteTemplate sSpriteTemplate_Cursor =
{
    .tileTag = 0xFFFF,
    .paletteTag = PALTAG_CURSOR,
    .oam = &sOamData_Cursor,
    .anims = sAnims_Cursor,
    .images = (struct SpriteFrameImage [])
    {
        /* Note width and height must match oam. */
        IMG_FRAME(sGraphics_Cursor, 32, 32, 0),
        IMG_FRAME(sGraphics_Cursor, 32, 32, 1),
    },
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

/* Windows are areas of the screen where text can be displayed. If two
 * windows share a background then they must not overlap. Windows will
 * also overwrite any tiles in the tilemap of the background they are
 * on, which is why the tilemaps are loaded to background 2 & 3, and the
 * windows are on background 0 & 1.
 * We have two:
 * - One which contains the header with information about controls.
 * - One which contains the nickname of the Pokémon under the cursor. */
enum
{
    WINDOW_HEADER,
    WINDOW_MESSAGE,
    WINDOW_YES_NO,
    WINDOW_NICKNAME,
    WINDOW_DATA,
};

static const struct WindowTemplate sScreen_WindowTemplates[] =
{
    [WINDOW_HEADER] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = HEADER_PALETTE,
    },
    [WINDOW_MESSAGE] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 27,
        .height = 4,
        .paletteNum = TEXT_PALETTE,
    },
    [WINDOW_YES_NO] =
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 9,
        .width = 5,
        .height = 4,
        .paletteNum = TEXT_PALETTE,
    },
    [WINDOW_NICKNAME] =
    {
        .bg = 1,
        .tilemapLeft = 4,
        .tilemapTop = 5,
        .width = 14,
        .height = 2,
        .paletteNum = TEXT_PALETTE,
    },
    [WINDOW_DATA] =
    {
        .bg = 1,
        .tilemapLeft = 4,
        .tilemapTop = 7,
        .width = 14,
        .height = 2,
        .paletteNum = TEXT_PALETTE,
    },
};

static const u8 sHeaderColors[3] = { 0x0, 0x1, 0x2 };
static const u8 sWindowColors[3] = { TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY };
static const u8 sNicknameColors[3] = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY };

static const u8 sText_Header[] = _("{DPAD_NONE}MOVE {A_BUTTON}OK {B_BUTTON}CANCEL");
static const u8 sText_Nickname1[] = _("NICKNAME");
static const u8 sText_Nickname2[] = _("{STR_VAR_1}{STR_VAR_2}");
static const u8 sText_Genderless[] = _("");
static const u8 sText_Level[] = _("{LV}{STR_VAR_1}");
static const u8 sText_AreYouSureYouWantToCancel[] = _("Are you sure you\nwant to cancel?");

enum
{
    STATE_HANDLE_INPUT,
    STATE_ANIMATE_CURSOR,
    STATE_CONFIRM_CANCEL_MESSAGE,
    STATE_CONFIRM_CANCEL_YES_NO,
};

#define CURSOR_COLUMNS 3
#define CURSOR_ROWS 2

struct Screen
{
    struct TilemapEntry topTilemap[32][32];
    struct TilemapEntry bottomTilemap[32][32];
    /* Variables that need to exist between frames go here. */
    u8 state;
    u8 cursorColumn;
    u8 cursorRow;
    u8 cursorSpriteId;
};

static EWRAM_DATA struct Screen *sScreen = NULL;

static u32 GetPartyIndex(void)
{
    return sScreen->cursorRow * CURSOR_COLUMNS + sScreen->cursorColumn;
}

static struct Coords16 GetCursorScreenCoords(void)
{
    return (struct Coords16)
    {
        .x = 48 + 40 * sScreen->cursorColumn,
        .y = 68 + 24 * sScreen->cursorRow,
    };
}

static void Task_ScrollBackground(u8 taskId)
{
    /* Every frame scroll 0.25 pixels up and left. Note that backgrounds
     * automatically wrap around, so this scrolls infinitely. */
    ChangeBgX(3, Q_8_8(-0.25), 2);
    ChangeBgY(3, Q_8_8(-0.25), 2);

    /* To stop scrolling we could call DestroyTask(taskId); */
}

static void UpdateCursor(void)
{
    u32 x;
    struct Pokemon *mon;

    mon = &gPlayerParty[GetPartyIndex()];

    FillWindowPixelBuffer(WINDOW_NICKNAME, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));
    FillWindowPixelBuffer(WINDOW_DATA, PIXEL_FILL(TEXT_COLOR_TRANSPARENT));

    /* Draw the NICKNAME: part of the text. */
    StringExpandPlaceholders(gStringVar4, sText_Nickname1);
    AddTextPrinterParameterized3(WINDOW_NICKNAME, FONT_NARROW, 0, 1, sNicknameColors, -1, gStringVar4);

    /* Draw the actual nickname right-aligned.
     * 14*8 is the width of the window in pixels. */
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    switch (GetMonGender(mon))
    {
    case MON_MALE:
        StringCopy(gStringVar2, gText_MaleSymbol);
        break;
    case MON_FEMALE:
        StringCopy(gStringVar2, gText_FemaleSymbol);
        break;
    case MON_GENDERLESS:
        StringCopy(gStringVar2, sText_Genderless);
        break;
    }
    StringExpandPlaceholders(gStringVar4, sText_Nickname2);
    x = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 14*8);
    AddTextPrinterParameterized3(WINDOW_NICKNAME, FONT_NARROW, x, 1, sNicknameColors, -1, gStringVar4);

    /* Buffer the level, and draw it right-aligned. */
    ConvertIntToDecimalStringN(gStringVar1, GetMonData(mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, sText_Level);
    x = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 14*8);
    AddTextPrinterParameterized3(WINDOW_DATA, 7, x, 1, sNicknameColors, -1, gStringVar4);

    PutWindowTilemap(WINDOW_NICKNAME);
    CopyWindowToVram(WINDOW_NICKNAME, 2);
    PutWindowTilemap(WINDOW_DATA);
    CopyWindowToVram(WINDOW_DATA, 2);
    ScheduleBgCopyTilemapToVram(1);
}

/* Called once before fading in. */
static bool32 InitScreen(void)
{
    int i;
    u32 x, y;
    struct Coords16 xy;

    /* Start a task that scrolls the background.
     * UpdateScreen is only called when the screen is not fading, but
     * the background needs to scroll during the fade. */
    CreateTask(Task_ScrollBackground, 1);

    /* Draw the header.
     * 1. Clears the window to the background color.
     * 2. Writes the text on the window.
     * 3. Puts the window tilemap into a buffer (pt 1).
     * 4. Puts the window tilemap into a buffer (pt 2).
     * 5. Schedules the background to be copied to VRAM. */
    FillWindowPixelBuffer(WINDOW_HEADER, PIXEL_FILL(0xF));
    AddTextPrinterParameterized3(WINDOW_HEADER, FONT_SMALL, 2, 1, sHeaderColors, -1, sText_Header);
    PutWindowTilemap(WINDOW_HEADER);
    CopyWindowToVram(WINDOW_HEADER, 2);
    ScheduleBgCopyTilemapToVram(0);

    /* Load the sprite palettes. */
    LoadSpritePalette(&sSpritePalette_Cursor);
    LoadMonIconPalettes();

    /* Create sprites for the party. */
    for (i = 0; i < gPlayerPartyCount; i++)
    {
        x = 48 + 40 * (i % 3);
        y = 80 + 24 * (i / 3);

        CreateMonIcon(GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL), SpriteCallbackDummy, x, y, 0, GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY, NULL), TRUE);

	/* Updates the top tilemap to put a shadow under the party member. */
        sScreen->topTilemap[y / 8][x / 8 - 1] = (struct TilemapEntry) { .tile = 0x00A, .palette = 0x0 };
        sScreen->topTilemap[y / 8][x / 8] = (struct TilemapEntry) { .tile = 0x00A, .hFlip = TRUE, .palette = 0x0 };
        sScreen->topTilemap[y / 8 + 1][x / 8 - 1] = (struct TilemapEntry) { .tile = 0x00B, .palette = 0x0 };
        sScreen->topTilemap[y / 8 + 1][x / 8] = (struct TilemapEntry) { .tile = 0x00B, .hFlip = TRUE, .palette = 0x0 };
        ScheduleBgCopyTilemapToVram(2);
    }

    /* Initialize the logical cursor.
     * Reads VAR_0x8004 to decide which party member to place the cursor
     * on. */
    if (gSpecialVar_0x8004 >= gPlayerPartyCount)
    {
        sScreen->cursorColumn = 0;
        sScreen->cursorRow = 0;
    }
    else
    {
        sScreen->cursorColumn = gSpecialVar_0x8004 % CURSOR_COLUMNS;
        sScreen->cursorRow = gSpecialVar_0x8004 / CURSOR_COLUMNS;
    }
    UpdateCursor();

    /* Initialize the cursor sprite. */
    xy = GetCursorScreenCoords();
    sScreen->cursorSpriteId = CreateSprite(&sSpriteTemplate_Cursor, xy.x, xy.y, 0);

    /* Plays the Pokémon Center music.
     * The OW music will be restored at the end because ShowExampleScreen
     * uses CB2_ReturnToFieldContinueScriptPlayMapMusic. */
    FadeOutAndPlayNewMapMusic(MUS_POKE_CENTER, 2);

    sScreen->state = STATE_HANDLE_INPUT;

    return TRUE;
}

/* Called each frame after the screen has faded in, until it returns
 * FALSE, which causes the screen to fade out and exit. */
static bool32 UpdateScreen(void)
{
    switch (sScreen->state)
    {
    /* Handle button presses. */
    case STATE_HANDLE_INPUT:
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            /* Set VAR_0x8004 to the chosen party member. */
            gSpecialVar_0x8004 = GetPartyIndex();
            return FALSE;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            DrawDialogFrameWithCustomTileAndPalette(WINDOW_MESSAGE, TRUE, DLG_FRAME_TILE, DLG_FRAME_PALETTE);
            gTextFlags.canABSpeedUpPrint = 1;
            AddTextPrinterParameterized2(WINDOW_MESSAGE, FONT_NORMAL, sText_AreYouSureYouWantToCancel, GetPlayerTextSpeedDelay(), NULL, sWindowColors[1], sWindowColors[0], sWindowColors[2]);
            sScreen->state = STATE_CONFIRM_CANCEL_MESSAGE;
        }
        else if (JOY_NEW(DPAD_ANY))
        {
            /* Check which direction was held, and whether we can move
             * in that direction. If we can, move the logical cursor,
             * and set updateCursor to TRUE. */
            bool32 updateCursor = FALSE;

            if (JOY_REPEAT(DPAD_LEFT)
             && sScreen->cursorColumn > 0)
            {
                sScreen->cursorColumn--;
                updateCursor = TRUE;
            }

            if (JOY_REPEAT(DPAD_RIGHT)
             && sScreen->cursorColumn < CURSOR_COLUMNS - 1
             && GetPartyIndex() + 1 < gPlayerPartyCount)
            {
                sScreen->cursorColumn++;
                updateCursor = TRUE;
            }

            if (JOY_REPEAT(DPAD_UP)
             && sScreen->cursorRow > 0)
            {
                sScreen->cursorRow--;
                updateCursor = TRUE;
            }

            if (JOY_REPEAT(DPAD_DOWN)
             && sScreen->cursorRow < CURSOR_ROWS - 1
             && GetPartyIndex() + CURSOR_COLUMNS < gPlayerPartyCount)
            {
                sScreen->cursorRow++;
                updateCursor = TRUE;
            }

            /* Play the cursor moving sound effect, update the nickname
             * text, and wait for the cursor sprite to move to its new
             * location. */
            if (updateCursor)
            {
                PlaySE(SE_SELECT);
                UpdateCursor();
                sScreen->state = STATE_ANIMATE_CURSOR;
            }
        }
        break;
    }

    /* Animate the cursor moving to its new location, then return to
     * handling input. */
    case STATE_ANIMATE_CURSOR:
    {
        struct Coords16 xy = GetCursorScreenCoords();
        struct Sprite *cursor = &gSprites[sScreen->cursorSpriteId];

        /* Move the cursor sprite towards its target location. */
        if (cursor->x < xy.x)
            cursor->x += 4;
        else if (cursor->x > xy.x)
            cursor->x -= 4;

        if (cursor->y < xy.y)
            cursor->y += 3;
        else if (cursor->y > xy.y)
            cursor->y -= 3;

        /* The cursor has arrived. Go back to handling input.
         * Note that this only works because the GetCursorScreenCoords()
         * always returns numbers that can be reached in steps of 4
         * pixels horizontally, and 3 pixels vertically. */
        if (cursor->x == xy.x && cursor->y == xy.y)
            sScreen->state = STATE_HANDLE_INPUT;

        break;
    }

    /* Wait for the "are you sure?" message to be displayed. */
    case STATE_CONFIRM_CANCEL_MESSAGE:
    {
        if (!RunTextPrintersRetIsActive(WINDOW_MESSAGE))
        {
            DrawStdFrameWithCustomTileAndPalette(WINDOW_YES_NO, FALSE, STD_FRAME_TILE, STD_FRAME_PALETTE);
            AddTextPrinterParameterized3(WINDOW_YES_NO, FONT_NORMAL, 8, 1, sWindowColors, -1, gText_YesNo);
            InitMenuInUpperLeftCornerPlaySoundWhenAPressed(WINDOW_YES_NO, 2, 0);
            ScheduleBgCopyTilemapToVram(0);
            sScreen->state = STATE_CONFIRM_CANCEL_YES_NO;
        }
        break;
    }

    /* Handle input in the yes/no box. */
    case STATE_CONFIRM_CANCEL_YES_NO:
    {
        switch (Menu_ProcessInputNoWrap())
        {
        case 0: // Yes.
            PlaySE(SE_SELECT);
            /* Hide the message and yes/no windows. */
            ClearDialogWindowAndFrameToTransparent(WINDOW_MESSAGE, FALSE);
            ClearStdWindowAndFrameToTransparent(WINDOW_YES_NO, FALSE);
            ScheduleBgCopyTilemapToVram(0);
            /* Set VAR_0x8004 to 0xFF so the script can detect cancellation. */
            gSpecialVar_0x8004 = 0xFF;
            /* Close the screen. */
            return FALSE;
            break;
        case 1: // No.
        case MENU_B_PRESSED:
            PlaySE(SE_SELECT);
            /* Hide the message and yes/no windows. */
            ClearDialogWindowAndFrameToTransparent(WINDOW_MESSAGE, FALSE);
            ClearStdWindowAndFrameToTransparent(WINDOW_YES_NO, FALSE);
            ScheduleBgCopyTilemapToVram(0);
            /* Return to handling input. */
            sScreen->state = STATE_HANDLE_INPUT;
            break;
        }
    }
    }

    return TRUE;
}

/* ShowExampleScreen is the name of the function which is used with
 * callnative. Renaming this changes what name to use in scripts. */
void ShowExampleScreen(void)
{
    static void CB2_Screen(void);
    SetMainCallback2(CB2_Screen);
    gMain.savedCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
}

/*
 * Boilerplate code below. You probably don't need to edit it.
 */

static void VBlankCB_Screen(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

enum
{
    MAIN_STATE_RESET,
    MAIN_STATE_LOAD,
    MAIN_STATE_WAIT_FADE,
    MAIN_STATE_LOOP,
    MAIN_STATE_EXIT,
};

static const struct BgTemplate sScreen_BackgroundTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
    },
};

static const struct WindowTemplate sDummyWindowTemplates[] = { DUMMY_WIN_TEMPLATE };

static void CB2_Screen(void)
{
    switch (gMain.state)
    {
    case MAIN_STATE_RESET:
        SetVBlankHBlankCallbacksToNull();
        ResetVramOamAndBgCntRegs();
        ResetBgsAndClearDma3BusyFlags(0);
        ClearScheduledBgCopiesToVram();
        ResetAllBgsCoordinates();
        DeactivateAllTextPrinters();
        ScanlineEffect_Stop();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        ResetTasks();
        ScanlineEffect_Stop();
        gMain.state = MAIN_STATE_LOAD;
        break;

    case MAIN_STATE_LOAD:
        if (!(sScreen = AllocZeroed(sizeof(*sScreen))))
            goto error;

        InitBgsFromTemplates(0, sScreen_BackgroundTemplates, ARRAY_COUNT(sScreen_BackgroundTemplates));

        InitWindows(sDummyWindowTemplates);
        {
            int i, baseBlock;
            struct WindowTemplate template;
            baseBlock = WINDOW_TILE;
            for (i = 0; i < ARRAY_COUNT(sScreen_WindowTemplates); ++i)
            {
                template = sScreen_WindowTemplates[i];
                template.baseBlock = baseBlock;
                AddWindow(&template);
                baseBlock += template.width * template.height;
            }
        }

        LZ77UnCompVram(sScreen_BackgroundTiles, (void *)BG_CHAR_ADDR(0));

        SetBgTilemapBuffer(2, sScreen->topTilemap);
        LZ77UnCompVram(sScreen_Background2Tilemap, sScreen->topTilemap);
        ScheduleBgCopyTilemapToVram(2);

        SetBgTilemapBuffer(3, sScreen->bottomTilemap);
        LZ77UnCompVram(sScreen_Background3Tilemap, sScreen->bottomTilemap);
        ScheduleBgCopyTilemapToVram(3);

        LoadPalette(sScreen_BackgroundPalette, 0x00, sizeof(sScreen_BackgroundPalette));
        LoadPalette(GetOverworldTextboxPalettePtr(), TEXT_PALETTE * 0x10, 0x20);
        LoadPalette(GetTextWindowPalette(2), HEADER_PALETTE * 0x10, 0x20);

        LoadUserWindowBorderGfx(0, STD_FRAME_TILE, STD_FRAME_PALETTE * 0x10);
        LoadMessageBoxGfx(0, DLG_FRAME_TILE, DLG_FRAME_PALETTE * 0x10);

        if (!InitScreen())
            goto error;

        SetVBlankCallback(VBlankCB_Screen);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state = MAIN_STATE_WAIT_FADE;
        break;

    case MAIN_STATE_WAIT_FADE:
        RunTasks();
        AnimateSprites();
        BuildOamBuffer();
        DoScheduledBgTilemapCopiesToVram();
        UpdatePaletteFade();
        if (!gPaletteFade.active)
            gMain.state = MAIN_STATE_LOOP;
        break;

    case MAIN_STATE_LOOP:
        RunTasks();
        AnimateSprites();
        BuildOamBuffer();
        DoScheduledBgTilemapCopiesToVram();
        UpdatePaletteFade();
        if (!UpdateScreen())
        {
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gMain.state = MAIN_STATE_EXIT;
        }
        break;

    case MAIN_STATE_EXIT:
        RunTasks();
        AnimateSprites();
        BuildOamBuffer();
        DoScheduledBgTilemapCopiesToVram();
        UpdatePaletteFade();
        if (!gPaletteFade.active)
        {
error:
            TRY_FREE_AND_SET_NULL(sScreen);
            SetMainCallback2(gMain.savedCallback);
        }
        break;
    }
}
