#include "global.h"
#include "test.h"
#include "battle.h"
#include "battle_main.h"
#include "data.h"
#include "malloc.h"
#include "string_util.h"
#include "constants/item.h"
#include "constants/abilities.h"
#include "constants/trainers.h"
#include "constants/battle.h"

static const struct TrainerMonCustomized sTestParty1[] = CUSTOMIZED_PARTY(|
    Bubbles (Wobbuffet) (F) @ Assault Vest
    Level: 67
    Happiness: 42
    Ability: Telepathy
    Shiny: Yes
    IVs: 25 HP / 26 Atk / 27 Def / 29 SpA / 30 SpD / 28 Spe
    EVs: 252 HP / 4 SpA / 252 Spe
    Ball: Master Ball
    Hasty Nature
    - Air Slash
    - Barrier
    - Solar Beam
    - Explosion

    Wobbuffet
    Level: 5
    Ability: Shadow Tag
|);

static const struct TrainerMonNoItemDefaultMoves sTestParty2[] =
{
    {
        .species = SPECIES_WOBBUFFET,
        .lvl = 5,
    },
    {
        .species = SPECIES_WOBBUFFET,
        .lvl = 6,
    }
};

static const struct Trainer sTestTrainer1 =
{
    .trainerName = _("Test1"),
    .party = EVERYTHING_CUSTOMIZED(sTestParty1),
};

static const struct Trainer sTestTrainer2 =
{
    .trainerName = _("Test2"),
    .party = NO_ITEM_DEFAULT_MOVES(sTestParty2),
};

TEST("CreateNPCTrainerPartyForTrainer generates customized Pokémon")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    u8 nickBuffer[20];
    CreateNPCTrainerPartyFromTrainer(testParty, &sTestTrainer1, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(IsMonShiny(&testParty[0]));
    EXPECT(!IsMonShiny(&testParty[1]));

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_POKEBALL, 0), ITEM_MASTER_BALL);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_POKEBALL, 0), ITEM_POKE_BALL);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPECIES, 0), SPECIES_WOBBUFFET);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPECIES, 0), SPECIES_WOBBUFFET);

    EXPECT_EQ(GetMonAbility(&testParty[0]), ABILITY_TELEPATHY);
    EXPECT_EQ(GetMonAbility(&testParty[1]), ABILITY_SHADOW_TAG);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_FRIENDSHIP, 0), 42);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_FRIENDSHIP, 0), 0);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HELD_ITEM, 0), ITEM_ASSAULT_VEST);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_HELD_ITEM, 0), ITEM_NONE);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HP_IV, 0), 25);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_ATK_IV, 0), 26);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_DEF_IV, 0), 27);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPEED_IV, 0), 28);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPATK_IV, 0), 29);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPDEF_IV, 0), 30);

    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_HP_IV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_ATK_IV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_DEF_IV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPEED_IV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPATK_IV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPDEF_IV, 0), 0);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_HP_EV, 0), 252);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_ATK_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_DEF_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPEED_EV, 0), 252);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPATK_EV, 0), 4);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_SPDEF_EV, 0), 0);

    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_HP_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_ATK_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_DEF_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPEED_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPATK_EV, 0), 0);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_SPDEF_EV, 0), 0);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_LEVEL, 0), 67);
    EXPECT_EQ(GetMonData(&testParty[1], MON_DATA_LEVEL, 0), 5);

    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE1, 0), MOVE_AIR_SLASH);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE2, 0), MOVE_BARRIER);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE3, 0), MOVE_SOLAR_BEAM);
    EXPECT_EQ(GetMonData(&testParty[0], MON_DATA_MOVE4, 0), MOVE_EXPLOSION);

    //GetMonData(&testParty[0], MON_DATA_NICKNAME, nickBuffer);
    //EXPECT(StringCompare(nickBuffer, (const u8[]) _("Bubbles")) == 0);

    //GetMonData(&testParty[1], MON_DATA_NICKNAME, nickBuffer);
    //EXPECT(StringCompare(nickBuffer, (const u8[]) _("Wobbuffet")) == 0);

    EXPECT_EQ(GetGenderFromSpeciesAndPersonality(GetMonData(&testParty[0], MON_DATA_SPECIES, 0), testParty[0].box.personality), MON_FEMALE);

    EXPECT_EQ(GetNature(&testParty[0]), NATURE_HASTY);

    Free(testParty);
}

TEST("CreateNPCTrainerPartyForTrainer generates different personalities for different mons")
{
    struct Pokemon *testParty = Alloc(6 * sizeof(struct Pokemon));
    CreateNPCTrainerPartyFromTrainer(testParty, &sTestTrainer2, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT(testParty[0].box.personality != testParty[1].box.personality);
    Free(testParty);
}
