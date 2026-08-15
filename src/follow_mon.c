#include "global.h"
#include "follow_mon.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "pokemon.h"
#include "constants/event_objects.h"
#include "constants/species.h"

struct FollowMonTrailEntry
{
    s16 x;
    s16 y;
    u8 direction;
    u8 padding;
};

struct FollowMonState
{
    u16 species;
    u16 graphicsId;
    u8 enabled;
    u8 spriteId;
    u8 trailHead;
    u8 trailCount;
    s16 lastX;
    s16 lastY;
    u8 lastDirection;
    u8 padding;
    struct FollowMonTrailEntry trail[FOLLOW_MON_HISTORY_LENGTH];
};

EWRAM_DATA static struct FollowMonState sFollowMonState = {0};

static bool8 FollowMon_IsPlayerReady(void);
static bool8 FollowMon_IsPlayerFollowable(void);
static void FollowMon_ClearTrail(void);
static void FollowMon_SeedTrail(void);
static void FollowMon_DestroySprite(void);
static void FollowMon_SyncToLeadMon(bool8 forceRespawn);
static void FollowMon_UpdateSpritePosition(struct Sprite *sprite);
static void FollowMon_SpriteCallback(struct Sprite *sprite);
static void FollowMon_PushTrail(s16 x, s16 y, u8 direction);
static const struct FollowMonTrailEntry *FollowMon_GetFollowEntry(void);
static u16 FollowMon_GetLeadMonSpecies(void);

void FollowMon_Init(void)
{
    memset(&sFollowMonState, 0, sizeof(sFollowMonState));
    sFollowMonState.spriteId = MAX_SPRITES;
    sFollowMonState.enabled = TRUE;
}

void FollowMon_Reset(void)
{
    FollowMon_DestroySprite();
    FollowMon_ClearTrail();
    sFollowMonState.species = SPECIES_NONE;
    sFollowMonState.graphicsId = 0xFFFF;
    sFollowMonState.lastX = 0;
    sFollowMonState.lastY = 0;
    sFollowMonState.lastDirection = DIR_SOUTH;
}

void FollowMon_OnWarp(void)
{
    FollowMon_Reset();
}

void FollowMon_OnMapLoad(void)
{
    if (!FollowMon_IsPlayerReady())
    {
        FollowMon_Reset();
        return;
    }

    FollowMon_SyncToLeadMon(TRUE);
}

void FollowMon_OnAvatarStateChange(void)
{
    FollowMon_SyncToLeadMon(FALSE);
}

void FollowMon_OnPlayerStep(u8 direction, u16 newKeys, u16 heldKeys)
{
    (void)direction;
    (void)newKeys;
    (void)heldKeys;

    if (!FollowMon_IsPlayerReady())
        return;

    FollowMon_SyncToLeadMon(FALSE);

    if (!FollowMon_IsEnabled())
    {
        FollowMon_Reset();
        return;
    }

    if (!FollowMon_IsPlayerFollowable())
    {
        FollowMon_Reset();
        return;
    }

    {
        struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
        s16 x = playerObjEvent->currentCoords.x;
        s16 y = playerObjEvent->currentCoords.y;
        u8 facing = GetPlayerFacingDirection();

        if (x != sFollowMonState.lastX || y != sFollowMonState.lastY)
        {
            FollowMon_PushTrail(x, y, facing);
            sFollowMonState.lastX = x;
            sFollowMonState.lastY = y;
            sFollowMonState.lastDirection = facing;
        }
        else if (sFollowMonState.trailCount != 0)
        {
            u8 newestIndex = (sFollowMonState.trailHead + FOLLOW_MON_HISTORY_LENGTH - 1) % FOLLOW_MON_HISTORY_LENGTH;
            sFollowMonState.trail[newestIndex].direction = facing;
            sFollowMonState.lastDirection = facing;
        }
    }
}

bool8 FollowMon_IsEnabled(void)
{
    return sFollowMonState.enabled;
}

void FollowMon_SetEnabled(bool8 enabled)
{
    sFollowMonState.enabled = enabled;
    if (!enabled)
        FollowMon_Reset();
}

u16 FollowMon_GetFollowSpecies(void)
{
    return sFollowMonState.species;
}

u16 FollowMon_GetFollowGraphicsId(u16 species)
{
    switch (species)
    {
    case SPECIES_PIKACHU:
        return OBJ_EVENT_GFX_PIKACHU;
    case SPECIES_ZIGZAGOON:
        return OBJ_EVENT_GFX_ZIGZAGOON;
    case SPECIES_WINGULL:
        return OBJ_EVENT_GFX_WINGULL;
    case SPECIES_SKITTY:
        return OBJ_EVENT_GFX_SKITTY;
    case SPECIES_AZURILL:
        return OBJ_EVENT_GFX_AZURILL;
    case SPECIES_AZUMARILL:
        return OBJ_EVENT_GFX_AZUMARILL;
    case SPECIES_POOCHYENA:
        return OBJ_EVENT_GFX_POOCHYENA;
    case SPECIES_KECLEON:
        return OBJ_EVENT_GFX_KECLEON_1;
    case SPECIES_LATIAS:
        return OBJ_EVENT_GFX_LATIAS;
    case SPECIES_LATIOS:
        return OBJ_EVENT_GFX_LATIOS;
    case SPECIES_KYOGRE:
        return OBJ_EVENT_GFX_KYOGRE_1;
    case SPECIES_GROUDON:
        return OBJ_EVENT_GFX_GROUDON_1;
    case SPECIES_REGIROCK:
        return OBJ_EVENT_GFX_REGIROCK;
    case SPECIES_REGICE:
        return OBJ_EVENT_GFX_REGICE;
    case SPECIES_REGISTEEL:
        return OBJ_EVENT_GFX_REGISTEEL;
    case SPECIES_RAYQUAZA:
        return OBJ_EVENT_GFX_RAYQUAZA;
    default:
        return 0xFFFF;
    }
}

bool8 FollowMon_HasSupportedSprite(u16 species)
{
    return FollowMon_GetFollowGraphicsId(species) != 0xFFFF;
}

u8 FollowMon_SpawnFollower(void)
{
    s16 x;
    s16 y;
    u8 direction;
    u8 spriteId;
    u16 graphicsId;

    if (!FollowMon_IsPlayerReady() || !FollowMon_IsEnabled() || !FollowMon_IsPlayerFollowable())
        return MAX_SPRITES;

    graphicsId = FollowMon_GetFollowGraphicsId(FollowMon_GetLeadMonSpecies());
    if (graphicsId == 0xFFFF)
        return MAX_SPRITES;

    FollowMon_RemoveFollower();
    FollowMon_ClearTrail();

    x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;
    direction = GetPlayerFacingDirection();
    FollowMon_PushTrail(x, y, direction);
    MoveCoords(GetOppositeDirection(direction), &x, &y);
    FollowMon_PushTrail(x, y, direction);

    sFollowMonState.species = FollowMon_GetLeadMonSpecies();
    sFollowMonState.graphicsId = graphicsId;
    sFollowMonState.lastX = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    sFollowMonState.lastY = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;
    sFollowMonState.lastDirection = direction;

    spriteId = AddPseudoObjectEvent(graphicsId, FollowMon_SpriteCallback, 0, 0, 1);
    if (spriteId != MAX_SPRITES)
    {
        struct Sprite *sprite = &gSprites[spriteId];
        sprite->coordOffsetEnabled = TRUE;
        sprite->oam.priority = 2;
        sprite->subpriority = 1;
        sFollowMonState.spriteId = spriteId;
        FollowMon_UpdateSpritePosition(sprite);
        return spriteId;
    }

    sFollowMonState.spriteId = MAX_SPRITES;
    return MAX_SPRITES;
}

void FollowMon_RemoveFollower(void)
{
    FollowMon_DestroySprite();
}

void FollowMon_UpdateFollower(void)
{
    FollowMon_SyncToLeadMon(FALSE);
}

void FollowMon_TickTrail(u8 direction, s16 x, s16 y)
{
    if (direction == DIR_NONE)
        direction = sFollowMonState.lastDirection;

    FollowMon_PushTrail(x, y, direction);
    sFollowMonState.lastX = x;
    sFollowMonState.lastY = y;
    sFollowMonState.lastDirection = direction;
}

bool8 FollowMon_GetTrailTarget(s16 *x, s16 *y, u8 *direction)
{
    const struct FollowMonTrailEntry *entry = FollowMon_GetFollowEntry();
    if (entry == NULL)
        return FALSE;

    *x = entry->x;
    *y = entry->y;
    *direction = entry->direction;
    return TRUE;
}

static bool8 FollowMon_IsPlayerReady(void)
{
    return gPlayerAvatar.flags != 0;
}

static bool8 FollowMon_IsPlayerFollowable(void)
{
    return TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ON_FOOT) != 0;
}

static void FollowMon_ClearTrail(void)
{
    memset(sFollowMonState.trail, 0, sizeof(sFollowMonState.trail));
    sFollowMonState.trailHead = 0;
    sFollowMonState.trailCount = 0;
}

static void FollowMon_SeedTrail(void)
{
    s16 x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    s16 y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;
    u8 direction = GetPlayerFacingDirection();

    FollowMon_ClearTrail();
    FollowMon_PushTrail(x, y, direction);
    MoveCoords(GetOppositeDirection(direction), &x, &y);
    FollowMon_PushTrail(x, y, direction);
    sFollowMonState.lastX = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    sFollowMonState.lastY = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;
    sFollowMonState.lastDirection = direction;
}

static void FollowMon_DestroySprite(void)
{
    if (sFollowMonState.spriteId < MAX_SPRITES)
    {
        DestroySpriteAndFreeResources(&gSprites[sFollowMonState.spriteId]);
        sFollowMonState.spriteId = MAX_SPRITES;
    }
}

static void FollowMon_SyncToLeadMon(bool8 forceRespawn)
{
    u16 species;
    u16 graphicsId;

    if (!FollowMon_IsPlayerReady())
        return;

    if (!FollowMon_IsEnabled() || !FollowMon_IsPlayerFollowable())
    {
        FollowMon_DestroySprite();
        return;
    }

    species = FollowMon_GetLeadMonSpecies();
    graphicsId = FollowMon_GetFollowGraphicsId(species);
    if (species == SPECIES_NONE || graphicsId == 0xFFFF)
    {
        FollowMon_DestroySprite();
        sFollowMonState.species = species;
        sFollowMonState.graphicsId = 0xFFFF;
        return;
    }

    if (forceRespawn || sFollowMonState.spriteId >= MAX_SPRITES || sFollowMonState.species != species || sFollowMonState.graphicsId != graphicsId)
    {
        FollowMon_DestroySprite();
        sFollowMonState.species = species;
        sFollowMonState.graphicsId = graphicsId;
        if (FollowMon_SpawnFollower() == MAX_SPRITES)
            return;
    }
    else if (sFollowMonState.trailCount == 0)
    {
        FollowMon_SeedTrail();
    }
}

static void FollowMon_UpdateSpritePosition(struct Sprite *sprite)
{
    s16 x;
    s16 y;
    u8 direction;

    if (!FollowMon_GetTrailTarget(&x, &y, &direction))
    {
        sprite->invisible = TRUE;
        return;
    }

    sub_8060470(&x, &y, 8, 8);
    sprite->x = x;
    sprite->y = y;
    sprite->invisible = FALSE;
    StartSpriteAnimIfDifferent(sprite, GetFaceDirectionAnimNum(direction));
}

static void FollowMon_SpriteCallback(struct Sprite *sprite)
{
    sprite->coordOffsetEnabled = TRUE;
    sprite->oam.priority = 2;
    sprite->subpriority = 1;
    FollowMon_UpdateSpritePosition(sprite);
}

static void FollowMon_PushTrail(s16 x, s16 y, u8 direction)
{
    struct FollowMonTrailEntry *entry = &sFollowMonState.trail[sFollowMonState.trailHead];

    entry->x = x;
    entry->y = y;
    entry->direction = direction;
    sFollowMonState.trailHead = (sFollowMonState.trailHead + 1) % FOLLOW_MON_HISTORY_LENGTH;
    if (sFollowMonState.trailCount < FOLLOW_MON_HISTORY_LENGTH)
        sFollowMonState.trailCount++;
}

static const struct FollowMonTrailEntry *FollowMon_GetFollowEntry(void)
{
    u8 index;

    if (sFollowMonState.trailCount == 0)
        return NULL;

    if (sFollowMonState.trailCount >= 2)
        index = (sFollowMonState.trailHead + FOLLOW_MON_HISTORY_LENGTH - 2) % FOLLOW_MON_HISTORY_LENGTH;
    else
        index = (sFollowMonState.trailHead + FOLLOW_MON_HISTORY_LENGTH - 1) % FOLLOW_MON_HISTORY_LENGTH;

    return &sFollowMonState.trail[index];
}

static u16 FollowMon_GetLeadMonSpecies(void)
{
    u8 leadMonIndex;

    if (!FollowMon_IsPlayerReady())
        return SPECIES_NONE;

    leadMonIndex = GetLeadMonIndex();
    if (leadMonIndex >= PARTY_SIZE)
        return SPECIES_NONE;

    return GetMonData(&gPlayerParty[leadMonIndex], MON_DATA_SPECIES, NULL);
}
