#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
}

SINGLE_BATTLE_TEST("Toxic inflicts bad poison")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(opponent, poison: TRUE);
    }
}

SINGLE_BATTLE_TEST("Toxic cannot miss if used by a Poison-type")
{
    u32 species;
    bool32 hit;
    PARAMETRIZE { species = SPECIES_WOBBUFFET; hit = FALSE; }
    PARAMETRIZE { species = SPECIES_NIDORAN_M; hit = TRUE; }
    GIVEN {
        ASSUME(B_TOXIC_NEVER_MISS >= GEN_6);
        ASSUME(gSpeciesInfo[SPECIES_NIDORAN_M].type1 == TYPE_POISON);
        PLAYER(species);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC, hit: FALSE); }
    } SCENE {
        if (hit)
            ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player);
        else
            NONE_OF { ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player); }
    }
}
