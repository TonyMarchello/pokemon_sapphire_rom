# Pokemon Ruby ROM Hack Change Log

This document summarizes the ROM-hack changes currently applied in this `pokeruby` workspace and points to the exact files to inspect if you want to port the work into another repo.

Scope:
- All implementation work lives in `pokeruby`.
- `pokeheartgold` was used only as a behavioral reference.
- This document covers both major systems that were added or expanded here:
  - Difficulty selection and scaling
  - Overworld follower Pokemon support

## 1. Difficulty System

### What it does
- Adds a global difficulty mode with three values:
  - `Easy`
  - `Medium`
  - `Hard`
- Lets the player choose difficulty from the main menu / new game flow.
- Persists the selected mode in save data.
- Applies scaling to wild battles, trainer battles, Battle Tower parties, secret-base party copies, and other trainer-style mon creation paths.
- Adds a small difficulty readout to the save screen.

### Core state and API
Inspect these files first:
- [include/global.h](include/global.h)
- [include/difficulty.h](include/difficulty.h)
- [src/difficulty.c](src/difficulty.c)

Relevant data:
- `SaveBlock2` now carries `difficultyMode`.
- `enum DifficultyMode` defines:
  - `DIFFICULTY_EASY`
  - `DIFFICULTY_MEDIUM`
  - `DIFFICULTY_HARD`

Key helper functions:
- `GetDifficultyMode(void)`
- `SetDifficultyMode(u8 mode)`
- `IsDifficultyModeValid(u8 mode)`
- `Difficulty_SanitizeSaveData(void)`
- `Difficulty_AdjustWildLevel(u8 baseLevel)`
- `Difficulty_AdjustTrainerLevel(u8 baseLevel)`
- `Difficulty_AdjustTrainerFixedIV(u8 baseFixedIV)`
- `Difficulty_GetTrainerHeldItem(u16 species, u16 heldItem)`

### Main menu and save flow
Inspect these files:
- [src/main_menu.c](src/main_menu.c)
- [src/new_game.c](src/new_game.c)
- [src/save.c](src/save.c)
- [include/main_menu.h](include/main_menu.h)
- [include/strings.h](include/strings.h)
- [src/strings.c](src/strings.c)

What to look for:
- Main menu difficulty picker entry and persistence logic.
- New game flow that defaults or opens the difficulty picker before gameplay begins.
- Save/load sanitation so invalid difficulty values fall back to `Medium`.
- String entries for:
  - `DIFFICULTY`
  - `EASY`
  - `MEDIUM`
  - `HARD`

### Save screen indicator
Inspect these files:
- [include/save_menu_util.h](include/save_menu_util.h)
- [src/save_menu_util.c](src/save_menu_util.c)

What changed:
- The save window now prints a `DIFFICULTY` row.
- The window height was expanded so the extra row fits cleanly.
- The difficulty label uses the same menu strings as the main menu.

### Battle and encounter scaling
Inspect these files:
- [src/wild_encounter.c](src/wild_encounter.c)
- [src/battle_main.c](src/battle_main.c)
- [src/battle_setup.c](src/battle_setup.c)
- [src/battle_tower.c](src/battle_tower.c)
- [src/pokemon_1.c](src/pokemon_1.c)
- [src/pokemon_2.c](src/pokemon_2.c)

What to look for:
- Wild encounter creation calls now use `Difficulty_AdjustWildLevel(...)`.
- Standard trainer party generation uses `Difficulty_AdjustTrainerLevel(...)`.
- Hard mode increases trainer-style IV floors through `Difficulty_AdjustTrainerFixedIV(...)`.
- Hard mode can also apply fallback held items through `Difficulty_GetTrainerHeldItem(...)` when the source mon has no item.
- Battle Tower and copied-special party sources are included so the difficulty setting behaves globally instead of only in normal trainer battles.

### Practical tuning notes
- Easy is a noticeable downscale.
- Medium is intended to stay close to vanilla.
- Hard currently scales up levels and raises battle readiness through IV and held-item support where the source data allows it.

## 2. Overworld Follower Pokemon

### What it does
- Adds a follower module that can spawn the lead party Pokemon behind the player in the overworld.
- Tracks player movement history so the follower can trail correctly instead of teleporting awkwardly.
- Uses a species-to-object graphics hook map so only supported species spawn as overworld followers.
- Stops at the asset boundary if a species does not have a supported overworld sprite.

### Core module
Inspect these files:
- [include/follow_mon.h](include/follow_mon.h)
- [src/follow_mon.c](src/follow_mon.c)

Key entry points:
- `FollowMon_Init(void)`
- `FollowMon_Reset(void)`
- `FollowMon_OnMapLoad(void)`
- `FollowMon_OnWarp(void)`
- `FollowMon_OnPlayerStep(u8 direction, u16 newKeys, u16 heldKeys)`
- `FollowMon_OnAvatarStateChange(void)`
- `FollowMon_IsEnabled(void)`
- `FollowMon_SetEnabled(bool8 enabled)`
- `FollowMon_GetFollowSpecies(void)`
- `FollowMon_GetFollowGraphicsId(u16 species)`
- `FollowMon_HasSupportedSprite(u16 species)`
- `FollowMon_SpawnFollower(void)`
- `FollowMon_RemoveFollower(void)`
- `FollowMon_UpdateFollower(void)`
- `FollowMon_TickTrail(u8 direction, s16 x, s16 y)`
- `FollowMon_GetTrailTarget(s16 *x, s16 *y, u8 *direction)`

### Species and sprite hook map
The follower module currently resolves these species to overworld graphics IDs:
- Pikachu
- Zigzagoon
- Wingull
- Skitty
- Azurill
- Azumarill
- Poochyena
- Kecleon
- Latias
- Latios
- Kyogre
- Groudon
- Regirock
- Regice
- Registeel
- Rayquaza

If you port this to another repo, `FollowMon_GetFollowGraphicsId(...)` is the exact place to extend.

### Overworld integration points
Inspect these files:
- [src/field_player_avatar.c](src/field_player_avatar.c)
- [src/overworld.c](src/overworld.c)
- [src/load_save.c](src/load_save.c)
- [src/save.c](src/save.c)

What to look for:
- Player movement and avatar-state callbacks that notify the follower module.
- Map load, warp, and save/load transitions that reset or respawn the follower as needed.

### Map and event data used by the follower work
Inspect these files if you want the asset side of the implementation:
- [data/maps/LittlerootTown_KanyesHome/map.json](data/maps/LittlerootTown_KanyesHome/map.json)
- [data/maps/LittlerootTown_KanyesHome/scripts.inc](data/maps/LittlerootTown_KanyesHome/scripts.inc)
- [data/maps/LittlerootTown_KanyesHome/text.inc](data/maps/LittlerootTown_KanyesHome/text.inc)
- [data/layouts/LittlerootTown_KanyesHome/map.bin](data/layouts/LittlerootTown_KanyesHome/map.bin)
- [data/layouts/LittlerootTown_KanyesHome/border.bin](data/layouts/LittlerootTown_KanyesHome/border.bin)

Also inspect the edited map data in:
- `data/maps/*`
- `data/layouts/*`
- `data/event_scripts.s`

Those files contain the map-object and layout plumbing that the follower system and related world edits rely on.

## 3. Files Most Likely Needed When Porting

If you only want the minimal code surface for another repo, start here:
- [include/global.h](include/global.h)
- [include/difficulty.h](include/difficulty.h)
- [src/difficulty.c](src/difficulty.c)
- [include/follow_mon.h](include/follow_mon.h)
- [src/follow_mon.c](src/follow_mon.c)
- [src/main_menu.c](src/main_menu.c)
- [src/new_game.c](src/new_game.c)
- [src/save.c](src/save.c)
- [src/save_menu_util.c](src/save_menu_util.c)
- [src/wild_encounter.c](src/wild_encounter.c)
- [src/battle_main.c](src/battle_main.c)
- [src/battle_setup.c](src/battle_setup.c)
- [src/battle_tower.c](src/battle_tower.c)
- [src/pokemon_1.c](src/pokemon_1.c)
- [src/pokemon_2.c](src/pokemon_2.c)
- [src/field_player_avatar.c](src/field_player_avatar.c)
- [src/overworld.c](src/overworld.c)

## 4. Notes For Another Repo

- The build system in this repo auto-discovers `src/*.c`, so new source files are picked up without manually editing the source list.
- The save data change is a real save-structure addition, so porting requires matching the `SaveBlock2` layout.
- The follower system is asset-limited by species graphics support. Unsupported species should stop at the asset boundary instead of silently falling back to an incorrect sprite.
- I did not compile-verify the ROM in this shell environment, so a target repo should still be built and tested after porting.

