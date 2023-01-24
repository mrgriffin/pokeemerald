#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Accuracy controls the proportion of misses")
{
    u32 move;
    PARAMETRIZE { move = MOVE_DYNAMIC_PUNCH; }
    PARAMETRIZE { move = MOVE_THUNDER; }
    PARAMETRIZE { move = MOVE_HYDRO_PUMP; }
    PARAMETRIZE { move = MOVE_RAZOR_LEAF; }
    PARAMETRIZE { move = MOVE_SCRATCH; }
    ASSUME(0 < gBattleMoves[move].accuracy && gBattleMoves[move].accuracy <= 100);
    PASSES_RANDOMLY(gBattleMoves[move].accuracy, 100);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, player);
    }
}

SINGLE_BATTLE_TEST("Secondary Effect Chance controls the proportion of secondary effects")
{
    u32 move;
    PARAMETRIZE { move = MOVE_THUNDER_SHOCK; }
    PARAMETRIZE { move = MOVE_DISCHARGE; }
    PARAMETRIZE { move = MOVE_NUZZLE; }
    ASSUME(gBattleMoves[move].accuracy == 100);
    ASSUME(gBattleMoves[move].effect == EFFECT_PARALYZE_HIT);
    ASSUME(0 < gBattleMoves[move].secondaryEffectChance && gBattleMoves[move].secondaryEffectChance <= 100);
    PASSES_RANDOMLY(gBattleMoves[move].secondaryEffectChance, 100);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        STATUS_ICON(opponent, paralysis: TRUE);
    }
}
