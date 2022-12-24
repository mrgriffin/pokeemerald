#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Cute Charm inflicts infatuation on contact")
{
    u32 move;
    PARAMETRIZE { move = MOVE_TACKLE; }
    PARAMETRIZE { move = MOVE_SWIFT; }
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TACKLE].flags & FLAG_MAKES_CONTACT);
        ASSUME(!(gBattleMoves[MOVE_SWIFT].flags & FLAG_MAKES_CONTACT));
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); }
        OPPONENT(SPECIES_CLEFAIRY) { Gender(MON_FEMALE); Ability(ABILITY_CUTE_CHARM); }
    } WHEN {
        TURN { MOVE(player, move); }
    } THEN {
        if (gBattleMoves[move].flags & FLAG_MAKES_CONTACT)
            EXPECT(player->status2 & STATUS2_INFATUATED_WITH(B_POSITION_OPPONENT_LEFT));
        else
            EXPECT(!(player->status2 & STATUS2_INFATUATED_WITH(B_POSITION_OPPONENT_LEFT)));
    }
}

SINGLE_BATTLE_TEST("Cute Charm cannot infatuate same gender")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); }
        OPPONENT(SPECIES_CLEFAIRY) { Gender(MON_MALE); Ability(ABILITY_CUTE_CHARM); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } THEN {
        EXPECT(!(player->status2 & STATUS2_INFATUATED_WITH(B_POSITION_OPPONENT_LEFT)));
    }
}
