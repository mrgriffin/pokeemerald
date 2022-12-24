#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_EXPLOSION].effect == EFFECT_EXPLOSION);
}

SINGLE_BATTLE_TEST("Explosion causes the user to faint")
{
    u16 remainingHP;
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EXPLOSION); }
    } SCENE {
        HP_BAR(player, hp: &remainingHP);
    } THEN {
        EXPECT_EQ(remainingHP, 0);
    }
}

SINGLE_BATTLE_TEST("Explosion causes the user to faint even if it misses")
{
    u16 remainingHP;
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EXPLOSION, hit: FALSE); }
    } SCENE {
        HP_BAR(player, hp: &remainingHP);
    } THEN {
        EXPECT_EQ(remainingHP, 0);
    }
}

SINGLE_BATTLE_TEST("Explosion causes the user to faint even if it has no effect")
{
    u16 remainingHP;
    GIVEN {
        ASSUME(gBattleMoves[MOVE_EXPLOSION].type == TYPE_NORMAL);
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].type1 == TYPE_GHOST);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GASTLY);
    } WHEN {
        TURN { MOVE(player, MOVE_EXPLOSION); }
    } SCENE {
        HP_BAR(player, hp: &remainingHP);
        MESSAGE("It doesn't affect Foe Gastly…");
    } THEN {
        EXPECT_EQ(remainingHP, 0);
    }
}
