#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "characters.h"
#include "main.h"
#include "malloc.h"
#include "test_battle.h"
#include "window.h"

#define INVALID(fmt, ...) Test_ExitWithResult(TEST_RESULT_INVALID, "%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__)
#define INVALID_IF(c, fmt, ...) do { if (c) Test_ExitWithResult(TEST_RESULT_INVALID, "%s:%d: " fmt, gTestRunnerState.test->filename, sourceLine, ##__VA_ARGS__); } while (0)

#define STATE gBattleTestRunnerState
#define DATA gBattleTestRunnerState->data

EWRAM_DATA struct BattleTestRunnerState *gBattleTestRunnerState = NULL;

static void CB2_BattleTest_NextParameter(void);
static void CB2_BattleTest_NextTrial(void);
static void PushBattlerAction(u32 sourceLine, s32 battlerId, u32 actionType, u32 byte);

NAKED static void InvokeSingleTestFunction(void *results, u32 i, struct BattlePokemon *player, struct BattlePokemon *opponent, SingleBattleTestFunction function, void *stack)
{
    asm("push {r4-r6,lr}\n\
         ldr r4, [sp, #16] @ function\n\
         ldr r5, [sp, #20] @ stack\n\
         mov r6, sp\n\
         mov sp, r5\n\
         push {r6}\n\
         ldr r6, =SingleRestoreSP + 1\n\
         mov lr, r6\n\
         bx r4\n\
    SingleRestoreSP:\n\
         pop {r0}\n\
         mov sp, r0\n\
         pop {r4-r6}\n\
         pop {r0}\n\
         bx r0\n\
        .pool");
}

NAKED static void InvokeDoubleTestFunction(void *results, u32 i, struct BattlePokemon *playerLeft, struct BattlePokemon *opponentLeft, struct BattlePokemon *playerRight, struct BattlePokemon *opponentRight, SingleBattleTestFunction function, void *stack)
{
    asm("push {r4-r7,lr}\n\
         ldr r4, [sp, #28] @ function\n\
         ldr r5, [sp, #32] @ stack\n\
         mov r6, sp\n\
         mov sp, r5\n\
         push {r6}\n\
         add r6, #20\n\
         ldmia r6, {r6, r7} @ playerRight, opponentRight\n\
         push {r6, r7}\n\
         ldr r6, =DoubleRestoreSP + 1\n\
         mov lr, r6\n\
         bx r4\n\
    DoubleRestoreSP:\n\
         add sp, #8\n\
         pop {r0}\n\
         mov sp, r0\n\
         pop {r4-r7}\n\
         pop {r0}\n\
         bx r0\n\
        .pool");
}

// Calls test->function, but pointing its stack at DATA.stack so that
// local variables are live after the function returns (and so can be
// referenced by HP_BAR, or the next call, etc).
// NOTE: This places the stack in EWRAM which has longer waitstates than
// IWRAM so could be much slower, but a) not that much code executes,
// and b) mga-rom-test isn't meaningfully limited by the GBA frame rate.
static void InvokeTestFunction(const struct BattleTest *test)
{
    STATE->parametersCount = 0;
    switch (test->type)
    {
    case BATTLE_TEST_SINGLES:
        InvokeSingleTestFunction(STATE->results, STATE->runParameter, &gBattleMons[B_POSITION_PLAYER_LEFT], &gBattleMons[B_POSITION_OPPONENT_LEFT], test->function.singles, &DATA.stack[BATTLE_TEST_STACK_SIZE]);
        break;
    case BATTLE_TEST_DOUBLES:
        InvokeDoubleTestFunction(STATE->results, STATE->runParameter, &gBattleMons[B_POSITION_PLAYER_LEFT], &gBattleMons[B_POSITION_OPPONENT_LEFT], &gBattleMons[B_POSITION_PLAYER_RIGHT], &gBattleMons[B_POSITION_OPPONENT_RIGHT], test->function.singles, &DATA.stack[BATTLE_TEST_STACK_SIZE]);
        break;
    }
}

static u32 SourceLine(u32 sourceLineOffset)
{
    const struct BattleTest *test = gTestRunnerState.test->data;
    return test->sourceLine + sourceLineOffset;
}

static u32 SourceLineOffset(u32 sourceLine)
{
    const struct BattleTest *test = gTestRunnerState.test->data;
    if (sourceLine - test->sourceLine > 0xFF)
        return 0;
    else
        return sourceLine - test->sourceLine;
}

static void BattleTest_SetUp(void *data)
{
    const struct BattleTest *test = data;
    STATE = AllocZeroed(sizeof(*STATE));
    if (!STATE)
        Test_ExitWithResult(TEST_RESULT_ERROR, "OOM: STATE = AllocZerod(%d)", sizeof(*STATE));
    InvokeTestFunction(test);
    STATE->parameters = STATE->parametersCount;
    STATE->results = AllocZeroed(test->resultsSize * STATE->parameters);
    if (!STATE)
        Test_ExitWithResult(TEST_RESULT_ERROR, "OOM: STATE->results = AllocZerod(%d)", sizeof(test->resultsSize * STATE->parameters));
    switch (test->type)
    {
    case BATTLE_TEST_SINGLES:
        STATE->battlersCount = 2;
        break;
    case BATTLE_TEST_DOUBLES:
        STATE->battlersCount = 4;
        break;
    }
}

// This does not take into account priority, statuses, or any other
// modifiers.
static void SetImplicitSpeeds(void)
{
    s32 i, j;
    u32 speed = 12;
    u32 hasSpeeds = 0;
    u32 allSpeeds = ((1 << DATA.playerPartySize) - 1) | (((1 << DATA.opponentPartySize) - 1) << 6);
    bool32 madeProgress;
    while (hasSpeeds != allSpeeds)
    {
        madeProgress = FALSE;
        for (i = 0; i < DATA.playerPartySize; i++)
        {
            if (!(hasSpeeds & (1 << i))
             && !(DATA.slowerThan[B_SIDE_PLAYER][i] & ~hasSpeeds))
            {
                SetMonData(&DATA.recordedBattle.playerParty[i], MON_DATA_SPEED, &speed);
                speed--;
                hasSpeeds |= 1 << i;
                madeProgress = TRUE;
            }
        }
        for (i = 0; i < DATA.opponentPartySize; i++)
        {
            if (!(hasSpeeds & ((1 << 6) << i))
             && !(DATA.slowerThan[B_SIDE_OPPONENT][i] & ~hasSpeeds))
            {
                SetMonData(&DATA.recordedBattle.opponentParty[i], MON_DATA_SPEED, &speed);
                speed--;
                hasSpeeds |= (1 << 6) << i;
                madeProgress = TRUE;
            }
        }
        if (!madeProgress)
            Test_ExitWithResult(TEST_RESULT_INVALID, "TURNs have contradictory speeds");
    }
}

/* RNG seeds for controlling the first move of the first turn.
 * Found via brute force.
 * The first 3 rolls are consumed by battle set-up. */

/* Default seed, triggers most things.
 * The 4th roll % 100 is <= 29, to make 30%+ accuracycheck pass.
 * The 5th roll is not a critical hit at the regular crit stage.
 * The 6th roll is consumed by damagecalc.
 * The 7th roll is consumed by adjustdamage.
 * The 8th roll % 100 is <= 9, to make 10%+ seteffectwithchance pass
 *          and % 3 is == 0, to make Poison Point and other 1/3s pass. */
#define RNG_SEED_DEFAULT 0x00000ED2

/* Causes the first attack to miss if possible.
 * The 4th roll % 100 is 99, to make 99%- accuracycheck fail. */
#define RNG_SEED_MISS 0x0000015B

static void BattleTest_Run(void *data)
{
    s32 i;
    u32 requiredPlayerPartySize;
    u32 requiredOpponentPartySize;
    const struct BattleTest *test = data;

    memset(&DATA, 0, sizeof(DATA));

    DATA.recordedBattle.rngSeed = RNG_SEED_DEFAULT;

    DATA.recordedBattle.textSpeed = OPTIONS_TEXT_SPEED_FAST;
    DATA.recordedBattle.battleFlags = BATTLE_TYPE_RECORDED_IS_MASTER | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_TRAINER | BATTLE_TYPE_IS_MASTER;
    if (test->type == BATTLE_TEST_DOUBLES)
        DATA.recordedBattle.battleFlags |= BATTLE_TYPE_DOUBLE;
    for (i = 0; i < STATE->battlersCount; i++)
    {
        DATA.recordedBattle.playersName[i][0] = CHAR_1 + i;
        DATA.recordedBattle.playersName[i][1] = EOS;
        DATA.recordedBattle.playersLanguage[i] = GAME_LANGUAGE;
        DATA.recordedBattle.playersBattlers[i] = i;

        DATA.currentMonIndexes[i] = (i & BIT_FLANK) == B_FLANK_LEFT ? 0 : 1;
    }

    STATE->runRandomly = TRUE;
    STATE->runGiven = TRUE;
    STATE->runWhen = TRUE;
    STATE->runScene = TRUE;
    InvokeTestFunction(test);

    requiredPlayerPartySize = 0;
    requiredOpponentPartySize = 0;
    for (i = 0; i < STATE->battlersCount; i++)
    {
        if ((i & BIT_SIDE) == B_SIDE_PLAYER)
            requiredPlayerPartySize = DATA.currentMonIndexes[i] + 1;
        else
            requiredOpponentPartySize = DATA.currentMonIndexes[i] + 1;
    }
    if (DATA.playerPartySize < requiredPlayerPartySize)
        Test_ExitWithResult(TEST_RESULT_INVALID, "%d PLAYER Pokemon required", requiredPlayerPartySize);
    if (DATA.opponentPartySize < requiredOpponentPartySize)
        Test_ExitWithResult(TEST_RESULT_INVALID, "%d OPPONENT Pokemon required", requiredOpponentPartySize);

    for (i = 0; i < STATE->battlersCount; i++)
        PushBattlerAction(0, i, RECORDED_BYTE, 0xFF);

    if (DATA.hasExplicitSpeeds)
    {
        if (DATA.explicitSpeeds[B_SIDE_PLAYER] != (1 << DATA.playerPartySize) - 1
         && DATA.explicitSpeeds[B_SIDE_OPPONENT] != (1 << DATA.opponentPartySize) - 1)
        {
            Test_ExitWithResult(TEST_RESULT_INVALID, "Speed required for all PLAYERs and OPPONENTs");
        }
    }
    else
    {
        SetImplicitSpeeds();
    }

    SetVariablesForRecordedBattle(&DATA.recordedBattle);

    if (STATE->trials)
        gMain.savedCallback = CB2_BattleTest_NextTrial;
    else if (STATE->parameters)
        gMain.savedCallback = CB2_BattleTest_NextParameter;
    else
        gMain.savedCallback = CB2_TestRunner;
    SetMainCallback2(CB2_InitBattle);

    STATE->checkProgressParameter = 0;
    STATE->checkProgressTrial = 0;
    STATE->checkProgressTurn = 0;
}

static bool32 TryAbilityPopUp(u32 i, u32 battlerId, u32 ability)
{
    struct QueuedAbilityEvent *event;

    if (DATA.queuedEvents[i].type != QUEUED_ABILITY_POPUP_EVENT)
        return FALSE;

    event = &DATA.queuedEvents[i].as.ability;

    return event->battlerId == battlerId
        && (event->ability == ABILITY_NONE || event->ability == ability);
}

void TestRunner_Battle_RecordAbilityPopUp(u32 battlerId, u32 ability)
{
    s32 i;
    if (DATA.queuedEvent == DATA.queuedEventsCount)
        return;
    for (i = DATA.queuedEvent; i < DATA.queuedEvent + DATA.queuedEvents[DATA.queuedEvent].groupSize; i++)
    {
        if (TryAbilityPopUp(i, battlerId, ability))
        {
            DATA.queuedEvent += DATA.queuedEvents[DATA.queuedEvent].groupSize;
            return;
        }
    }
}

static bool32 TryAnimation(u32 i, u32 animType, u32 animId)
{
    struct QueuedAnimationEvent *event;

    if (DATA.queuedEvents[i].type != QUEUED_ANIMATION_EVENT)
        return FALSE;

    event = &DATA.queuedEvents[i].as.animation;

    return event->type == animType
        && event->id == animId
        && (event->attacker == 0xF || event->attacker == gBattleAnimAttacker)
        && (event->target == 0xF || event->target == gBattleAnimTarget);
}

void TestRunner_Battle_RecordAnimation(u32 animType, u32 animId)
{
    s32 i;
    if (DATA.queuedEvent == DATA.queuedEventsCount)
        return;
    for (i = DATA.queuedEvent; i < DATA.queuedEvent + DATA.queuedEvents[DATA.queuedEvent].groupSize; i++)
    {
        if (TryAnimation(i, animType, animId))
        {
            DATA.queuedEvent += DATA.queuedEvents[DATA.queuedEvent].groupSize;
            return;
        }
    }
}

static bool32 TryHP(u32 i, u32 battlerId, u32 oldHP, u32 newHP)
{
    struct QueuedHPEvent *event;

    if (DATA.queuedEvents[i].type != QUEUED_HP_EVENT)
        return FALSE;

    event = &DATA.queuedEvents[i].as.hp;

    if (event->battlerId == battlerId)
    {
        if (event->address)
        {
            switch (event->type)
            {
            case HP_EVENT_NEW_HP:
                *(u16 *)event->address = newHP;
                break;
            case HP_EVENT_DELTA_HP:
                *(s16 *)event->address = oldHP - newHP;
                break;
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void TestRunner_Battle_RecordHP(u32 battlerId, u32 oldHP, u32 newHP)
{
    s32 i;
    if (DATA.queuedEvent == DATA.queuedEventsCount)
        return;
    for (i = DATA.queuedEvent; i < DATA.queuedEvent + DATA.queuedEvents[DATA.queuedEvent].groupSize; i++)
    {
        if (TryHP(i, battlerId, oldHP, newHP))
        {
            DATA.queuedEvent += DATA.queuedEvents[DATA.queuedEvent].groupSize;
            return;
        }
    }
}

static inline void SkipWhitespace(const u8 *s, s32 *i)
{
    while (TRUE)
    {
        switch (s[*i])
        {
        case CHAR_SPACE:
        case CHAR_PROMPT_SCROLL:
        case CHAR_PROMPT_CLEAR:
        case CHAR_NEWLINE:
            (*i)++;
            break;
        default:
            return;
        }
    }
}

static bool32 TryMessage(u32 i, const u8 *string)
{
    s32 j, k;
    struct QueuedMessageEvent *event;

    if (DATA.queuedEvents[i].type != QUEUED_MESSAGE_EVENT)
        return FALSE;

    event = &DATA.queuedEvents[i].as.message;
    for (j = k = 0; ; j++, k++)
    {
        if (event->pattern[k] == CHAR_SPACE)
        {
            SkipWhitespace(string, &j);
            k++;
        }
        if (event->pattern[k] == EOS)
        {
            SkipWhitespace(string, &j); // Consume trailing '\p', etc.
        }
        if (string[j] != event->pattern[k])
        {
            return FALSE;
        }
        else if (string[j] == EOS)
        {
            return TRUE;
        }
    }
}

void TestRunner_Battle_RecordMessage(const u8 *string)
{
    s32 i;
    if (DATA.queuedEvent == DATA.queuedEventsCount)
        return;
    for (i = DATA.queuedEvent; i < DATA.queuedEvent + DATA.queuedEvents[DATA.queuedEvent].groupSize; i++)
    {
        if (TryMessage(i, string))
        {
            DATA.queuedEvent += DATA.queuedEvents[DATA.queuedEvent].groupSize;
            return;
        }
    }
}

static const char *const sEventTypeMacros[] =
{
    [QUEUED_ABILITY_POPUP_EVENT] = "ABILITY_POPUP",
    [QUEUED_ANIMATION_EVENT] = "ANIMATION",
    [QUEUED_HP_EVENT] = "HP_BAR",
    [QUEUED_MESSAGE_EVENT] = "MESSAGE",
};

void TestRunner_Battle_AfterLastTurn(void)
{
    const struct BattleTest *test = gTestRunnerState.test->data;

    if (DATA.queuedEvent != DATA.queuedEventsCount)
    {
        const char *filename = gTestRunnerState.test->filename;
        u32 line = SourceLine(DATA.queuedEvents[DATA.queuedEvent].sourceLineOffset);
        const char *macro = sEventTypeMacros[DATA.queuedEvents[DATA.queuedEvent].type];
        Test_ExitWithResult(TEST_RESULT_FAIL, "%s:%d: Unmatched %s", filename, line, macro);
    }

    if (DATA.turns -1 != DATA.lastActionTurn)
    {
        const char *filename = gTestRunnerState.test->filename;
        Test_ExitWithResult(TEST_RESULT_FAIL, "%s: %d TURNs specified, but %d ran", filename, DATA.turns, DATA.lastActionTurn + 1);
    }

    STATE->runThen = TRUE;
    STATE->runFinally = STATE->runParameter + 1 == STATE->parameters;
    InvokeTestFunction(test);
}

static void CB2_BattleTest_NextParameter(void)
{
    if (++STATE->runParameter >= STATE->parameters)
        SetMainCallback2(CB2_TestRunner);
    else
        BattleTest_Run(gTestRunnerState.test->data);
}

static void CB2_BattleTest_NextTrial(void)
{
    SetMainCallback2(CB2_BattleTest_NextParameter);
    if (++STATE->runTrial < STATE->trials)
    {
        switch (gTestRunnerState.result)
        {
        case TEST_RESULT_FAIL:
            break;
        case TEST_RESULT_PASS:
            STATE->observedPasses++;
            break;
        case TEST_RESULT_SKIP:
            STATE->skippedTrials++;
            if (STATE->skippedTrials > STATE->trials / 4)
                Test_ExitWithResult(TEST_RESULT_INVALID, "25%% of the trials were SKIPed");
            break;
        default:
            return;
        }
        gTestRunnerState.result = TEST_RESULT_PASS;
        DATA.recordedBattle.rngSeed = STATE->runTrial;
        DATA.queuedEvent = 0;
        SetVariablesForRecordedBattle(&DATA.recordedBattle);
        FreeMonSpritesGfx();
        FreeBattleSpritesData();
        FreeBattleResources();
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_InitBattle);
    }
    else
    {
        if (abs(STATE->observedPasses - STATE->expectedPasses) <= 1)
            gTestRunnerState.result = TEST_RESULT_PASS;
        else
            Test_ExitWithResult(TEST_RESULT_FAIL, "Expected %d/%d passes, observed %d/%d", STATE->expectedPasses, STATE->trials, STATE->observedPasses, STATE->trials);
    }
}

static void BattleTest_TearDown(void *data)
{
    if (STATE)
    {
        FREE_AND_SET_NULL(STATE->results);
        FREE_AND_SET_NULL(STATE);
    }
}

static const u8 *BattleTest_TestIndex(void *data)
{
    if (!STATE
     || STATE->parameters == 0
     || STATE->runningFinally)
    {
        return NULL;
    }
    else
    {
        return &STATE->runParameter;
    }
}

static bool32 BattleTest_CheckProgress(void *data)
{
    bool32 madeProgress
         = STATE->checkProgressParameter < STATE->runParameter
        || STATE->checkProgressTrial < STATE->runTrial
        || STATE->checkProgressTurn < gBattleResults.battleTurnCounter;
    STATE->checkProgressParameter = STATE->runParameter;
    STATE->checkProgressTrial = STATE->runTrial;
    STATE->checkProgressTurn = gBattleResults.battleTurnCounter;
    return madeProgress;
}

static bool32 BattleTest_HandleExitWithResult(void *data, enum TestResult result)
{
    if (result != TEST_RESULT_INVALID
     && result != TEST_RESULT_ERROR
     && result != TEST_RESULT_TIMEOUT
     && STATE->runTrial < STATE->trials)
    {
        SetMainCallback2(CB2_BattleTest_NextTrial);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void Randomly(u32 passes, u32 trials)
{
    // This is a tolerance of +/- 2.5%.
    STATE->trials = 40;
    STATE->expectedPasses = 40 * passes / trials;
    DATA.recordedBattle.rngSeed = 0;
}

void RNGSeed_(u32 sourceLine, u32 seed)
{
    INVALID_IF(DATA.recordedBattle.rngSeed != RNG_SEED_DEFAULT, "RNG seed already set");
    DATA.recordedBattle.rngSeed = seed;
}

const struct TestRunner gBattleTestRunner =
{
    .setUp = BattleTest_SetUp,
    .run = BattleTest_Run,
    .tearDown = BattleTest_TearDown,
    .testIndex = BattleTest_TestIndex,
    .checkProgress = BattleTest_CheckProgress,
    .handleExitWithResult = BattleTest_HandleExitWithResult,
};

void OpenPokemon(u32 sourceLine, u32 side, u32 species)
{
    s32 i, data;
    u8 *partySize;
    struct Pokemon *party;
    INVALID_IF(species >= SPECIES_EGG, "Invalid species: %d", species);
    if (side == B_SIDE_PLAYER)
    {
        partySize = &DATA.playerPartySize;
        party = DATA.recordedBattle.playerParty;
    }
    else
    {
        partySize = &DATA.opponentPartySize;
        party = DATA.recordedBattle.opponentParty;
    }
    INVALID_IF(*partySize == PARTY_SIZE, "Too many Pokemon in party");
    DATA.currentSide = side;
    DATA.currentPartyIndex = *partySize;
    DATA.currentMon = &party[DATA.currentPartyIndex];
    DATA.gender = MON_MALE;
    DATA.nature = NATURE_HARDY;
    (*partySize)++;

    CreateMon(DATA.currentMon, species, 100, 0, TRUE, 0, OT_ID_PRESET, 0);
    data = MOVE_NONE;
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonData(DATA.currentMon, MON_DATA_MOVE1 + i, &data);
}

// (sNaturePersonalities[i] % NUM_NATURES) == i
// (sNaturePersonalities[i] & 0xFF) == 0
// NOTE: Using 25 << 8 rather than 0 << 8 to prevent shiny females.
static const u16 sNaturePersonalities[NUM_NATURES] =
{
    25 << 8, 21 << 8, 17 << 8, 13 << 8,  9 << 8,
     5 << 8,  1 << 8, 22 << 8, 18 << 8, 14 << 8,
    10 << 8,  6 << 8,  2 << 8, 23 << 8, 19 << 8,
    15 << 8, 11 << 8,  7 << 8,  3 << 8, 24 << 8,
    20 << 8, 16 << 8, 12 << 8,  8 << 8,  4 << 8,
};

static u32 GenerateNature(u32 nature, u32 offset)
{
    int i;
    if (offset <= nature)
        nature -= offset;
    else
        nature = nature + NUM_NATURES - offset;
    return sNaturePersonalities[nature];
}

void ClosePokemon(u32 sourceLine)
{
    s32 i;
    INVALID_IF(DATA.hasExplicitSpeeds && !(DATA.explicitSpeeds[DATA.currentSide] & (1 << DATA.currentPartyIndex)), "Speed required");
    for (i = 0; i < STATE->battlersCount; i++)
    {
        if ((i & BIT_SIDE) == DATA.currentSide
         && DATA.currentMonIndexes[i] == DATA.currentPartyIndex)
        {
            INVALID_IF(GetMonData(DATA.currentMon, MON_DATA_HP) == 0, "Battlers cannot be fainted");
        }
    }
    UpdateMonPersonality(&DATA.currentMon->box, GenerateNature(DATA.nature, DATA.gender % NUM_NATURES) | DATA.gender);
    DATA.currentMon = NULL;
}

void Gender_(u32 sourceLine, u32 gender)
{
    const struct SpeciesInfo *info;
    INVALID_IF(!DATA.currentMon, "Gender outside of PLAYER/OPPONENT");
    info = &gSpeciesInfo[GetMonData(DATA.currentMon, MON_DATA_SPECIES)];
    switch (gender)
    {
    case MON_MALE:
        DATA.gender = 0xFF;
        INVALID_IF(info->genderRatio == MON_GENDERLESS || info->genderRatio == MON_FEMALE, "Illegal male");
        break;
    case MON_FEMALE:
        DATA.gender = 0x00;
        INVALID_IF(info->genderRatio == MON_GENDERLESS || info->genderRatio == MON_MALE, "Illegal female");
        break;
    case MON_GENDERLESS:
        INVALID_IF(info->genderRatio != gender, "Illegal genderless");
        break;
    }
}

void Nature_(u32 sourceLine, u32 nature)
{
    INVALID_IF(!DATA.currentMon, "Nature outside of PLAYER/OPPONENT");
    INVALID_IF(nature >= NUM_NATURES, "Illegal nature: %d", nature);
    DATA.nature = nature;
}

void Ability_(u32 sourceLine, u32 ability)
{
    s32 i;
    u32 species;
    const struct SpeciesInfo *info;
    INVALID_IF(!DATA.currentMon, "Ability outside of PLAYER/OPPONENT");
    species = GetMonData(DATA.currentMon, MON_DATA_SPECIES);
    info = &gSpeciesInfo[species];
    for (i = 0; i < NUM_ABILITY_SLOTS; i++)
    {
        if (info->abilities[i] == ability)
        {
            SetMonData(DATA.currentMon, MON_DATA_ABILITY_NUM, &i);
            break;
        }
    }
    INVALID_IF(i == NUM_ABILITY_SLOTS, "%S cannot have %S", gSpeciesNames[species], gAbilityNames[ability]);
}

void Level_(u32 sourceLine, u32 level)
{
    // TODO: Preserve any explicitly-set stats.
    INVALID_IF(!DATA.currentMon, "Level outside of PLAYER/OPPONENT");
    INVALID_IF(level == 0 || level > MAX_LEVEL, "Illegal level: %d", level);
    SetMonData(DATA.currentMon, MON_DATA_LEVEL, &level);
}

void MaxHP_(u32 sourceLine, u32 maxHP)
{
    INVALID_IF(!DATA.currentMon, "MaxHP outside of PLAYER/OPPONENT");
    INVALID_IF(maxHP == 0, "Illegal max HP: %d", maxHP);
    SetMonData(DATA.currentMon, MON_DATA_MAX_HP, &maxHP);
}

void HP_(u32 sourceLine, u32 hp)
{
    INVALID_IF(!DATA.currentMon, "HP outside of PLAYER/OPPONENT");
    if (hp > GetMonData(DATA.currentMon, MON_DATA_MAX_HP))
        SetMonData(DATA.currentMon, MON_DATA_MAX_HP, &hp);
    SetMonData(DATA.currentMon, MON_DATA_HP, &hp);
}

void Attack_(u32 sourceLine, u32 attack)
{
    INVALID_IF(!DATA.currentMon, "Attack outside of PLAYER/OPPONENT");
    INVALID_IF(attack == 0, "Illegal attack: %d", attack);
    SetMonData(DATA.currentMon, MON_DATA_ATK, &attack);
}

void Defense_(u32 sourceLine, u32 defense)
{
    INVALID_IF(!DATA.currentMon, "Defense outside of PLAYER/OPPONENT");
    INVALID_IF(defense == 0, "Illegal defense: %d", defense);
    SetMonData(DATA.currentMon, MON_DATA_DEF, &defense);
}

void SpAttack_(u32 sourceLine, u32 spAttack)
{
    INVALID_IF(!DATA.currentMon, "SpAttack outside of PLAYER/OPPONENT");
    INVALID_IF(spAttack == 0, "Illegal special attack: %d", spAttack);
    SetMonData(DATA.currentMon, MON_DATA_SPATK, &spAttack);
}

void SpDefense_(u32 sourceLine, u32 spDefense)
{
    INVALID_IF(!DATA.currentMon, "SpDefense outside of PLAYER/OPPONENT");
    INVALID_IF(spDefense == 0, "Illegal special defense: %d", spDefense);
    SetMonData(DATA.currentMon, MON_DATA_SPDEF, &spDefense);
}

void Speed_(u32 sourceLine, u32 speed)
{
    INVALID_IF(!DATA.currentMon, "Speed outside of PLAYER/OPPONENT");
    INVALID_IF(speed == 0, "Illegal speed: %d", speed);
    SetMonData(DATA.currentMon, MON_DATA_SPEED, &speed);
    DATA.hasExplicitSpeeds = TRUE;
    DATA.explicitSpeeds[DATA.currentSide] |= 1 << DATA.currentPartyIndex;
}

void Item_(u32 sourceLine, u32 item)
{
    INVALID_IF(!DATA.currentMon, "Item outside of PLAYER/OPPONENT");
    INVALID_IF(item >= ITEMS_COUNT, "Illegal item: %d", item);
    SetMonData(DATA.currentMon, MON_DATA_HELD_ITEM, &item);
}

void Moves_(u32 sourceLine, const u16 moves[MAX_MON_MOVES])
{
    s32 i;
    INVALID_IF(!DATA.currentMon, "Moves outside of PLAYER/OPPONENT");
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] == MOVE_NONE)
            break;
        INVALID_IF(moves[i] >= MOVES_COUNT, "Illegal move: %d", moves[i]);
        SetMonData(DATA.currentMon, MON_DATA_MOVE1 + i, &moves[i]);
        SetMonData(DATA.currentMon, MON_DATA_PP1 + i, &gBattleMoves[moves[i]].pp);
    }
    DATA.explicitMoves[DATA.currentSide] |= 1 << DATA.currentPartyIndex;
}

void Friendship_(u32 sourceLine, u32 friendship)
{
    INVALID_IF(!DATA.currentMon, "Friendship outside of PLAYER/OPPONENT");
    SetMonData(DATA.currentMon, MON_DATA_FRIENDSHIP, &friendship);
}

void Status1_(u32 sourceLine, u32 status1)
{
    INVALID_IF(!DATA.currentMon, "Status1 outside of PLAYER/OPPONENT");
    INVALID_IF(status1 & STATUS1_TOXIC_COUNTER, "Illegal status1: has TOXIC_TURN");
    SetMonData(DATA.currentMon, MON_DATA_STATUS, &status1);
}

static void PushBattlerAction(u32 sourceLine, s32 battlerId, u32 actionType, u32 byte)
{
    u32 recordIndex = DATA.recordIndexes[battlerId]++;
    if (recordIndex >= BATTLER_RECORD_SIZE)
        Test_ExitWithResult(TEST_RESULT_INVALID, "Too many actions");
    DATA.battleRecordTypes[battlerId][recordIndex] = actionType;
    DATA.battleRecordSourceLineOffsets[battlerId][recordIndex] = SourceLineOffset(sourceLine);
    DATA.recordedBattle.battleRecord[battlerId][recordIndex] = byte;
}

void BattleTest_CheckBattleRecordActionType(u32 battlerId, u32 recordIndex, u32 actionType)
{
    DATA.lastActionTurn = gBattleResults.battleTurnCounter;
    if (actionType != DATA.battleRecordTypes[battlerId][recordIndex])
    {
        const char *actualMacro = NULL;
        const char *filename = gTestRunnerState.test->filename;
        u32 line = SourceLine(DATA.battleRecordSourceLineOffsets[battlerId][recordIndex]);

        switch (DATA.battleRecordTypes[battlerId][recordIndex])
        {
        case RECORDED_ACTION_TYPE:
            switch (DATA.recordedBattle.battleRecord[battlerId][recordIndex])
            {
            case B_ACTION_USE_MOVE:
                actualMacro = "MOVE";
                break;
            case B_ACTION_SWITCH:
                actualMacro = "SWITCH";
                break;
            }
            break;
        case RECORDED_PARTY_INDEX:
            actualMacro = "SEND_OUT";
            break;
        }

        if (actualMacro)
        {
            switch (actionType)
            {
            case RECORDED_ACTION_TYPE:
                Test_ExitWithResult(TEST_RESULT_INVALID, "%s:%d: Expected MOVE/SWITCH, got %s", filename, line, actualMacro);
            case RECORDED_PARTY_INDEX:
                Test_ExitWithResult(TEST_RESULT_INVALID, "%s:%d: Expected SEND_OUT, got %s", filename, line, actualMacro);
            }
        }

        Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: Illegal battle record", filename, line);
    }
}

void OpenTurn(u32 sourceLine)
{
    INVALID_IF(DATA.turnState != TURN_CLOSED, "Nested TURN");
    DATA.turnState = TURN_OPEN;
    DATA.actionBattlers = 0x00;
    DATA.moveBattlers = 0x00;
}

static void SetSlowerThan(s32 battlerId)
{
    s32 i, slowerThan;
    slowerThan = 0;
    for (i = 0; i < STATE->battlersCount; i++)
    {
        if (DATA.moveBattlers & (1 << i))
        {
            if ((i & BIT_SIDE) == B_SIDE_PLAYER)
                slowerThan |= 1 << DATA.currentMonIndexes[i];
            else
                slowerThan |= (1 << 6) << DATA.currentMonIndexes[i];
        }
    }
    DATA.slowerThan[battlerId & BIT_SIDE][DATA.currentMonIndexes[battlerId]] |= slowerThan;
}

void CloseTurn(u32 sourceLine)
{
    s32 i;
    INVALID_IF(DATA.turnState != TURN_OPEN, "Nested TURN");
    DATA.turnState = TURN_CLOSING;
    for (i = 0; i < STATE->battlersCount; i++)
    {
        if (!(DATA.actionBattlers & (1 << i)))
            Move(sourceLine, &gBattleMons[i], (struct MoveContext) { move: MOVE_SPLASH, explicitMove: TRUE });
    }
    DATA.turnState = TURN_CLOSED;
    DATA.turns++;
}

static struct Pokemon *CurrentMon(s32 battlerId)
{
    struct Pokemon *party;
    if ((battlerId & BIT_SIDE) == B_SIDE_PLAYER)
        party = DATA.recordedBattle.playerParty;
    else
        party = DATA.recordedBattle.opponentParty;
    return &party[DATA.currentMonIndexes[battlerId]];
}

static s32 MoveSlot(u32 sourceLine, s32 battlerId, u32 move)
{
}

void Move(u32 sourceLine, struct BattlePokemon *user, struct MoveContext ctx)
{
    s32 i;
    s32 battlerId = user - gBattleMons;
    struct Pokemon *mon = CurrentMon(battlerId);
    u32 moveId, moveSlot;
    s32 target;

    INVALID_IF(DATA.turnState == TURN_CLOSED, "MOVE outside TURN");
    INVALID_IF(DATA.actionBattlers & (1 << battlerId), "Multiple battler actions");

    if (ctx.explicitMove)
    {
        INVALID_IF(ctx.move == MOVE_NONE || ctx.move >= MOVES_COUNT, "Illegal move: %d", ctx.move);
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            moveId = GetMonData(mon, MON_DATA_MOVE1 + i);
            if (moveId == ctx.move)
            {
                moveSlot = i;
                break;
            }
            else if (moveId == MOVE_NONE)
            {
                INVALID_IF(DATA.explicitMoves[battlerId & BIT_SIDE] & (1 << DATA.currentMonIndexes[battlerId]), "Missing explicit %S", gMoveNames[ctx.move]);
                INVALID_IF(i == MAX_MON_MOVES, "Too many different moves");
                SetMonData(mon, MON_DATA_MOVE1 + i, &ctx.move);
                SetMonData(DATA.currentMon, MON_DATA_PP1 + i, &gBattleMoves[ctx.move].pp);
                moveSlot = i;
                moveId = ctx.move;
                break;
            }
        }
    }
    else if (ctx.explicitMoveSlot)
    {
        moveSlot = ctx.moveSlot;
        moveId = GetMonData(mon, MON_DATA_MOVE1 + moveSlot);
        INVALID_IF(moveId == MOVE_NONE, "Empty moveSlot: %d", ctx.moveSlot);
    }
    else
    {
        INVALID("No move or moveSlot");
    }

    if (ctx.explicitMegaEvolve && ctx.megaEvolve)
        moveSlot |= RET_MEGA_EVOLUTION;

    if (ctx.explicitTarget)
    {
        target = ctx.target - gBattleMons;
    }
    else
    {
        const struct BattleMove *move = &gBattleMoves[moveId];
        if (move->target == MOVE_TARGET_BOTH
         || move->target == MOVE_TARGET_FOES_AND_ALLY
         || move->target == MOVE_TARGET_OPPONENTS_FIELD
         || move->target == MOVE_TARGET_ALL_BATTLERS)
        {
            target = BATTLE_OPPOSITE(battlerId);
        }
        else if (move->target == MOVE_TARGET_SELECTED)
        {
            INVALID_IF(STATE->battlersCount > 2, "%S requires explicit target", gMoveNames[moveId]);

            target = BATTLE_OPPOSITE(battlerId);
        }
        else if (move->target == MOVE_TARGET_USER)
        {
            target = battlerId;
        }
        else if (move->target == MOVE_TARGET_ALLY)
        {
            target = BATTLE_PARTNER(battlerId);
        }
        else
        {
            INVALID("%S requires explicit target", gMoveNames[moveId]);
        }
    }

    if (ctx.explicitHit)
    {
        if (DATA.turns != 0 || DATA.hasRNGActions != 0)
            Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: hit only supported on the first move", gTestRunnerState.test->filename, sourceLine);
        INVALID_IF(DATA.recordedBattle.rngSeed != RNG_SEED_DEFAULT, "RNG seed already set");
        DATA.recordedBattle.rngSeed = RNG_SEED_MISS;
    }

    PushBattlerAction(sourceLine, battlerId, RECORDED_ACTION_TYPE, B_ACTION_USE_MOVE);
    PushBattlerAction(sourceLine, battlerId, RECORDED_MOVE_SLOT, moveSlot);
    PushBattlerAction(sourceLine, battlerId, RECORDED_MOVE_TARGET, target);

    if (DATA.turnState == TURN_OPEN)
    {
        if (!DATA.hasExplicitSpeeds)
            SetSlowerThan(battlerId);

        DATA.actionBattlers |= 1 << battlerId;
        DATA.moveBattlers |= 1 << battlerId;
    }

    // WARNING: Approximation. The move could still cause the RNG to
    // advance.
    if (gBattleMoves[moveId].accuracy != 0
     || gBattleMoves[moveId].split != SPLIT_STATUS)
    {
        DATA.hasRNGActions = TRUE;
    }
}

void Switch(u32 sourceLine, struct BattlePokemon *battler, u32 partyIndex)
{
    s32 i;
    s32 battlerId = battler - gBattleMons;
    INVALID_IF(DATA.turnState == TURN_CLOSED, "SWITCH outside TURN");
    INVALID_IF(DATA.actionBattlers & (1 << battlerId), "Multiple battler actions");
    INVALID_IF(partyIndex >= ((battlerId & BIT_SIDE) == B_SIDE_PLAYER ? DATA.playerPartySize : DATA.opponentPartySize), "SWITCH to invalid party index");

    for (i = 0; i < STATE->battlersCount; i++)
    {
        if (battlerId != i && (battlerId & BIT_SIDE) == (i & BIT_SIDE))
            INVALID_IF(DATA.currentMonIndexes[i] == partyIndex, "SWITCH to battler");
    }

    PushBattlerAction(sourceLine, battlerId, RECORDED_ACTION_TYPE, B_ACTION_SWITCH);
    PushBattlerAction(sourceLine, battlerId, RECORDED_PARTY_INDEX, partyIndex);
    DATA.currentMonIndexes[battlerId] = partyIndex;

    DATA.actionBattlers |= 1 << battlerId;
}

void SendOut(u32 sourceLine, struct BattlePokemon *battler, u32 partyIndex)
{
    s32 i;
    s32 battlerId = battler - gBattleMons;
    INVALID_IF(DATA.turnState == TURN_CLOSED, "SEND_OUT outside TURN");
    INVALID_IF(partyIndex >= ((battlerId & BIT_SIDE) == B_SIDE_PLAYER ? DATA.playerPartySize : DATA.opponentPartySize), "SWITCH to invalid party index");
    for (i = 0; i < STATE->battlersCount; i++)
    {
        if (battlerId != i && (battlerId & BIT_SIDE) == (i & BIT_SIDE))
            INVALID_IF(DATA.currentMonIndexes[i] == partyIndex, "SEND_OUT to battler");
    }
    if (!(DATA.actionBattlers & (1 << battlerId)))
        Move(sourceLine, battler, (struct MoveContext) { move: MOVE_SPLASH, explicitMove: TRUE });
    PushBattlerAction(sourceLine, battlerId, RECORDED_PARTY_INDEX, partyIndex);
    DATA.currentMonIndexes[battlerId] = partyIndex;
}

static const char *const sQueueGroupTypeMacros[] =
{
    [QUEUE_GROUP_NONE] = NULL,
    [QUEUE_GROUP_ONE_OF] = "ONE_OF",
};

void OpenQueueGroup(u32 sourceLine, enum QueueGroupType type)
{
    INVALID_IF(DATA.queueGroupType, "%s inside %s", sQueueGroupTypeMacros[type], sQueueGroupTypeMacros[DATA.queueGroupType]);
    DATA.queueGroupType = type;
    DATA.queueGroupStart = DATA.queuedEventsCount;
}

void CloseQueueGroup(u32 sourceLine)
{
    u32 groupSize = DATA.queuedEventsCount - DATA.queueGroupStart;
    if (groupSize > 0)
    {
        DATA.queuedEvents[DATA.queueGroupStart].groupSize = groupSize;
        DATA.queueGroupType = QUEUE_GROUP_NONE;
    }
}

void QueueAbility(u32 sourceLine, struct BattlePokemon *battler, struct AbilityEventContext ctx)
{
#if B_ABILITY_POP_UP
    s32 battlerId = battler - gBattleMons;
    if (DATA.queuedEventsCount == MAX_QUEUED_EVENTS)
        Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: ABILITY exceeds MAX_QUEUED_EVENTS", gTestRunnerState.test->filename, sourceLine);
    DATA.queuedEvents[DATA.queuedEventsCount++] = (struct QueuedEvent) {
        .type = QUEUED_ABILITY_POPUP_EVENT,
        .sourceLineOffset = SourceLineOffset(sourceLine),
        .groupSize = 1,
        .as = { .ability = {
            .battlerId = battlerId,
            .ability = ctx.ability,
        }},
    };
#endif
}

void QueueAnimation(u32 sourceLine, u32 type, u32 id, struct AnimationEventContext ctx)
{
    s32 attackerId, targetId;

    if (DATA.queuedEventsCount == MAX_QUEUED_EVENTS)
        Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: ANIMATION exceeds MAX_QUEUED_EVENTS", gTestRunnerState.test->filename, sourceLine);

    attackerId = ctx.attacker ? ctx.attacker - gBattleMons : 0xF;
    if (type == ANIM_TYPE_MOVE)
    {
        targetId = ctx.target ? ctx.target - gBattleMons : 0xF;
    }
    else
    {
        INVALID_IF(ctx.target, "ANIMATION target set for non-ANIM_TYPE_MOVE");
        targetId = 0xF;
    }

    DATA.queuedEvents[DATA.queuedEventsCount++] = (struct QueuedEvent) {
        .type = QUEUED_ANIMATION_EVENT,
        .sourceLineOffset = SourceLineOffset(sourceLine),
        .groupSize = 1,
        .as = { .animation = {
            .type = type,
            .id = id,
            .attacker = attackerId,
            .target = targetId,
        }},
    };
}

void QueueHP(u32 sourceLine, struct BattlePokemon *battler, struct HPEventContext ctx)
{
    s32 battlerId = battler - gBattleMons;
    u32 type;
    uintptr_t address;

    if (DATA.queuedEventsCount == MAX_QUEUED_EVENTS)
        Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: HP_BAR exceeds MAX_QUEUED_EVENTS", gTestRunnerState.test->filename, sourceLine);

    if (ctx.hp)
    {
        type = HP_EVENT_NEW_HP;
        address = (uintptr_t)ctx.hp;
    }
    else if (ctx.damage)
    {
        type = HP_EVENT_DELTA_HP;
        *ctx.damage = 0;
        address = (uintptr_t)ctx.damage;
    }
    else
    {
        address = 0;
    }

    DATA.queuedEvents[DATA.queuedEventsCount++] = (struct QueuedEvent) {
        .type = QUEUED_HP_EVENT,
        .sourceLineOffset = SourceLineOffset(sourceLine),
        .groupSize = 1,
        .as = { .hp = {
            .battlerId = battlerId,
            .type = type,
            .address = address,
        }},
    };
}

void QueueMessage(u32 sourceLine, const u8 *pattern)
{
    if (DATA.queuedEventsCount == MAX_QUEUED_EVENTS)
        Test_ExitWithResult(TEST_RESULT_ERROR, "%s:%d: MESSAGE exceeds MAX_QUEUED_EVENTS", gTestRunnerState.test->filename, sourceLine);
    DATA.queuedEvents[DATA.queuedEventsCount++] = (struct QueuedEvent) {
        .type = QUEUED_MESSAGE_EVENT,
        .sourceLineOffset = SourceLineOffset(sourceLine),
        .groupSize = 1,
        .as = { .message = {
            .pattern = pattern,
        }},
    };
}
