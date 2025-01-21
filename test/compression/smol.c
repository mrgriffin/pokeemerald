#include "global.h"
#include "main.h"
#include "malloc.h"
#include "random.h"
#include "sprite.h"
#include "test/test.h"

TEST("Compression test: tileset smol")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Tileset Smol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            DebugPrintf("different at %x", &compBuffer[i]);
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: tileset fastSmol")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp.fastSmol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Tileset fastSmol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: tileset LZ")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/tilesetTest.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Tileset LZ: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: secondary tileset smol")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Secondary Tileset Smol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: secondary tileset fastSmol")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp.fastSmol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Secondary Tileset fastSmol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: secondary tileset LZ")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/secondary_tileset.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Secondary Tileset LZ: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}
TEST("Compression test: simple battle sprite smol")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/simple_battle_sprite.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/simple_battle_sprite.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Simple Sprite smol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: simple battle sprite LZ")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/simple_battle_sprite.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/simple_battle_sprite.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Simple Sprite LZ: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: complex battle sprite")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/complex_battle_sprite.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/complex_battle_sprite.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Complex Sprite smol: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: complex battle sprite")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/complex_battle_sprite.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/complex_battle_sprite.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Time Complex Sprite LZ: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: Gossifleur")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/gossifleur.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/gossifleur.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Gossifleur: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: Ledian")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/ledian.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/ledian.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Ledian: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: Mr. Mime")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/mr_mime.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/mr_mime.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Ledian: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: bubbles")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_bubbles.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_bubbles.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small Bubbles: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 0")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_0.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 0: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}
TEST("Compression test: medium mode 0")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_0.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 0: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 0")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_0.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 0: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 1")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_1.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 1: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 1")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_1.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 1: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 1")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_1.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 1: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 2")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_2.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 2: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 2")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_2.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 2: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 2")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_2.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 2: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 3")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_3.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 3: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 3")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_3.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 3: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 3")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_3.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 3: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 4")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_4.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 4: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 4")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_4.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 4: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 4")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_4.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 4: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 5")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_5.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 5: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 5")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_5.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 5: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 5")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_5.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 5: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 0 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_0.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 0 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}
TEST("Compression test: medium mode 0 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_0.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 0 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 0 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_0.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_0.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 0 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 1 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_1.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 1 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 1 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_1.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 1 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 1 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_1.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 1 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 2 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_2.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 2 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 2 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_2.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 2 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 2 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_2.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 2 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 3 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_3.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 3 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 3 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_3.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 3 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 3 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_3.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 3 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 4 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_4.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 4 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 4 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_4.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 4 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 4 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_4.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 4 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: small mode 5 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/small_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/small_mode_5.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Small mode 5 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: medium mode 5 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/medium_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/medium_mode_5.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Medium mode 5 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: large mode 5 lz")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/large_mode_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/large_mode_5.4bpp.lz");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    CycleCountStart();
    DecompressDataWithHeaderWram(compFile, compBuffer);
    u32 timeTaken = CycleCountEnd();
    DebugPrintf("Large mode 5 lz: %u", timeTaken);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 63 1")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_63_1.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_63_1.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 62 2")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_62_2.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_62_2.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 61 3")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_61_3.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_61_3.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 60 4")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_60_4.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_60_4.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 59 5")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_59_5.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_59_5.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 58 6")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_58_6.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_58_6.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 57 7")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_57_7.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_57_7.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 56 8")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_56_8.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_56_8.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 55 9")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_55_9.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_55_9.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 54 10")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_54_10.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_54_10.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 53 11")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_53_11.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_53_11.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 52 12")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_52_12.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_52_12.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 51 13")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_51_13.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_51_13.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 50 14")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_50_14.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_50_14.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 49 15")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_49_15.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_49_15.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 48 16")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_48_16.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_48_16.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 47 17")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_47_17.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_47_17.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 46 18")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_46_18.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_46_18.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 45 19")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_45_19.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_45_19.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 44 20")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_44_20.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_44_20.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 43 21")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_43_21.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_43_21.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 42 22")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_42_22.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_42_22.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 41 23")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_41_23.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_41_23.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 40 24")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_40_24.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_40_24.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 39 25")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_39_25.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_39_25.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 38 26")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_38_26.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_38_26.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 37 27")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_37_27.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_37_27.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 36 28")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_36_28.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_36_28.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 35 29")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_35_29.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_35_29.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 34 30")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_34_30.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_34_30.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 33 31")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_33_31.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_33_31.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}

TEST("Compression test: table generation 32 32")
{
    static const u32 origFile[] = INCBIN_U32("test/compression/table_32_32.4bpp");
    static const u32 compFile[] = INCBIN_U32("test/compression/table_32_32.4bpp.smol");
    u32 imageSize = GetDecompressedDataSize(compFile);
    u32 *compBuffer = Alloc(imageSize);
    DecompressDataWithHeaderWram(compFile, compBuffer);

    u32 val1 = 0;
    u32 val2 = 0;
    bool32 areEqual = TRUE;
    for (u32 i = 0; i < imageSize/4; i++)
    {
        val1 = origFile[i];
        val2 = compBuffer[i];
        if (val1 != val2)
        {
            areEqual = FALSE;
            break;
        }
    }

    EXPECT_EQ(areEqual, TRUE);

    Free(compBuffer);
}
