# Pokemon Sapphire ROM

This repository contains a GBA ROM hack built from the `pokeruby` decompilation base.

It currently includes:

- A global difficulty system with `Easy`, `Medium`, and `Hard`
- A follower Pokemon system for supported overworld sprites
- Map, menu, battle, save, and encounter updates needed to support those features
- A detailed changelog in [ROM_HACK_README.md](ROM_HACK_README.md)

## Build

For setup and toolchain requirements, see [INSTALL.md](INSTALL.md).

## Notes

- `pokeruby` is the active game source tree.
- `pokeheartgold` is only used as a reference for behavior and structure.
- Build outputs are not tracked in Git.

## Acknowledgements

- [Porymap](https://github.com/huderlem/porymap) for the map editor and map-editing workflow used during development.
- [pokeheartgold](https://github.com/pret/pokeheartgold) for behavioral and structural reference when designing the overworld follower system.

The MIT license in this repository applies to the original hack modifications and documentation. Third-party code, tools, assets, and their associated licenses remain subject to their respective upstream terms.
diff --git a/LICENSE b/LICENSE
