#ifndef GUARD_TEST_RUNNER_H
#define GUARD_TEST_RUNNER_H

enum TestResult
{
    TEST_RESULT_FAIL,
    TEST_RESULT_PASS,
    TEST_RESULT_SKIP,
    TEST_RESULT_INVALID,
    TEST_RESULT_ERROR,
    TEST_RESULT_TIMEOUT,
};

extern const bool8 gTestRunnerEnabled;
extern const bool8 gTestRunnerHeadless;
extern const bool8 gTestRunnerSkipIsFail;

void TestRunner_Battle_RecordAbilityPopUp(u32 battlerId, u32 ability);
void TestRunner_Battle_RecordAnimation(u32 animType, u32 animId);
void TestRunner_Battle_RecordHP(u32 battlerId, u32 oldHP, u32 newHP);
void TestRunner_Battle_RecordMessage(const u8 *message);
void TestRunner_Battle_RecordStatus1(u32 battlerId, u32 status1);
void TestRunner_Battle_AfterLastTurn(void);

void Test_ExitWithResult(enum TestResult result, const char *fmt, ...);
void BattleTest_CheckBattleRecordActionType(u32 battlerId, u32 recordIndex, u32 actionType);

#endif
