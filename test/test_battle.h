#ifndef GUARD_TEST_BATTLE_H
#define GUARD_TEST_BATTLE_H

#include "battle.h"
#include "battle_anim.h"
#include "data.h"
#include "recorded_battle.h"
#include "test.h"
#include "util.h"
#include "constants/abilities.h"
#include "constants/battle_anim.h"
#include "constants/battle_move_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"

// NOTE: If the stack is too small the test runner will probably crash
// or loop.
#define BATTLE_TEST_STACK_SIZE 1024
#define MAX_QUEUED_EVENTS 16

enum { BATTLE_TEST_SINGLES, BATTLE_TEST_DOUBLES };

typedef void (*SingleBattleTestFunction)(void *, u32, struct BattlePokemon *, struct BattlePokemon *);
typedef void (*DoubleBattleTestFunction)(void *, u32, struct BattlePokemon *, struct BattlePokemon *, struct BattlePokemon *, struct BattlePokemon *);

struct BattleTest
{
    u8 type;
    u16 sourceLine;
    union
    {
        SingleBattleTestFunction singles;
        DoubleBattleTestFunction doubles;
    } function;
    size_t resultsSize;
};

enum
{
    QUEUED_ABILITY_POPUP_EVENT,
    QUEUED_ANIMATION_EVENT,
    QUEUED_HP_EVENT,
    QUEUED_MESSAGE_EVENT,
    QUEUED_STATUS_EVENT,
};

struct QueuedAbilityEvent
{
    u8 battlerId;
    u16 ability;
};

struct QueuedAnimationEvent
{
    u8 type;
    u16 id;
    u8 attacker:4;
    u8 target:4;
};

enum { HP_EVENT_NEW_HP, HP_EVENT_DELTA_HP };

struct QueuedHPEvent
{
    u32 battlerId:3;
    u32 type:1;
    u32 address:28;
};

struct QueuedMessageEvent
{
    const u8 *pattern;
};

struct QueuedStatusEvent
{
    u32 battlerId:3;
    u32 mask:8;
    u32 unused_01:21;
};

struct QueuedEvent
{
    u8 type;
    u8 sourceLineOffset;
    u8 groupType:2;
    u8 groupSize:6;
    union
    {
        struct QueuedAbilityEvent ability;
        struct QueuedAnimationEvent animation;
        struct QueuedHPEvent hp;
        struct QueuedMessageEvent message;
        struct QueuedStatusEvent status;
    } as;
};

struct BattleTestData
{
    u8 stack[BATTLE_TEST_STACK_SIZE];

    u8 playerPartySize;
    u8 opponentPartySize;
    u8 explicitMoves[NUM_BATTLE_SIDES];
    bool8 hasExplicitSpeeds;
    u8 explicitSpeeds[NUM_BATTLE_SIDES];
    u16 slowerThan[NUM_BATTLE_SIDES][PARTY_SIZE];
    u8 currentSide;
    u8 currentPartyIndex;
    struct Pokemon *currentMon;
    u8 gender;
    u8 nature;

    u8 currentMonIndexes[MAX_BATTLERS_COUNT];
    u8 turnState;
    u8 turns;
    u8 actionBattlers;
    u8 moveBattlers;
    bool8 hasRNGActions:1;

    struct RecordedBattleSave recordedBattle;
    u8 battleRecordTypes[MAX_BATTLERS_COUNT][BATTLER_RECORD_SIZE];
    u8 battleRecordSourceLineOffsets[MAX_BATTLERS_COUNT][BATTLER_RECORD_SIZE];
    u16 recordIndexes[MAX_BATTLERS_COUNT];
    u8 lastActionTurn;

    u8 queuedEventsCount;
    u8 queueGroupType;
    u8 queueGroupStart;
    u8 queuedEvent;
    struct QueuedEvent queuedEvents[MAX_QUEUED_EVENTS];
};

struct BattleTestRunnerState
{
    u8 battlersCount;
    u8 parametersCount; // Valid only in BattleTest_Setup.
    u8 parameters;
    u8 runParameter;
    u8 trials;
    u8 expectedPasses;
    u8 observedPasses;
    u8 skippedTrials;
    u8 runTrial;
    bool8 runRandomly:1;
    bool8 runGiven:1;
    bool8 runWhen:1;
    bool8 runScene:1;
    bool8 runThen:1;
    bool8 runFinally:1;
    bool8 runningFinally:1;
    struct BattleTestData data;
    u8 *results;
    u8 checkProgressParameter;
    u8 checkProgressTrial;
    u8 checkProgressTurn;
};

extern const struct TestRunner gBattleTestRunner;
extern struct BattleTestRunnerState *gBattleTestRunnerState;

#define MEMBERS(...) VARARG_8(MEMBERS_, __VA_ARGS__)
#define MEMBERS_0()
#define MEMBERS_1(a) a;
#define MEMBERS_2(a, b) a; b;
#define MEMBERS_3(a, b, c) a; b; c;
#define MEMBERS_4(a, b, c, d) a; b; c; d;
#define MEMBERS_5(a, b, c, d, e) a; b; c; d; e;
#define MEMBERS_6(a, b, c, d, e, f) a; b; c; d; e; f;
#define MEMBERS_7(a, b, c, d, e, f, g) a; b; c; d; e; f; g;
#define MEMBERS_8(a, b, c, d, e, f, g, h) a; b; c; d; e; f; g; h;

#define APPEND_TRUE(...) VARARG_8(APPEND_TRUE_, __VA_ARGS__)
#define APPEND_TRUE_0()
#define APPEND_TRUE_1(a) a, TRUE
#define APPEND_TRUE_2(a, b) a, TRUE, b, TRUE
#define APPEND_TRUE_3(a, b, c) a, TRUE, b, TRUE, c, TRUE
#define APPEND_TRUE_4(a, b, c, d) a, TRUE, b, TRUE, c, TRUE, d, TRUE
#define APPEND_TRUE_5(a, b, c, d, e) a, TRUE, b, TRUE, c, TRUE, d, TRUE, e, TRUE
#define APPEND_TRUE_6(a, b, c, d, e, f) a, TRUE, b, TRUE, c, TRUE, d, TRUE, e, TRUE, f, TRUE
#define APPEND_TRUE_7(a, b, c, d, e, f, g) a, TRUE, b, TRUE, c, TRUE, d, TRUE, e, TRUE, f, TRUE, g, TRUE
#define APPEND_TRUE_8(a, b, c, d, e, f, g, h) a, TRUE, b, TRUE, c, TRUE, d, TRUE, e, TRUE, f, TRUE, g, TRUE, h, TRUE

/* Test */

#define SINGLE_BATTLE_TEST(_name, ...) \
    struct CAT(Result, __LINE__) { MEMBERS(__VA_ARGS__) }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *, u32, struct BattlePokemon *, struct BattlePokemon *); \
    __attribute__((section(".tests"))) static const struct Test CAT(sTest, __LINE__) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gBattleTestRunner, \
        .data = (void *)&(const struct BattleTest) \
        { \
            .type = BATTLE_TEST_SINGLES, \
            .sourceLine = __LINE__, \
            .function = { .singles = (SingleBattleTestFunction)CAT(Test, __LINE__) }, \
            .resultsSize = sizeof(struct CAT(Result, __LINE__)), \
        }, \
    }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *results, u32 i, struct BattlePokemon *player, struct BattlePokemon *opponent)

#define DOUBLE_BATTLE_TEST(_name, ...) \
    struct CAT(Result, __LINE__) { MEMBERS(__VA_ARGS__) }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *, u32, struct BattlePokemon *, struct BattlePokemon *, struct BattlePokemon *, struct BattlePokemon *); \
    __attribute__((section(".tests"))) static const struct Test CAT(sTest, __LINE__) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gBattleTestRunner, \
        .data = (void *)&(const struct BattleTest) \
        { \
            .type = BATTLE_TEST_DOUBLES, \
            .sourceLine = __LINE__, \
            .function = { .doubles = (DoubleBattleTestFunction)CAT(Test, __LINE__) }, \
            .resultsSize = sizeof(struct CAT(Result, __LINE__)), \
        }, \
    }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *results, u32 i, struct BattlePokemon *playerLeft, struct BattlePokemon *opponentLeft, struct BattlePokemon *playerRight, struct BattlePokemon *opponentRight)

/* Parametrize */

#define PARAMETRIZE if (gBattleTestRunnerState->parametersCount++ == i)

/* Randomly */

#define PASSES_RANDOMLY(passes, trials) for (; gBattleTestRunnerState->runRandomly; gBattleTestRunnerState->runRandomly = FALSE) Randomly(__LINE__, passes, trials)

void Randomly(u32 sourceLine, u32 passes, u32 trials);

/* Given */

#define GIVEN for (; gBattleTestRunnerState->runGiven; gBattleTestRunnerState->runGiven = FALSE)

#define RNGSeed(seed) RNGSeed_(__LINE__, seed)

#define PLAYER(species) for (OpenPokemon(__LINE__, B_SIDE_PLAYER, species); gBattleTestRunnerState->data.currentMon; ClosePokemon(__LINE__))
#define OPPONENT(species) for (OpenPokemon(__LINE__, B_SIDE_OPPONENT, species); gBattleTestRunnerState->data.currentMon; ClosePokemon(__LINE__))

#define Gender(gender) Gender_(__LINE__, gender)
#define Nature(nature) Nature_(__LINE__, nature)
#define Ability(ability) Ability_(__LINE__, ability)
#define Level(level) Level_(__LINE__, level)
#define MaxHP(maxHP) MaxHP_(__LINE__, maxHP)
#define HP(hp) HP_(__LINE__, hp)
#define Attack(attack) Attack_(__LINE__, attack)
#define Defense(defense) Defense_(__LINE__, defense)
#define SpAttack(spAttack) SpAttack_(__LINE__, spAttack)
#define SpDefense(spDefense) SpDefense_(__LINE__, spDefense)
#define Speed(speed) Speed_(__LINE__, speed)
#define Item(item) Item_(__LINE__, item)
#define Moves(move1, ...) Moves_(__LINE__, (const u16 [MAX_MON_MOVES]) { move1, __VA_ARGS__ })
#define Friendship(friendship) Friendship_(__LINE__, friendship)
#define Status1(status1) Status1_(__LINE__, status1)

void OpenPokemon(u32 sourceLine, u32 side, u32 species);
void ClosePokemon(u32 sourceLine);

void RNGSeed_(u32 sourceLine, u32 seed);
void Gender_(u32 sourceLine, u32 gender);
void Nature_(u32 sourceLine, u32 nature);
void Ability_(u32 sourceLine, u32 ability);
void Level_(u32 sourceLine, u32 level);
void MaxHP_(u32 sourceLine, u32 maxHP);
void HP_(u32 sourceLine, u32 hp);
void Attack_(u32 sourceLine, u32 attack);
void Defense_(u32 sourceLine, u32 defense);
void SpAttack_(u32 sourceLine, u32 spAttack);
void SpDefense_(u32 sourceLine, u32 spDefense);
void Speed_(u32 sourceLine, u32 speed);
void Item_(u32 sourceLine, u32 item);
void Moves_(u32 sourceLine, const u16 moves[MAX_MON_MOVES]);
void Friendship_(u32 sourceLine, u32 friendship);
void Status1_(u32 sourceLine, u32 status1);

#define PLAYER_PARTY (gBattleTestRunnerState->data.recordedBattle.playerParty)
#define OPPONENT_PARTY (gBattleTestRunnerState->data.recordedBattle.opponentParty)

/* When */

#define WHEN for (; gBattleTestRunnerState->runWhen; gBattleTestRunnerState->runWhen = FALSE)

enum { TURN_CLOSED, TURN_OPEN, TURN_CLOSING };

#define TURN for (OpenTurn(__LINE__); gBattleTestRunnerState->data.turnState == TURN_OPEN; CloseTurn(__LINE__))

#define MOVE(user, ...) Move(__LINE__, user, (struct MoveContext) { APPEND_TRUE(__VA_ARGS__) })
#define SWITCH(battler, partyIndex) Switch(__LINE__, battler, partyIndex)
#define SEND_OUT(battler, partyIndex) SendOut(__LINE__, battler, partyIndex)

struct MoveContext
{
    u16 move;
    u16 explicitMove:1;
    u16 moveSlot:2;
    u16 explicitMoveSlot:1;
    u16 hit:1;
    u16 explicitHit:1;
    u16 megaEvolve:1;
    u16 explicitMegaEvolve:1;
    // TODO: u8 zMove:1;
    u16 allowed:1;
    u16 explicitAllowed:1;
    struct BattlePokemon *target;
    bool8 explicitTarget;
};

void OpenTurn(u32 sourceLine);
void CloseTurn(u32 sourceLine);
void Move(u32 sourceLine, struct BattlePokemon *, struct MoveContext);
void Switch(u32 sourceLine, struct BattlePokemon *, u32 partyIndex);

void SendOut(u32 sourceLine, struct BattlePokemon *, u32 partyIndex);

/* Scene */

#define SCENE for (; gBattleTestRunnerState->runScene; gBattleTestRunnerState->runScene = FALSE)

#define ONE_OF for (OpenQueueGroup(__LINE__, QUEUE_GROUP_ONE_OF); gBattleTestRunnerState->data.queueGroupType != QUEUE_GROUP_NONE; CloseQueueGroup(__LINE__))
#define NONE_OF for (OpenQueueGroup(__LINE__, QUEUE_GROUP_NONE_OF); gBattleTestRunnerState->data.queueGroupType != QUEUE_GROUP_NONE; CloseQueueGroup(__LINE__))

#define ABILITY_POPUP(battler, ...) QueueAbility(__LINE__, battler, (struct AbilityEventContext) { __VA_ARGS__ })
#define ANIMATION(type, id, ...) QueueAnimation(__LINE__, type, id, (struct AnimationEventContext) { __VA_ARGS__ })
#define HP_BAR(battler, ...) QueueHP(__LINE__, battler, (struct HPEventContext) { APPEND_TRUE(__VA_ARGS__) })
#define MESSAGE(pattern) QueueMessage(__LINE__, (const u8 []) _(pattern))
#define STATUS_ICON(battler, ...) QueueStatus(__LINE__, battler, (struct StatusEventContext) { __VA_ARGS__ })

enum QueueGroupType
{
    QUEUE_GROUP_NONE,
    QUEUE_GROUP_ONE_OF,
    QUEUE_GROUP_NONE_OF,
};

struct AbilityEventContext
{
    u16 ability;
};

struct AnimationEventContext
{
    struct BattlePokemon *attacker;
    struct BattlePokemon *target;
};

struct HPEventContext
{
    u8 _;
    u16 hp;
    bool8 explicitHP;
    s16 damage;
    bool8 explicitDamage;
    u16 *captureHP;
    bool8 explicitCaptureHP;
    s16 *captureDamage;
    bool8 explicitCaptureDamage;
};

struct StatusEventContext
{
    bool8 none:1;
    bool8 sleep:1;
    bool8 poison:1;
    bool8 burn:1;
    bool8 freeze:1;
    bool8 paralysis:1;
};

void OpenQueueGroup(u32 sourceLine, enum QueueGroupType);
void CloseQueueGroup(u32 sourceLine);

void QueueAbility(u32 sourceLine, struct BattlePokemon *battler, struct AbilityEventContext);
void QueueAnimation(u32 sourceLine, u32 type, u32 id, struct AnimationEventContext);
void QueueHP(u32 sourceLine, struct BattlePokemon *battler, struct HPEventContext);
void QueueMessage(u32 sourceLine, const u8 *pattern);
void QueueStatus(u32 sourceLine, struct BattlePokemon *battler, struct StatusEventContext);

/* Then */

#define THEN for (; gBattleTestRunnerState->runThen; gBattleTestRunnerState->runThen = FALSE)

/* Finally */

#define FINALLY for (; gBattleTestRunnerState->runFinally; gBattleTestRunnerState->runFinally = FALSE) if ((gBattleTestRunnerState->runningFinally = TRUE))

#define PARAMETERS (gBattleTestRunnerState->parameters)

/* Expect */

#define EXPECT_MUL_EQ(a, m, b) \
    do \
    { \
        s32 _a = (a), _m = (m), _b = (b); \
        s32 _am = Q_4_12_TO_INT(_a * _m); \
        s32 _t = Q_4_12_TO_INT(abs(_m) + Q_4_12_ROUND); \
        if (abs(_am-_b) > _t) \
            Test_ExitWithResult(TEST_RESULT_FAIL, "%s:%d: EXPECT_MUL_EQ(%d, %q, %d) failed: %d not in [%d..%d]", gTestRunnerState.test->filename, __LINE__, _a, _m, _b, _am, _b-_t, _b+_t); \
    } while (0)

#endif
