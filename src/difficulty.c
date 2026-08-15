#include "global.h"
#include "difficulty.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "pokemon.h"

static u8 ApplyDifficultyScale(u8 baseLevel)
{
    u32 level = baseLevel;

    switch (GetDifficultyMode())
    {
    case DIFFICULTY_EASY:
        level = (level * 3 + 1) / 4;
        break;
    case DIFFICULTY_HARD:
        level = (level * 13 + 9) / 10;
        break;
    case DIFFICULTY_MEDIUM:
    default:
        break;
    }

    if (level < 1)
        level = 1;
    if (level > 100)
        level = 100;

    return level;
}

static u16 Difficulty_GetTypeHeldItem(u8 type)
{
    switch (type)
    {
    case TYPE_FIRE:
        return ITEM_CHARCOAL;
    case TYPE_WATER:
        return ITEM_MYSTIC_WATER;
    case TYPE_GRASS:
        return ITEM_MIRACLE_SEED;
    case TYPE_ELECTRIC:
        return ITEM_MAGNET;
    case TYPE_ICE:
        return ITEM_NEVER_MELT_ICE;
    case TYPE_FIGHTING:
        return ITEM_BLACK_BELT;
    case TYPE_POISON:
        return ITEM_POISON_BARB;
    case TYPE_GROUND:
        return ITEM_SOFT_SAND;
    case TYPE_FLYING:
        return ITEM_SHARP_BEAK;
    case TYPE_PSYCHIC:
        return ITEM_TWISTED_SPOON;
    case TYPE_BUG:
        return ITEM_SILVER_POWDER;
    case TYPE_ROCK:
        return ITEM_HARD_STONE;
    case TYPE_GHOST:
        return ITEM_SPELL_TAG;
    case TYPE_DRAGON:
        return ITEM_DRAGON_FANG;
    case TYPE_DARK:
        return ITEM_BLACK_GLASSES;
    case TYPE_STEEL:
        return ITEM_METAL_COAT;
    case TYPE_NORMAL:
        return ITEM_SILK_SCARF;
    default:
        return ITEM_LEFTOVERS;
    }
}

bool8 IsDifficultyModeValid(u8 mode)
{
    return mode < DIFFICULTY_COUNT;
}

u8 GetDifficultyMode(void)
{
    if (!IsDifficultyModeValid(gSaveBlock2.difficultyMode))
        return DIFFICULTY_MEDIUM;

    return gSaveBlock2.difficultyMode;
}

void SetDifficultyMode(u8 mode)
{
    if (!IsDifficultyModeValid(mode))
        mode = DIFFICULTY_MEDIUM;

    gSaveBlock2.difficultyMode = mode;
}

void Difficulty_SanitizeSaveData(void)
{
    SetDifficultyMode(gSaveBlock2.difficultyMode);
}

u8 Difficulty_AdjustWildLevel(u8 baseLevel)
{
    return ApplyDifficultyScale(baseLevel);
}

u8 Difficulty_AdjustTrainerLevel(u8 baseLevel)
{
    return ApplyDifficultyScale(baseLevel);
}

u8 Difficulty_AdjustTrainerFixedIV(u8 baseFixedIV)
{
    u32 fixedIV = baseFixedIV;

    switch (GetDifficultyMode())
    {
    case DIFFICULTY_EASY:
        fixedIV = (fixedIV * 3) / 4;
        break;
    case DIFFICULTY_HARD:
        fixedIV = 31;
        break;
    case DIFFICULTY_MEDIUM:
    default:
        break;
    }

    if (fixedIV > 31)
        fixedIV = 31;

    return fixedIV;
}

u16 Difficulty_GetTrainerHeldItem(u16 species, u16 heldItem)
{
    u8 type1;
    u8 type2;

    if (heldItem != ITEM_NONE || GetDifficultyMode() != DIFFICULTY_HARD)
        return heldItem;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return ITEM_LEFTOVERS;

    type1 = gBaseStats[species].type1;
    type2 = gBaseStats[species].type2;

    if (type1 == TYPE_NORMAL)
    {
        if (type2 != TYPE_NORMAL && type2 != TYPE_MYSTERY)
            return Difficulty_GetTypeHeldItem(type2);
        return Difficulty_GetTypeHeldItem(type1);
    }

    if (type1 != TYPE_MYSTERY)
        return Difficulty_GetTypeHeldItem(type1);

    if (type2 != TYPE_MYSTERY)
        return Difficulty_GetTypeHeldItem(type2);

    return ITEM_LEFTOVERS;
}
