#ifndef GUARD_DIFFICULTY_H
#define GUARD_DIFFICULTY_H

#include "global.h"

enum DifficultyMode
{
    DIFFICULTY_EASY = 0,
    DIFFICULTY_MEDIUM = 1,
    DIFFICULTY_HARD = 2,
    DIFFICULTY_COUNT,
};

u8 GetDifficultyMode(void);
void SetDifficultyMode(u8 mode);
bool8 IsDifficultyModeValid(u8 mode);
void Difficulty_SanitizeSaveData(void);
u8 Difficulty_AdjustWildLevel(u8 baseLevel);
u8 Difficulty_AdjustTrainerLevel(u8 baseLevel);
u8 Difficulty_AdjustTrainerFixedIV(u8 baseFixedIV);
u16 Difficulty_GetTrainerHeldItem(u16 species, u16 heldItem);

#endif // GUARD_DIFFICULTY_H
