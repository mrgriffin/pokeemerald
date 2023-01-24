#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_POISON_STING].effect == EFFECT_POISON_HIT);
}

SINGLE_BATTLE_TEST("Poison Sting inflicts poison")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_STING); }
    } SCENE {
        STATUS_ICON(opponent, poison: TRUE);
    }
}

SINGLE_BATTLE_TEST("Poison Sting cannot poison Poison- or Steel-types")
{
    u32 species;
    PARAMETRIZE { species = SPECIES_NIDORAN_M; }
    PARAMETRIZE { species = SPECIES_STEELIX; }
    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_NIDORAN_M].type1 == TYPE_POISON);
        ASSUME(gSpeciesInfo[SPECIES_STEELIX].type1 == TYPE_STEEL);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species);
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_STING); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
    }
}
