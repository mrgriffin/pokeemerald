#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_POWDER_SNOW].effect == EFFECT_FREEZE_HIT);
}

SINGLE_BATTLE_TEST("Powder Snow inflicts freeze")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POWDER_SNOW); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_FREEZE);
    }
}

SINGLE_BATTLE_TEST("Powder Snow cannot freeze an Ice-type")
{
    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_SNORUNT].type1 == TYPE_ICE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_SNORUNT);
    } WHEN {
        TURN { MOVE(player, MOVE_POWDER_SNOW); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}
