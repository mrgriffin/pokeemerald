#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Pastel Veil prevents Poison Sting poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_POISON_STING].effect == EFFECT_POISON_HIT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_STING); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
    }
}

DOUBLE_BATTLE_TEST("Pastel Veil prevents Poison Sting poison on partner")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_POISON_STING].effect == EFFECT_POISON_HIT);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POISON_STING, target: opponentRight); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponentRight, poison: TRUE); }
    }
}

SINGLE_BATTLE_TEST("Pastel Veil immediately cures Mold Breaker poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
        PLAYER(SPECIES_DRILBUR) { Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        STATUS_ICON(opponent, poison: TRUE);
        STATUS_ICON(opponent, none: TRUE);
    }
}

DOUBLE_BATTLE_TEST("Pastel Veil does not cure Mold Breaker poison on partner")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
        PLAYER(SPECIES_DRILBUR) { Ability(ABILITY_MOLD_BREAKER); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_TOXIC, target: opponentRight); }
    } SCENE {
        STATUS_ICON(opponentRight, poison: TRUE);
        NONE_OF { STATUS_ICON(opponentRight, none: TRUE); }
    }
}

SINGLE_BATTLE_TEST("Pastel Veil prevents Toxic bad poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
    }
}

DOUBLE_BATTLE_TEST("Pastel Veil prevents Toxic bad poison on partner")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_TOXIC, target: opponentRight); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponentRight, poison: TRUE); }
    }
}

SINGLE_BATTLE_TEST("Pastel Veil prevents Toxic Spikes poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC_SPIKES].effect == EFFECT_TOXIC_SPIKES);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
    }
}

DOUBLE_BATTLE_TEST("Pastel Veil prevents Toxic Spikes poison on partner")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC_SPIKES].effect == EFFECT_TOXIC_SPIKES);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponentRight, 2); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponentRight, poison: TRUE); }
    }
}
