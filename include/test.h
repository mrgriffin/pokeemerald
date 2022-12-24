#ifndef GUARD_TEST_H
#define GUARD_TEST_H

#include "test_runner.h"

struct TestRunner
{
    void (*setUp)(void *);
    void (*run)(void *);
    void (*tearDown)(void *);
    const u8 *(*testIndex)(void *);
    bool32 (*checkProgress)(void *);
    bool32 (*handleExitWithResult)(void *, enum TestResult);
};

struct Test
{
    const char *name;
    const char *filename;
    const struct TestRunner *runner;
    void *data;
};

struct TestRunnerState
{
    u8 state;
    u8 exitCode;
    s32 tests;
    s32 passes;
    s32 skips;
    const char *skipFilename;
    const struct Test *test;

    u8 result;
    u32 timeoutSeconds;
};

extern const struct TestRunner gAssumptionsRunner;
extern struct TestRunnerState gTestRunnerState;

void CB2_TestRunner(void);

#define ASSUMPTIONS \
    static void Assumptions(void); \
    __attribute__((section(".tests"))) static const struct Test sAssumptions = \
    { \
        .name = "ASSUMPTIONS: " __FILE__, \
        .filename = __FILE__, \
        .runner = &gAssumptionsRunner, \
        .data = Assumptions, \
    }; \
    static void Assumptions(void)

#define ASSUME(c) \
    do \
    { \
        if (!(c)) \
            Test_ExitWithResult(TEST_RESULT_SKIP, "%s:%d: ASSUME failed", gTestRunnerState.test->filename, __LINE__); \
    } while (0)

#define EXPECT(c) \
    do \
    { \
        if (!(c)) \
            Test_ExitWithResult(TEST_RESULT_FAIL, "%s:%d: EXPECT failed", gTestRunnerState.test->filename, __LINE__); \
    } while (0)

#define EXPECT_EQ(a, b) \
    do \
    { \
        typeof(a) _a = (a), _b = (b); \
        if (_a != _b) \
            Test_ExitWithResult(TEST_RESULT_FAIL, "%s:%d: EXPECT_EQ(%d, %d) failed", gTestRunnerState.test->filename, __LINE__, _a, _b); \
    } while (0)

#endif
