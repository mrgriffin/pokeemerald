#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Static inflicts paralysis on contact")
{
    u32 move;
    PARAMETRIZE { move = MOVE_TACKLE; }
    PARAMETRIZE { move = MOVE_SWIFT; }
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TACKLE].flags & FLAG_MAKES_CONTACT);
        ASSUME(!(gBattleMoves[MOVE_SWIFT].flags & FLAG_MAKES_CONTACT));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PIKACHU) { Ability(ABILITY_STATIC); }
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        ONLY_IF(gBattleMoves[move].flags & FLAG_MAKES_CONTACT) STATUS_ICON(player, paralysis: TRUE);
    }
}
