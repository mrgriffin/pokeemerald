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
        u32 maxHP = GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP);
        HP_BAR(player, damage: maxHP / 2);
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
        u32 maxHP = GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP);
        HP_BAR(player, damage: maxHP / 2);
    }
}

SINGLE_BATTLE_TEST("Jump Kick has no recoil if no target")
{
    KNOWN_FAILING; // #2596.
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_HEALING_WISH); MOVE(player, MOVE_JUMP_KICK, hit: FALSE); SEND_OUT(opponent, 1); }
    } SCENE {
        u32 maxHP = GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP);
        NOT HP_BAR(player, damage: maxHP / 2);
    }
}
