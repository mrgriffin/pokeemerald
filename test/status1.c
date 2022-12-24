#include "global.h"
#include "test_battle.h"

SINGLE_BATTLE_TEST("Sleep prevents the battler from using a move")
{
    u32 turns;
    PARAMETRIZE { turns = 1; }
    PARAMETRIZE { turns = 2; }
    PARAMETRIZE { turns = 3; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_SLEEP_TURN(turns)); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        for (i = 0; i < turns; i++)
            TURN { MOVE(player, MOVE_SPLASH); }
    } SCENE {
        for (i = 0; i < turns - 1; i++)
            MESSAGE("Wobbuffet is fast asleep.");
        MESSAGE("Wobbuffet woke up!");
        MESSAGE("Wobbuffet used Splash!");
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Poison deals 1/8th damage per turn")
{
    s16 damages[4];
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            TURN {}
    } SCENE {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            HP_BAR(player, damage: &damages[i]);
    } THEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
          EXPECT_MUL_EQ(player->maxHP, Q_4_12(1.0/8.0), damages[i]);
    }
}

SINGLE_BATTLE_TEST("Burn deals 1/16th damage per turn")
{
    s16 damages[4];
    GIVEN {
        ASSUME(B_BURN_DAMAGE >= GEN_LATEST);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            TURN {}
    } SCENE {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            HP_BAR(player, damage: &damages[i]);
    } THEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
          EXPECT_MUL_EQ(player->maxHP, Q_4_12(1.0/16.0), damages[i]);
    }
}

SINGLE_BATTLE_TEST("Burn reduces attack by 50%", s16 damage)
{
    bool32 burned;
    PARAMETRIZE { burned = FALSE; }
    PARAMETRIZE { burned = TRUE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { if (burned) Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(opponent, damage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Freeze has a 20% chance of being thawed")
{
    PASSES_RANDOMLY(20, 100);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_FREEZE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); }
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Freeze is thawed by opponent's Fire-type attacks")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_EMBER].type == TYPE_FIRE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_FREEZE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); MOVE(opponent, MOVE_EMBER); }
    } SCENE {
        MESSAGE("Wobbuffet is frozen solid!");
        MESSAGE("Foe Wobbuffet used Ember!");
        MESSAGE("Wobbuffet was defrosted!");
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Freeze is thawed by user's Flame Wheel")
{
    GIVEN {
        ASSUME(gBattleMoves[MOVE_FLAME_WHEEL].flags & FLAG_THAW_USER);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_FREEZE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_FLAME_WHEEL); }
    } SCENE {
        MESSAGE("Wobbuffet was defrosted by Flame Wheel!");
        MESSAGE("Wobbuffet used Flame Wheel!");
    } THEN {
        EXPECT_EQ(player->status1, STATUS1_NONE);
    }
}

SINGLE_BATTLE_TEST("Paralysis reduces speed by 50%")
{
    u16 playerSpeed;
    bool32 playerFirst;
    PARAMETRIZE { playerSpeed = 98; playerFirst = FALSE; }
    PARAMETRIZE { playerSpeed = 102; playerFirst = TRUE; }
    GIVEN {
        ASSUME(B_PARALYSIS_SPEED >= GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); Speed(playerSpeed); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); MOVE(opponent, MOVE_SPLASH); }
    } SCENE {
        if (playerFirst) {
            ONE_OF {
                MESSAGE("Wobbuffet used Splash!");
                MESSAGE("Wobbuffet is paralyzed! It can't move!");
            }
            MESSAGE("Foe Wobbuffet used Splash!");
        } else {
            MESSAGE("Foe Wobbuffet used Splash!");
            ONE_OF {
                MESSAGE("Wobbuffet used Splash!");
                MESSAGE("Wobbuffet is paralyzed! It can't move!");
            }
        }
    }
}

SINGLE_BATTLE_TEST("Paralysis has a 25% chance of skipping the turn")
{
    PASSES_RANDOMLY(25, 100);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); }
    } SCENE {
        MESSAGE("Wobbuffet is paralyzed! It can't move!");
    }
}

SINGLE_BATTLE_TEST("Bad poison deals 1/16th cumulative damage per turn")
{
    s16 damages[4];
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            TURN {}
    } SCENE {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            HP_BAR(player, damage: &damages[i]);
    } THEN {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            EXPECT_EQ(player->maxHP / 16 * (i + 1), damages[i]);
    }
}

SINGLE_BATTLE_TEST("Bad poison cumulative damage resets on switch")
{
    s16 damages[4];
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_TOXIC_POISON); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
        TURN {}
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
        TURN {}
        TURN {}
    } SCENE {
        for (i = 0; i < ARRAY_COUNT(damages); i++)
            HP_BAR(player, damage: &damages[i]);
    } THEN {
        EXPECT_EQ(damages[0], damages[2]);
        EXPECT_EQ(damages[1], damages[3]);
    }
}
