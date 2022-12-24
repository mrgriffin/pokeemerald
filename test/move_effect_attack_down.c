#include "global.h"
#include "test_battle.h"

ASSUMPTIONS
{
    ASSUME(gBattleMoves[MOVE_GROWL].effect == EFFECT_ATTACK_DOWN);
}

SINGLE_BATTLE_TEST("Growl lowers Attack", s16 damage)
{
    bool32 lowerAttack;
    PARAMETRIZE { lowerAttack = FALSE; }
    PARAMETRIZE { lowerAttack = TRUE; }
    GIVEN {
        ASSUME(gBattleMoves[MOVE_TACKLE].split == SPLIT_PHYSICAL);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        if (lowerAttack) TURN { MOVE(player, MOVE_GROWL); }
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage);
    }
}
