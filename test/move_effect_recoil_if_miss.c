#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_JUMP_KICK].effect == EFFECT_RECOIL_IF_MISS);
}

SINGLE_BATTLE_TEST("Jump Kick has 50% recoil on miss")
{
    s16 recoil;
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_JUMP_KICK, hit: FALSE); }
    } SCENE {
        HP_BAR(player, captureDamage: &recoil);
    } THEN {
        EXPECT_MUL_EQ(player->maxHP, Q_4_12(0.5), recoil);
    }
}

SINGLE_BATTLE_TEST("Jump Kick has 50% recoil on protect")
{
    s16 recoil;
    GIVEN {
        ASSUME(gBattleMoves[MOVE_JUMP_KICK].flags & FLAG_PROTECT_AFFECTED);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_PROTECT); MOVE(player, MOVE_JUMP_KICK, hit: FALSE); }
    } SCENE {
        HP_BAR(player, captureDamage: &recoil);
    } THEN {
        EXPECT_MUL_EQ(player->maxHP, Q_4_12(0.5), recoil);
    }
}

SINGLE_BATTLE_TEST("Jump Kick has no recoil if no target")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_HEALING_WISH); MOVE(player, MOVE_JUMP_KICK, hit: FALSE); }
    } THEN {
        ASSUME(opponent->hp == 0);
        EXPECT_EQ(player->hp, player->maxHP);
    }
}
