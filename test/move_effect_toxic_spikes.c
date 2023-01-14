#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Toxic Spikes inflicts poison / toxic poison")
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
        EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == layers);
        EXPECT(opponent[0].status1 & status1);
    }
}

// FIXME: If we have an include directory for tests those should move to a header file
//        with the respective inline functions below
enum AirbornReason
{
    FLYING,
    LEVITATE,
    MAGNET_RISE,
    BALLOON
};

enum GroundedReason
{
    NOT_GROUNDED,
    IRON_BALL,
    GRAVITY,
    INGRAIN,
};

static inline u32 SpeciesFromAirbornReason(enum AirbornReason airbornReason)
{
    if (airbornReason == FLYING)
        return SPECIES_PIDGEOT;
    else if (airbornReason == LEVITATE)
        return SPECIES_UNOWN;
    else
        return SPECIES_WOBBUFFET;
}

static inline u32 GetItemFromReasons(enum AirbornReason airbornReason, enum GroundedReason groundedReason)
{
    if (airbornReason == BALLOON)
        return ITEM_AIR_BALLOON;
    else if (groundedReason == IRON_BALL)
        return ITEM_IRON_BALL;
    return ITEM_NONE;
}

SINGLE_BATTLE_TEST("Non-Grounded prevents toxic spike effect.")
{
    enum AirbornReason airbornReason;
    enum GroundedReason groundedReason;
    u32 status1;

    PARAMETRIZE { airbornReason = FLYING; groundedReason = NOT_GROUNDED; status1 = STATUS1_NONE; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = IRON_BALL; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = GRAVITY; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = INGRAIN; status1 = STATUS1_POISON; };

    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = NOT_GROUNDED; status1 = STATUS1_NONE; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = IRON_BALL; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = GRAVITY; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = INGRAIN; status1 = STATUS1_POISON; };

    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = NOT_GROUNDED; status1 = STATUS1_NONE; };
    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = IRON_BALL; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = GRAVITY; status1 = STATUS1_POISON; };
    // We don't test MAGNET_RISE with INGRAIN, because we cannot baton pass both

    PARAMETRIZE { airbornReason = BALLOON; groundedReason = NOT_GROUNDED; status1 = STATUS1_NONE; };
    PARAMETRIZE { airbornReason = BALLOON; groundedReason = GRAVITY; status1 = STATUS1_POISON; };
    PARAMETRIZE { airbornReason = BALLOON; groundedReason = INGRAIN; status1 = STATUS1_POISON; };
    // We don't test BALLOON with IRON_BALL for the same reason

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); };
        OPPONENT(SPECIES_WOBBUFFET) { Speed(90); };
        OPPONENT(SpeciesFromAirbornReason(airbornReason)) { Item(GetItemFromReasons(airbornReason, groundedReason)); Speed(100); };
    } WHEN {
        if (airbornReason == MAGNET_RISE) {
            TURN { MOVE(opponent, MOVE_MAGNET_RISE); }
        } else if (groundedReason == INGRAIN) {
            TURN { MOVE(opponent, MOVE_INGRAIN); }
        }
        if (groundedReason == GRAVITY) {
            TURN { MOVE(player, MOVE_GRAVITY); }
        }
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } THEN {
        EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == 1);
        EXPECT(opponent->status1 == status1);
    }
}

enum PoisonImmunityReason
{
    STEEL,
    IMMUNITY,
    PASTEL_VEIL
};

SINGLE_BATTLE_TEST("Poison immunity prevents toxic spike effect")
{
    enum PoisonImmunityReason immunityReason;
    
    PARAMETRIZE { immunityReason = STEEL; }
    PARAMETRIZE { immunityReason = IMMUNITY; }
    PARAMETRIZE { immunityReason = PASTEL_VEIL; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        switch (immunityReason) {
        case STEEL:
            OPPONENT(SPECIES_STEELIX);
            break;
        case IMMUNITY:
            OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_IMMUNITY); };
            break;
        case PASTEL_VEIL:
            OPPONENT(SPECIES_PONYTA_GALARIAN) { Ability(ABILITY_PASTEL_VEIL); };
            break;
        }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); }
        TURN { SWITCH(opponent, 1); }
    } THEN {
        EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == 1);
        EXPECT(!(opponent->status1 & STATUS1_POISON));
    }
}

SINGLE_BATTLE_TEST("Grounded poison-type soak toxic spikes")
{
    enum GroundedReason groundedReason;
    bool32 airborne;
    bool32 shouldSoak;

    PARAMETRIZE { airborne = FALSE; groundedReason = NOT_GROUNDED; shouldSoak = TRUE; }
    PARAMETRIZE { airborne = TRUE; groundedReason = NOT_GROUNDED; shouldSoak = FALSE; }
    PARAMETRIZE { airborne = TRUE; groundedReason = IRON_BALL; shouldSoak = TRUE; }
    PARAMETRIZE { airborne = TRUE; groundedReason = GRAVITY; shouldSoak = TRUE; }
    PARAMETRIZE { airborne = TRUE; groundedReason = INGRAIN; shouldSoak = TRUE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); };
        OPPONENT(SPECIES_WOBBUFFET) { Speed(90); };
        OPPONENT(airborne ? SPECIES_ZUBAT : SPECIES_EKANS) { Item(groundedReason == IRON_BALL ? ITEM_IRON_BALL : ITEM_NONE); Speed(100); };
    } WHEN {
        if (groundedReason == GRAVITY) {
            TURN { MOVE(player, MOVE_GRAVITY); }
        }
        if (groundedReason == INGRAIN) {
            TURN { MOVE(opponent, MOVE_INGRAIN); }
        }
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } SCENE {
        if (shouldSoak)
            MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
    } THEN {
        EXPECT(!(opponent->status1 & STATUS1_POISON));
        if (shouldSoak)
            EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == 0);
        else
            EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == 1);
    }
}

// This would test for what I believe to be a bug in the mainline games.
// A pokémon that gets passed magnet rise should still soak the toxic spikes
// even though it is airborne.
// The test currently fails, because we don't incorporate this bug.
/*
SINGLE_BATTLE_TEST("Grounded poison-type soak toxic spikes (Magnet Rise)")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); };
        OPPONENT(SPECIES_WOBBUFFET) { Speed(90); };
        OPPONENT(SPECIES_EKANS) { Speed(100); };
    } WHEN {
        TURN { MOVE(opponent, MOVE_MAGNET_RISE); }
        TURN { MOVE(player, MOVE_TOXIC_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The poison spikes disappeared from around the opposing team's feet!");
    } THEN {
        EXPECT(!(opponent->status1 & STATUS1_POISON));
        EXPECT(gSideTimers[B_SIDE_OPPONENT].toxicSpikesAmount == 0);
    }
}
*/
