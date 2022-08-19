#ifndef LAYOUT_H
#define LAYOUT_H

/* TODO: Comment explaining how these work. */

#define SIZEOF_MEMBER(type, member) sizeof(((type *)NULL)->member)

#define NOOP(...)

#define MK_PALETTE_LAYOUT_STRUCT__STATIC_PALETTE(id, data, count) u8 id[(count)];
#define MK_PALETTE_LAYOUT_STRUCT__DYNAMIC_PALETTE(id, count) u8 id[(count)];
#define MK_PALETTE_LAYOUT_STRUCT__UNUSED(count) u8 CAT(unused_, __COUNTER__)[(count)];
#define MK_PALETTE_LAYOUT_STRUCT \
    struct PaletteLayout \
    { \
        PALETTE_LAYOUT(MK_PALETTE_LAYOUT_STRUCT__STATIC_PALETTE, MK_PALETTE_LAYOUT_STRUCT__DYNAMIC_PALETTE, MK_PALETTE_LAYOUT_STRUCT__UNUSED) \
    }; \
    STATIC_ASSERT(sizeof(struct PaletteLayout) <= 16, PaletteLayoutSize)

#define MK_PALETTE_ENUMS__STATIC_PALETTE(id, data, count) \
    CAT(id, _NUM) = offsetof(struct PaletteLayout, id), \
    CAT(id, _COUNT) = (count),
#define MK_PALETTE_ENUMS__DYNAMIC_PALETTE(id, count) \
    CAT(id, _NUM) = offsetof(struct PaletteLayout, id), \
    CAT(id, _COUNT) = (count),
#define MK_PALETTE_ENUMS \
    enum \
    { \
        PALETTE_LAYOUT(MK_PALETTE_ENUMS__STATIC_PALETTE, MK_PALETTE_ENUMS__DYNAMIC_PALETTE, NOOP) \
    }

// TODO: Define for 32 (PALETTE_SIZE_4BPP?).
#define LoadPalette_Checked(palette, id) do { \
        STATIC_ASSERT(sizeof(palette) <= CAT(id, _COUNT) * 32, palette ## _Overflows_ ## id); \
        LoadPalette(palette, CAT(id, _NUM) * 16, sizeof(palette)); \
    } while (0)

#define LoadPalette_Unchecked(palette, id) do { \
        LoadPalette(palette, CAT(id, _NUM) * 16, CAT(id, _COUNT) * 32); \
    } while (0)

/*****/

// TODO: How do we handle a group of windows?
#define MK_TILE_LAYOUT_STRUCT__TILES(id, count) u8 id[(count)];
#define MK_TILE_LAYOUT_STRUCT__WINDOW(id, width, height) u8 id[(width) * (height)];
#define MK_TILE_LAYOUT_STRUCT__UNUSED(count) u8 CAT(unused_, __COUNTER__)[(count)];
// XXX: Misnomer, this is the layout of one/two charblocks.
#define MK_TILE_LAYOUT_STRUCT \
    struct TileLayout \
    { \
        TILE_LAYOUT(MK_TILE_LAYOUT_STRUCT__TILES, MK_TILE_LAYOUT_STRUCT__WINDOW, MK_TILE_LAYOUT_STRUCT__UNUSED) \
    }; \
    STATIC_ASSERT(sizeof(struct TileLayout) <= 1024, TileLayoutSize)

#define MK_TILE_WINDOW_ENUMS__TILES(id, count) \
    CAT(id, _BASE_BLOCK) = offsetof(struct TileLayout, id), \
    CAT(id, _COUNT) = (count),
#define MK_TILE_WINDOW_ENUMS__WINDOW(id, width, height) \
    CAT(id, _WIDTH) = (width), \
    CAT(id, _HEIGHT) = (height), \
    CAT(id, _BASE_BLOCK) = offsetof(struct TileLayout, id), \
    CAT(id, _COUNT) = ((width) * (height)),
#define MK_TILE_WINDOW_ENUMS \
    enum \
    { \
        TILE_LAYOUT(MK_TILE_WINDOW_ENUMS__TILES, MK_TILE_WINDOW_ENUMS__WINDOW, NOOP) \
    };

#define LoadBgTiles_Unchecked(bg, tiles, id) do { \
        LoadBgTiles(bg, tiles, CAT(id, _COUNT) * TILE_SIZE_4BPP, CAT(id, _BASE_BLOCK)); \
    } while (0)

#endif
