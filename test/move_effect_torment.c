#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_TORMENT].effect == EFFECT_TORMENT);
}

SINGLE_BATTLE_TEST("Torment prevents consecutive move uses")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TORMENT); MOVE(opponent, MOVE_SPLASH); }
        TURN { MOVE(opponent, MOVE_SPLASH, allowed: FALSE); }
    } SCENE {
        MESSAGE("Foe Wobbuffet used Splash!");
        NONE_OF { MESSAGE("Foe Wobbuffet used Splash!"); }
    }
}

SINGLE_BATTLE_TEST("Torment allows non-consecutive move uses")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TORMENT); MOVE(opponent, MOVE_SPLASH); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_SPLASH); }
    } SCENE {
        MESSAGE("Foe Wobbuffet used Splash!");
        MESSAGE("Foe Wobbuffet used Celebrate!");
        MESSAGE("Foe Wobbuffet used Splash!");
    }
}
