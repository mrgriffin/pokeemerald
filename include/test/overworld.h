#ifndef GUARD_TEST_OVERWORLD_H
#define GUARD_TEST_OVERWORLD_H

#include "test/test.h"

#define OVERWORLD_TEST(_name) \
    static void CAT(Test, __LINE__)(void); \
    __attribute__((section(".tests"), used)) static const struct Test CAT(sTest, __LINE__) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gOverworldTestRunner, \
        .sourceLine = __LINE__, \
        .data = (void *)CAT(Test, __LINE__), \
    }; \
    static void CAT(Test, __LINE__)(void)

#endif
