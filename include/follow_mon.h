#ifndef GUARD_FOLLOW_MON_H
#define GUARD_FOLLOW_MON_H

#include "global.h"

#define FOLLOW_MON_HISTORY_LENGTH 8

void FollowMon_Init(void);
void FollowMon_Reset(void);
void FollowMon_OnMapLoad(void);
void FollowMon_OnWarp(void);
void FollowMon_OnPlayerStep(u8 direction, u16 newKeys, u16 heldKeys);
void FollowMon_OnAvatarStateChange(void);

bool8 FollowMon_IsEnabled(void);
void FollowMon_SetEnabled(bool8 enabled);

u16 FollowMon_GetFollowSpecies(void);
u16 FollowMon_GetFollowGraphicsId(u16 species);
bool8 FollowMon_HasSupportedSprite(u16 species);

u8 FollowMon_SpawnFollower(void);
void FollowMon_RemoveFollower(void);
void FollowMon_UpdateFollower(void);

void FollowMon_TickTrail(u8 direction, s16 x, s16 y);
bool8 FollowMon_GetTrailTarget(s16 *x, s16 *y, u8 *direction);

#endif // GUARD_FOLLOW_MON_H
