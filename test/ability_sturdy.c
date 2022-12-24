#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Sturdy prevents OHKO moves")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_FISSURE].effect == EFFECT_OHKO);
        PLAYER(SPECIES_GEODUDE) { Ability(ABILITY_STURDY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_FISSURE); }
    } SCENE {
        MESSAGE("Foe Wobbuffet used Fissure!");
        ABILITY_POPUP(player);
        MESSAGE("Geodude was protected by Sturdy!");
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

SINGLE_BATTLE_TEST("Sturdy prevents OHKOs")
{
    u16 hp, remainingHP;
    bool32 endures;
    PARAMETRIZE { hp = 99; endures = FALSE; }
    PARAMETRIZE { hp = 100; endures = TRUE; }
    GIVEN {
        PLAYER(SPECIES_GEODUDE) { Ability(ABILITY_STURDY); MaxHP(100); HP(hp); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SEISMIC_TOSS); }
    } SCENE {
        HP_BAR(player, hp: &remainingHP);
        if (endures) {
            ABILITY_POPUP(player);
            MESSAGE("Geodude Endured the hit using Sturdy!");
        }
    } THEN {
        if (endures)
            EXPECT_EQ(remainingHP, 1);
        else
            EXPECT_EQ(remainingHP, 0);
    }
}
