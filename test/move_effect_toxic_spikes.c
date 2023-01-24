#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_TOXIC_SPIKES].effect == EFFECT_TOXIC_SPIKES);
}

SINGLE_BATTLE_TEST("Toxic Spikes inflicts poison on switch in")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
        TURN {}
    } SCENE {
        u32 maxHP = GetMonData(&OPPONENT_PARTY[1], MON_DATA_MAX_HP);
        STATUS_ICON(opponent, poison: TRUE);
        HP_BAR(opponent, damage: maxHP / 8);
        HP_BAR(opponent, damage: maxHP / 8);
    }
}

SINGLE_BATTLE_TEST("Toxic Spikes inflicts bad poison on switch in")
{
    u32 layers;
    PARAMETRIZE { layers = 2; }
    PARAMETRIZE { layers = 3; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        u32 count;
        for (count = 0; count < layers; count++)
            TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
        TURN {}
    } SCENE {
        u32 maxHP = GetMonData(&OPPONENT_PARTY[1], MON_DATA_MAX_HP);
        STATUS_ICON(opponent, poison: TRUE);
        HP_BAR(opponent, damage: maxHP / 16 * 1);
        HP_BAR(opponent, damage: maxHP / 16 * 2);
    }
}

SINGLE_BATTLE_TEST("Toxic Spikes inflicts poison on subsequent switch ins")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
        TURN { SWITCH(opponent, 0); }
        TURN {}
    } SCENE {
        u32 maxHP = GetMonData(&OPPONENT_PARTY[0], MON_DATA_MAX_HP);
        MESSAGE("2 sent out Wynaut!");
        STATUS_ICON(opponent, poison: TRUE);
        HP_BAR(opponent, damage: maxHP / 8);
        HP_BAR(opponent, damage: maxHP / 8);
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
    } SCENE {
        if (airborne)
            NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
        else
            STATUS_ICON(opponent, poison: TRUE);
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
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
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
        TURN { SWITCH(opponent, 0); }
    } SCENE {
        if (grounded) {
            NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
            MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
            NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
        } else {
            NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
            ANIMATION(ANIM_TYPE_MOVE, MOVE_BATON_PASS, opponent);
            STATUS_ICON(opponent, poison: TRUE);
        }
    }
}

// This would test for what I believe to be a bug in the mainline games.
// A Pokémon that gets passed magnet rise should still remove the Toxic
// Spikes even though it is airborne.
// The test currently fails, because we don't incorporate this bug.
SINGLE_BATTLE_TEST("Toxic Spikes are removed by Poison-types affected by Magnet Rise")
{
    KNOWN_FAILING;
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EKANS);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MAGNET_RISE); }
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
        TURN { SWITCH(opponent, 0); }
    } SCENE {
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
        MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
        NONE_OF { STATUS_ICON(opponent, poison: TRUE); }
    }
}
