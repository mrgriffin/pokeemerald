#ifndef LAYOUT_H
#define LAYOUT_H

/* TODO: Comment explaining how these work. */

#define LoadPalette_Checked(palette, member) do { \
        STATIC_ASSERT(sizeof(palette) <= SIZEOF_MEMBER(struct PaletteLayout, member), sizeOf ## palette); \
        LoadPalette(palette, offsetof(struct PaletteLayout, member) / sizeof(u16), sizeof(palette)); \
    } while (0)
#define LoadPalette_Unchecked(palette, member) do { \
        LoadPalette(palette, offsetof(struct PaletteLayout, member) / sizeof(u16), SIZEOF_MEMBER(struct PaletteLayout, member)); \
    } while (0)

#define LoadBgTiles_Unchecked(bg, tiles, member) do { \
        LoadBgTiles(bg, tiles, SIZEOF_MEMBER(struct VramLayout, member) * TILE_SIZE_4BPP, offsetof(struct VramLayout, member)); \
    } while (0)

struct Palette { u16 _[16]; };

#define TILES(id, count) u8 id[count];
#define WINDOW(id, width, height) u8 id[height][width]
#define WINDOW_WIDTH(id) sizeof(((struct VramLayout *)NULL)->id[0])
#define WINDOW_HEIGHT(id) (sizeof(((struct VramLayout *)NULL)->id) / sizeof(((struct VramLayout *)NULL)->id[0]))
#define WINDOW_BASE_BLOCK(id) offsetof(struct VramLayout, id)

#define PALETTE_NUM(id) (offsetof(struct PaletteLayout, id) / sizeof(struct Palette))

#endif
