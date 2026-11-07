# Switch 2 games on Android: status

Where the Android port stands on Switch 2 (`Ounce`) games after the November 2026 upstream sync. Levels as in [ANDROID_COMPATIBILITY.md](ANDROID_COMPATIBILITY.md). **No Switch 2 game has been built, started or played on Android.**

## What the sync brought

Upstream's Switch 2 support is mostly in shared code, which the Android host now compiles:

- **SDK detection**: `dm_sdk_ounce()` (loader.c) reads the `sdk` module's spec string; the host branches on it for Switch 2 behaviour (the guest tick rate, the GOB order of textures the game tiles). Newer SDK behaviour the Switch 2 games share, such as SDK 22's thread IDs in the ThreadLocalRegion (thread.c), is keyed on the SDK, not the console.
- **NVN2**: `hle_nvn.c` handles NVN2 control sections (`nvn_ctl_lens`) and sets the Switch 2's order inside a GOB for textures a game tiles itself (`nvn_gob_switch2`, `nvn_deswizzle.h`). These run before the replay backend, so the Vulkan replay receives the same objects as Metal.
- **Profiles**: 11 Switch 2 profiles (`dm/games/04…`), among them Pokémon Pokopia (`0400A3A01EE4C000`), which upstream reports playing on the Mac.
- **Packaging**: upstream's `build_ipa_from_nsp.py` reads NUP packages (`scripts/pack_nup.py`) and extracted dumps (`scripts/pack_dump.py`) besides NSP / XCI.

Level: build only (the shared code compiles for Android; see [UPSTREAM_SYNC_STATUS.md](UPSTREAM_SYNC_STATUS.md)).

## What is not done

| Item | State |
|---|---|
| NUP / extracted-dump input in the Android packer | `build_android_from_nsp.py` still takes NSP / XCI only. Upstream's packer selects `extract_nup` / `extract_dump` per version; the Android script calls `extract` directly. Next step: route through the same selection |
| NVN2 shaders → SPIR-V | not investigated. The Android shader path is the split (`shader_split.py`) then `dm/android/tools/spv_build.py` through the translator; whether NVN2 control sections and newer instructions pass through it is unknown |
| Vulkan features NVN2 games use | not investigated (formats, descriptor counts, compute, synchronisation) |
| Memory | Pokopia's profile reserves about 9.75 GB of address space with a 9 GB pool (`mem_reserve 0x270000000`, `app_pool 0x240000000`, copied from Donkey Kong Bananza). Address space is not the limit on Android; the pool's resident use is. Untested on any phone |
| Per-queue bound state | not in the Vulkan replay (see ANDROID_COMPATIBILITY.md); whether Switch 2 games need it is unknown |

## Pokémon Pokopia milestones

| # | Milestone | State |
|---|---|---|
| 1 | Package format recognized | not attempted (NUP input missing in the Android packer) |
| 2 | Profile loaded | the profile is in the tree; not loaded by an Android build |
| 3 | Modules rewritten for Android | not attempted |
| 4 | Shaders translated to SPIR-V | not attempted |
| 5 | Host and APK built | not attempted |
| 6 | Game process starts | — |
| 7 | Title screen reached | — |
| 8 | Title screen renders correctly | — |
| 9 | Controller input | — |
| 10 | Gameplay | — |
| 11 | Audio, saves, stability | — |

Prerequisite: the original Switch games pass on this branch first ([ANDROID_COMPATIBILITY.md](ANDROID_COMPATIBILITY.md)).
