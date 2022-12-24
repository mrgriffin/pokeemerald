#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_REFLECT].effect == EFFECT_REFLECT);
}

SINGLE_BATTLE_TEST("Reflect reduces physical damage", s16 damage)
{
    u32 move;
    PARAMETRIZE { move = MOVE_CELEBRATE; }
    PARAMETRIZE { move = MOVE_REFLECT; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_LT(results[1].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("Reflect applies for 5 turns")
{
    u16 damage[6];
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_REFLECT); MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &damage[0]);
        HP_BAR(player, captureDamage: &damage[1]);
        HP_BAR(player, captureDamage: &damage[2]);
        HP_BAR(player, captureDamage: &damage[3]);
        HP_BAR(player, captureDamage: &damage[4]);
        HP_BAR(player, captureDamage: &damage[5]);
    } THEN {
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.0), damage[1]);
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.0), damage[2]);
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.0), damage[3]);
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.0), damage[4]);
        EXPECT_LT(damage[0], damage[5]);
    }
}

SINGLE_BATTLE_TEST("Reflect fails if already active")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_REFLECT); }
        TURN { MOVE(player, MOVE_REFLECT); }
    } SCENE {
        MESSAGE("Wobbuffet used Reflect!");
        MESSAGE("Wobbuffet used Reflect!");
        MESSAGE("But it failed!");
    }
}
