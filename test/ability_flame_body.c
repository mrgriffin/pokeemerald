#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Flame Body inflicts burn on contact")
{
    u32 move;
    PARAMETRIZE { move = MOVE_TACKLE; }
    PARAMETRIZE { move = MOVE_SWIFT; }
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TACKLE].flags & FLAG_MAKES_CONTACT);
        ASSUME(!(gBattleMoves[MOVE_SWIFT].flags & FLAG_MAKES_CONTACT));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_MAGMAR) { Ability(ABILITY_FLAME_BODY); }
    } WHEN {
        TURN { MOVE(player, move); }
    } THEN {
        if (gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
            EXPECT_EQ(player->status1, STATUS1_BURN);
        else
            EXPECT_EQ(player->status1, STATUS1_NONE);
    }
}
