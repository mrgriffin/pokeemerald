#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Poison Point inflicts poison on contact")
{
    u32 move;
    PARAMETRIZE { move = MOVE_TACKLE; }
    PARAMETRIZE { move = MOVE_SWIFT; }
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TACKLE].flags & FLAG_MAKES_CONTACT);
        ASSUME(!(gBattleMoves[MOVE_SWIFT].flags & FLAG_MAKES_CONTACT));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_NIDORAN_M) { Ability(ABILITY_POISON_POINT); }
    } WHEN {
        TURN { MOVE(player, move); }
    } THEN {
        if (gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
            STATUS_ICON(player, poison: TRUE);
        else
            NONE_OF { STATUS_ICON(player, poison: TRUE); }
    }
}
