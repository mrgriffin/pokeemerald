#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_SPIKES].effect == EFFECT_SPIKES);
}

SINGLE_BATTLE_TEST("Spikes damage on switch in")
{
    u32 layers;
    u32 divisor;
    s16 damage;

    PARAMETRIZE { layers = 1; divisor = 8; }
    PARAMETRIZE { layers = 2; divisor = 6; }
    PARAMETRIZE { layers = 3; divisor = 4; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        u32 count;
        for (count = 0; count < layers; ++count) {
            TURN { MOVE(player, MOVE_SPIKES); }
        }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        HP_BAR(opponent, damage: &damage);
    } THEN {
        EXPECT_EQ(damage, opponent->maxHP / divisor);
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].spikesAmount, layers);
    }
}

SINGLE_BATTLE_TEST("Spikes do not damage airborne Pokemon")
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
        TURN { MOVE(player, MOVE_SPIKES); MOVE(opponent, move1); }
        TURN { MOVE(opponent, move2); }
        TURN { MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } THEN {
        if (airborne)
            EXPECT_EQ(opponent->hp, opponent->maxHP);
        else
            EXPECT_NE(opponent->hp, opponent->maxHP);
    }
}
