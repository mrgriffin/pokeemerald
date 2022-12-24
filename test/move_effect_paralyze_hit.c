#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_THUNDER_SHOCK].effect == EFFECT_PARALYZE_HIT);
}

SINGLE_BATTLE_TEST("Thunder Shock inflicts paralysis")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_SHOCK); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_PARALYSIS);
    }
}

SINGLE_BATTLE_TEST("Thunder Shock cannot paralyze an Electric-type")
{
    GIVEN {
        ASSUME(B_PARALYZE_ELECTRIC >= GEN_6);
        ASSUME(gSpeciesInfo[SPECIES_PIKACHU].type1 == TYPE_ELECTRIC);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PIKACHU);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_SHOCK); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}
