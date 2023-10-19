#ifndef GUARD_TEST_OVERWORLD_H
#define GUARD_TEST_OVERWORLD_H

#include "global.h"
#include "test/test.h"

struct OverworldTest
{
    u16 sourceLine;
    void (*function)(void);
};

#define OVERWORLD_TEST(_name) \
    static void CAT(Test, __LINE__)(void); \
    __attribute__((section(".tests"))) static const struct Test CAT(sTest, __LINE__) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gOverworldTestRunner, \
        .data = (void *)&(const struct OverworldTest) \
        { \
            .sourceLine = __LINE__, \
            .function = CAT(Test, __LINE__), \
        }, \
    }; \
    static void CAT(Test, __LINE__)(void)

#endif
