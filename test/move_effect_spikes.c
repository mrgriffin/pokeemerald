#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Spikes deal damage on switchin")
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
        u32 cnt;
        for (cnt = 0; cnt < layers; ++cnt) {
            TURN { MOVE(player, MOVE_SPIKES); }
        }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        HP_BAR(opponent, damage: &damage);
    } THEN {
        EXPECT_EQ(damage, opponent->maxHP / divisor);
        EXPECT(gSideTimers[B_SIDE_OPPONENT].spikesAmount == layers);
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

SINGLE_BATTLE_TEST("Non-Grounded prevents spikes effect.")
{
    enum AirbornReason airbornReason;
    enum GroundedReason groundedReason;
    bool32 shouldDamage;

    PARAMETRIZE { airbornReason = FLYING; groundedReason = NOT_GROUNDED; shouldDamage = FALSE; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = IRON_BALL; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = GRAVITY; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = FLYING; groundedReason = INGRAIN; shouldDamage = TRUE; };

    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = NOT_GROUNDED; shouldDamage = FALSE; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = IRON_BALL; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = GRAVITY; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = LEVITATE; groundedReason = INGRAIN; shouldDamage = TRUE; };

    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = NOT_GROUNDED; shouldDamage = FALSE; };
    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = IRON_BALL; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = MAGNET_RISE; groundedReason = GRAVITY; shouldDamage = TRUE; };
    // We don't test MAGNET_RISE with INGRAIN, because we cannot baton pass both

    PARAMETRIZE { airbornReason = BALLOON; groundedReason = NOT_GROUNDED; shouldDamage = FALSE; };
    PARAMETRIZE { airbornReason = BALLOON; groundedReason = GRAVITY; shouldDamage = TRUE; };
    PARAMETRIZE { airbornReason = BALLOON; groundedReason = INGRAIN; shouldDamage = TRUE;  };
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
        TURN { MOVE(player, MOVE_SPIKES); MOVE(opponent, MOVE_BATON_PASS); SEND_OUT(opponent, 1); }
    } THEN {
        if (shouldDamage)
            EXPECT(opponent->hp < opponent->maxHP);
        else
            EXPECT(opponent->hp == opponent->maxHP);
    }
}
