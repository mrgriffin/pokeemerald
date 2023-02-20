#include "global.h"
#include "random.h"

EWRAM_DATA static u8 sUnknown = 0;
EWRAM_DATA static u32 sRandCount = 0;

// IWRAM common
u32 gRngValue;
u32 gRng2Value;

u16 Random(void)
{
    gRngValue = ISO_RANDOMIZE1(gRngValue);
    sRandCount++;
    return gRngValue >> 16;
}

void SeedRng(u16 seed)
{
    gRngValue = seed;
    sUnknown = 0;
}

void SeedRng2(u16 seed)
{
    gRng2Value = seed;
}

u16 Random2(void)
{
    gRng2Value = ISO_RANDOMIZE1(gRng2Value);
    return gRng2Value >> 16;
}

__attribute__((weak, alias("RandomUniformDefault")))
u32 RandomUniform(enum RandomTag tag, u32 n);

__attribute__((weak, alias("RandomWeightedArrayDefault")))
u32 RandomWeightedArray(enum RandomTag tag, u32 n, const u8 *weights);

u32 RandomUniformDefault(enum RandomTag tag, u32 n)
{
    return (n * Random()) >> 16;
}

u32 RandomWeightedArrayDefault(enum RandomTag tag, u32 n, const u8 *weights)
{
    s32 i, sum, targetSum;

    sum = 0;
    for (i = 0; i < n; i++)
        sum += weights[i];

    targetSum = (sum * Random()) >> 16;
    for (i = 0; i < n - 1; i++)
    {
        targetSum -= weights[i];
        if (targetSum < 0)
            return i;
    }
    return n - 1;
}
