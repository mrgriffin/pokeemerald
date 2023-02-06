#include "global.h"
#include "stackful_coroutine.h"
#include "task.h"
#include "test.h"

void CoroutineOOM(void)
{
    Test_ExitWithResult(TEST_RESULT_FAIL, "OOM");
}

static EWRAM_DATA u32 sState = 0;

static void SetStateTo1(void)
{
    sState = 1;
}

TEST("CreateCoroutine converts regular functions into coroutines")
{
    struct Coroutine *co;
    sState = 0;
    co = CreateCoroutine(SetStateTo1);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    EXPECT_EQ(sState, 1);
}

static void SetStateTo(u32 to)
{
    sState = to;
}

TEST("CreateCoroutine passes its argument to the function")
{
    struct Coroutine *co;
    sState = 0;
    co = CreateCoroutine(SetStateTo, 2);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    EXPECT_EQ(sState, 2);
}

static void IncrementStateTwice(void)
{
    sState++;
    SuspendCoroutine();
    sState++;
}

TEST("SuspendCoroutine returns control to ResumeCoroutine and vice-versa")
{
    struct Coroutine *co;
    sState = 0;
    co = CreateCoroutine(IncrementStateTwice);
    co = ResumeCoroutine(co);
    EXPECT_NE(co, NULL);
    EXPECT_EQ(sState, 1);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    EXPECT_EQ(sState, 2);
}

static void SuspendSetStateTo(u32 to)
{
    SuspendCoroutine();
    sState = to;
}

TEST("Arguments are preserved through context switches")
{
    struct Coroutine *co;
    sState = 0;
    co = CreateCoroutine(SuspendSetStateTo, 2);
    co = ResumeCoroutine(co);
    EXPECT_NE(co, NULL);
    EXPECT_EQ(sState, 0);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    EXPECT_EQ(sState, 2);
}

static void AllocateOnStackSuspend(u32 base)
{
    u32 values[8];
    u32 i;
    for (i = 0; i < 8; i++)
        values[i] = base + i;
    SuspendCoroutine();
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], base + i);
}

TEST("Local variables are preserved through context switches")
{
    u32 values[8];
    u32 i;
    struct Coroutine *co;
    for (i = 0; i < 8; i++)
        values[i] = i;
    co = CreateCoroutine(AllocateOnStackSuspend, 8);
    co = ResumeCoroutine(co);
    EXPECT_NE(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
}

static struct Coroutine *ResumeArgument(struct Coroutine *co)
{
    u32 values[8];
    u32 i;
    for (i = 0; i < 8; i++)
        values[i] = 16 + i;
    co = ResumeCoroutine(co);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], 16 + i);
    return co;
}

static void AllocateOnStackSuspendTwice(u32 base)
{
    u32 values[8];
    u32 i;
    for (i = 0; i < 8; i++)
        values[i] = base + i;
    SuspendCoroutine();
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], base + i);
    SuspendCoroutine();
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], base + i);
}

// NOTE: Although the library can support calling in different contexts,
// users should avoid doing so wherever possible because the performance
// can be poor, especially for deep stacks.
TEST("ResumeCoroutine can be called in deeper contexts")
{
    u32 values[8];
    u32 i;
    struct Coroutine *co;
    for (i = 0; i < 8; i++)
        values[i] = i;
    co = CreateCoroutine(AllocateOnStackSuspendTwice, 8);
    co = ResumeCoroutine(co);
    EXPECT_NE(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
    co = ResumeArgument(co);
    EXPECT_NE(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
    co = ResumeCoroutine(co);
    EXPECT_EQ(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
}

TEST("ResumeCoroutine can be called in shallower contexts")
{
    u32 values[8];
    u32 i;
    struct Coroutine *co;
    for (i = 0; i < 8; i++)
        values[i] = i;
    co = CreateCoroutine(AllocateOnStackSuspendTwice, 8);
    co = ResumeArgument(co);
    EXPECT_NE(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
    co = ResumeCoroutine(co);
    EXPECT_NE(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
    co = ResumeArgument(co);
    EXPECT_EQ(co, NULL);
    for (i = 0; i < 8; i++)
        EXPECT_EQ(values[i], i);
}

TEST("Coroutines can be interleaved: 1, 2, 1, 2")
{
    struct Coroutine *co1;
    struct Coroutine *co2;
    co1 = CreateCoroutine(AllocateOnStackSuspend, 8);
    co2 = CreateCoroutine(AllocateOnStackSuspend, 16);
    co1 = ResumeCoroutine(co1);
    EXPECT_NE(co1, NULL);
    co2 = ResumeCoroutine(co2);
    EXPECT_NE(co2, NULL);
    co1 = ResumeCoroutine(co1);
    EXPECT_EQ(co1, NULL);
    co2 = ResumeCoroutine(co2);
    EXPECT_EQ(co2, NULL);
}

TEST("Coroutines can be interleaved: 1, 2, 2, 1")
{
    struct Coroutine *co1;
    struct Coroutine *co2;
    co1 = CreateCoroutine(AllocateOnStackSuspend, 8);
    co2 = CreateCoroutine(AllocateOnStackSuspend, 16);
    co1 = ResumeCoroutine(co1);
    EXPECT_NE(co1, NULL);
    co2 = ResumeCoroutine(co2);
    EXPECT_NE(co2, NULL);
    co2 = ResumeCoroutine(co2);
    EXPECT_EQ(co2, NULL);
    co1 = ResumeCoroutine(co1);
    EXPECT_EQ(co1, NULL);
}

TEST("CreateCoroutineTask resumes the coroutine when RunTasks called")
{
    s32 i;
    sState = 0;
    CreateCoroutineTask(CreateCoroutine(IncrementStateTwice), 1);
    EXPECT_EQ(sState, 0);
    RunTasks();
    EXPECT_EQ(sState, 1);
    RunTasks();
    EXPECT_EQ(sState, 2);
    for (i = 0; i < NUM_TASKS; i++)
        EXPECT(!gTasks[i].isActive);
}
