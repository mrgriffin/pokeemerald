#ifndef UI_H
#define UI_H

// TODO: Compressed palettes/tiles.
// TODO: Prevent AT from moving backwards.

/* BG_PALETTE_LAYOUT(PALETTE, AT, UNUSED)
 * PALETTE(id, size)
 * AT(offset)
 * UNUSED(count)
 *
 * MK_BG_PALETTE_LAYOUT */

// TODO: Support some way of aliasing windows.
/* BG_VRAM_LAYOUT(TILES, WINDOW, BACKGROUND, AT, UNUSED)
 * TILES(id, size)
 * WINDOW(id, bg, ...)
 * BACKGROUND(id, n, ...)
 * AT(offset)
 * UNUSED(count)
 *
 * MK_BG_VRAM_LAYOUT */

#define BG_CHAR(n) ((n) * 512)
#define BG_SCREEN(n) ((n) * 64)

#define BG_TEMPLATE_INITIALIZER(id) \
    { \
        .bg = id, \
        .charBaseIndex = CAT(id, _CHAR_BASE_INDEX), \
        .mapBaseIndex = CAT(id, _SCREEN), \
        .screenSize = CAT(id, _SCREEN_SIZE), \
        .paletteMode = CAT(id, _PALETTE_MODE), \
        .priority = CAT(id, _PRIORITY), \
        .baseTile = CAT(id, _BASE_TILE), \
    }

#define WINDOW_TEMPLATE_INITIALIZER(id) \
    { \
        .bg = CAT(id, _BG_NUM), \
        .tilemapLeft = CAT(id, _TILEMAP_LEFT), \
        .tilemapTop = CAT(id, _TILEMAP_TOP), \
        .width = CAT(id, _WIDTH), \
        .height = CAT(id, _HEIGHT), \
        .paletteNum = CAT(id, _PALETTE_NUM), \
        .baseBlock = CAT(id, _OFFSET) - CAT(id, _BG_OFFSET), \
    }

///

#define MK_BPL_ENUM_PALETTE(id, size) \
    CAT(id, _SIZE) = (size), \
    CAT(id, _COUNT) = (CAT(id, _SIZE) + PLTT_SIZE_4BPP - 1) / PLTT_SIZE_4BPP,
#define MK_BPL_ENUM_AT(offset)
#define MK_BPL_ENUM_UNUSED(count)

#define MK_BPL_SIZE_PALETTE(id, size) \
    id, \
    CAT(_bg_pal_next_, id) = id + CAT(id, _COUNT) - 1,
#define MK_BPL_SIZE_AT(offset) \
    CAT(_bg_pal_at_, __COUNTER__) = (offset) - 1,
#define MK_BPL_UNUSED(count) \
    MK_BPL_UNUSED_(__COUNTER__, count)
#define MK_BPL_UNUSED_(id, count) \
    CAT(_bg_pal_unused_, id), \
    CAT(_bg_pal_unused_next_, id) = CAT(_bg_vram_unused_, id) + count - 1,

#define MK_BG_PALETTE_LAYOUT(DEFN) \
    enum \
    { \
        DEFN(MK_BPL_ENUM_PALETTE, MK_BPL_ENUM_AT, MK_BPL_ENUM_UNUSED) \
    }; \
    enum \
    { \
        DEFN(MK_BPL_SIZE_PALETTE, MK_BPL_SIZE_AT, MK_BPL_SIZE_UNUSED) \
        _bg_pal_size \
    }; \
    STATIC_ASSERT(_bg_pal_size <= 16, BgPaletteFreeSpace)

///

#define MK_BVL_ENUM1_TILES(id, size)
#define MK_BVL_ENUM1_WINDOW(id, bg, ...)
#define MK_BVL_ENUM1_BACKGROUND(id, n, ...) \
    id = (n), \
    PREFIX_WITH(CAT(id, _), __VA_ARGS__), \
    CAT(id, _OFFSET) = BG_CHAR(CAT(id, _CHAR_BASE_INDEX)) + CAT(id, _BASE_TILE),
#define MK_BVL_ENUM1_AT(offset)
#define MK_BVL_ENUM1_UNUSED(count)

#define MK_BVL_ENUM2_TILES(id, size) \
    CAT(id, _SIZE) = (size), \
    CAT(id, _COUNT) = ((size) + TILE_SIZE_4BPP - 1) / TILE_SIZE_4BPP,
#define MK_BVL_ENUM2_WINDOW(id, bg, ...) \
    CAT(id, _BG_NUM) = bg, \
    CAT(id, _BG_OFFSET) = CAT(bg, _OFFSET), \
    PREFIX_WITH(CAT(id, _), __VA_ARGS__),
#define MK_BVL_ENUM2_BACKGROUND(id, n, ...)
#define MK_BVL_ENUM2_AT(offset)
#define MK_BVL_ENUM2_UNUSED(count)

#define MK_BVL_SIZE_TILES(id, size) \
    CAT(id, _OFFSET), \
    CAT(_bg_vram_tiles_next_, id) = CAT(id, _OFFSET) + CAT(id, _COUNT) - 1,
#define MK_BVL_SIZE_WINDOW(id, bg, ...) \
    CAT(id, _OFFSET), \
    CAT(_bg_vram_window_next_, id) = CAT(id, _OFFSET) + CAT(id, _WIDTH) * CAT(id, _HEIGHT) - 1,
#define MK_BVL_SIZE_BACKGROUND(id, n, ...) \
    CAT(_bg_vram_bg_prev_, id), \
    CAT(_bg_vram_bg_offset_, id) = (CAT(_bg_vram_bg_prev_, id) + 63) / 64 * 64, \
    CAT(id, _SCREEN) = CAT(_bg_vram_bg_offset_, id) / 64, \
    CAT(_bg_vram_bg_next_, id) = CAT(_bg_vram_bg_offset_, id) + 64 * (CAT(id, _SCREEN_SIZE) + 1) - 1,
#define MK_BVL_SIZE_AT(offset) \
    CAT(_bg_vram_at_, __COUNTER__) = (offset) - 1,
#define MK_BVL_SIZE_UNUSED(count) \
    MK_BVL_SIZE_UNUSED_(__COUNTER__, count)
#define MK_BVL_SIZE_UNUSED_(id, count) \
    CAT(_bg_vram_unused_, id), \
    CAT(_bg_vram_unused_next_, id) = CAT(_bg_vram_unused_, id) + count - 1,

#define MK_BG_VRAM_LAYOUT(DEFN) \
    enum \
    { \
        DEFN(MK_BVL_ENUM1_TILES, MK_BVL_ENUM1_WINDOW, MK_BVL_ENUM1_BACKGROUND, MK_BVL_ENUM1_AT, MK_BVL_ENUM1_UNUSED) \
    }; \
    enum \
    { \
        DEFN(MK_BVL_ENUM2_TILES, MK_BVL_ENUM2_WINDOW, MK_BVL_ENUM2_BACKGROUND, MK_BVL_ENUM2_AT, MK_BVL_ENUM2_UNUSED) \
    }; \
    enum \
    { \
        DEFN(MK_BVL_SIZE_TILES, MK_BVL_SIZE_WINDOW, MK_BVL_SIZE_BACKGROUND, MK_BVL_SIZE_AT, MK_BVL_SIZE_UNUSED) \
        _bg_vram_size \
    }; \
    STATIC_ASSERT(_bg_vram_size <= 2048, BgVramFreeSpace)

#define PREFIX_WITH(prefix, ...) CAT(PREFIX_WITH, ARGCOUNT(__VA_ARGS__))(prefix, __VA_ARGS__)
#define PREFIX_WITH1(prefix, _1) CAT(prefix, _1)
#define PREFIX_WITH2(prefix, _1, _2) CAT(prefix, _1), CAT(prefix, _2)
#define PREFIX_WITH3(prefix, _1, _2, _3) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3)
#define PREFIX_WITH4(prefix, _1, _2, _3, _4) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4)
#define PREFIX_WITH5(prefix, _1, _2, _3, _4, _5) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5)
#define PREFIX_WITH6(prefix, _1, _2, _3, _4, _5, _6) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5), CAT(prefix, _6)
#define PREFIX_WITH7(prefix, _1, _2, _3, _4, _5, _6, _7) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5), CAT(prefix, _6), CAT(prefix, _7)
#define PREFIX_WITH8(prefix, _1, _2, _3, _4, _5, _6, _7, _8) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5), CAT(prefix, _6), CAT(prefix, _7), CAT(prefix, _8)
#define PREFIX_WITH9(prefix, _1, _2, _3, _4, _5, _6, _7, _8, _9) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5), CAT(prefix, _6), CAT(prefix, _7), CAT(prefix, _8), CAT(prefix, _9)
#define PREFIX_WITH10(prefix, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) CAT(prefix, _1), CAT(prefix, _2), CAT(prefix, _3), CAT(prefix, _4), CAT(prefix, _5), CAT(prefix, _6), CAT(prefix, _7), CAT(prefix, _8), CAT(prefix, _9), CAT(prefix, _10)

#endif
