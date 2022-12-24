#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_TOXIC_SPIKES].effect == EFFECT_TOXIC_SPIKES);
}

SINGLE_BATTLE_TEST("Toxic Spikes inflicts poison/bad poison")
{
    u32 layers;
    u32 status1;
    PARAMETRIZE {layers = 1; status1 = STATUS1_POISON; }
    PARAMETRIZE {layers = 2; status1 = STATUS1_TOXIC_POISON; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        u32 cnt;
        for (cnt = 0; cnt < layers; ++cnt)
            TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
    } THEN {
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount, layers);
        EXPECT(opponent->status1 & status1);
    }
}

SINGLE_BATTLE_TEST("Toxic Spikes do not poison airborne Pokemon")
{
    u32 species = SPECIES_WOBBUFFET;
    u32 item = ITEM_NONE;
    u32 move1 = MOVE_CELEBRATE;
    u32 move2 = MOVE_CELEBRATE;
    bool32 airborne;

    ASSUME(gSpeciesInfo[SPECIES_PIDGEY].type2 == TYPE_FLYING);
    PARAMETRIZE { species = SPECIES_PIDGEY; airborne = TRUE; }
    PARAMETRIZE { species = SPECIES_PIDGEY; item = ITEM_IRON_BALL; airborne = FALSE; }
    PARAMETRIZE { species = SPECIES_PIDGEY; move1 = MOVE_GRAVITY; airborne = FALSE; }
    PARAMETRIZE { species = SPECIES_PIDGEY; move1 = MOVE_INGRAIN; airborne = FALSE; }

    ASSUME(gSpeciesInfo[SPECIES_UNOWN].abilities[0] == ABILITY_LEVITATE);
    PARAMETRIZE { species = SPECIES_UNOWN; airborne = TRUE; }
    PARAMETRIZE { species = SPECIES_UNOWN; item = ITEM_IRON_BALL; airborne = FALSE; }
    PARAMETRIZE { species = SPECIES_UNOWN; move1 = MOVE_GRAVITY; airborne = FALSE; }
    PARAMETRIZE { species = SPECIES_UNOWN; move1 = MOVE_INGRAIN; airborne = FALSE; }

    PARAMETRIZE { move1 = MOVE_MAGNET_RISE; airborne = TRUE; }
    PARAMETRIZE { move1 = MOVE_MAGNET_RISE; item = ITEM_IRON_BALL; airborne = FALSE; }
    PARAMETRIZE { move1 = MOVE_MAGNET_RISE; move2 = MOVE_GRAVITY; airborne = FALSE; }
    // Magnet Rise fails under Gravity.
    // Magnet Rise fails under Ingrain and vice-versa.

    PARAMETRIZE { item = ITEM_AIR_BALLOON; airborne = TRUE; }
    PARAMETRIZE { item = ITEM_AIR_BALLOON; move1 = MOVE_GRAVITY; airborne = FALSE; }
    PARAMETRIZE { item = ITEM_AIR_BALLOON; move1 = MOVE_INGRAIN; airborne = FALSE; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(species) { Item(item); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, move1); }
        TURN { MOVE(opponent, move2); }
        TURN { MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } THEN {
        EXPECT_EQ(opponent->status1, airborne ? STATUS1_NONE : STATUS1_POISON);
    }
}

SINGLE_BATTLE_TEST("Toxic Spikes do not affect Steel-types")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_STEELIX);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        NONE_OF { ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, opponent); }
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Toxic Spikes are removed by grounded Poison-types")
{
    u32 species;
    u32 item = ITEM_NONE;
    u32 move = MOVE_CELEBRATE;
    bool32 grounded;
    PARAMETRIZE { species = SPECIES_EKANS; grounded = TRUE; }
    PARAMETRIZE { species = SPECIES_ZUBAT; grounded = FALSE; }
    PARAMETRIZE { species = SPECIES_ZUBAT; item = ITEM_IRON_BALL; grounded = TRUE; }
    PARAMETRIZE { species = SPECIES_ZUBAT; move = MOVE_GRAVITY; grounded = TRUE; }
    PARAMETRIZE { species = SPECIES_ZUBAT; move = MOVE_INGRAIN; grounded = TRUE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(species) { Item(item); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, move); }
        TURN { MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } SCENE {
        if (grounded)
            MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount, grounded ? 0 : 1);
    }
}

// This would test for what I believe to be a bug in the mainline games.
// A Pokémon that gets passed magnet rise should still remove the Toxic
// Spikes even though it is airborne.
// The test currently fails, because we don't incorporate this bug.
/*
SINGLE_BATTLE_TEST("Toxic Spikes are removed by Poison-types affected by Magnet Rise")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EKANS);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MAGNET_RISE); }
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
    } THEN {
        EXPECT_EQ(opponent->status1, STATUS1_NONE);
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount, 0);
    }
}
*/
