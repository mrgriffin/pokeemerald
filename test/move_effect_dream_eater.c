#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_DREAM_EATER].effect == EFFECT_DREAM_EATER);
}

SINGLE_BATTLE_TEST("Dream Eater recovers 50% of the damage dealt")
{
    s16 damage;
    s16 healed;
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(100); HP(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_SLEEP); }
    } WHEN {
        TURN { MOVE(player, MOVE_DREAM_EATER); }
    } SCENE {
        HP_BAR(opponent, damage: &damage);
        HP_BAR(player, damage: &healed);
    } THEN {
        EXPECT_MUL_EQ(damage, Q_4_12(-1.0/2.0), healed);
    }
}

SINGLE_BATTLE_TEST("Dream Eater fails on awake targets")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DREAM_EATER); }
    } SCENE {
        MESSAGE("Wobbuffet used Dream Eater!");
        MESSAGE("Foe Wobbuffet wasn't affected!");
    }
}
