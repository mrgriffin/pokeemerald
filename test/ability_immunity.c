#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Immunity prevents Poison Sting poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_POISON_STING].effect == EFFECT_POISON_HIT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_STING); }
    } SCENE {
        NONE_OF { ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, opponent); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Immunity prevents Toxic bad poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC].effect == EFFECT_TOXIC);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        NONE_OF { ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, opponent); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Immunity prevents Toxic Spikes poison")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TOXIC_SPIKES].effect == EFFECT_TOXIC_SPIKES);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        NONE_OF { ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, opponent); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}
