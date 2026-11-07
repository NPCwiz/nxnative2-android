# Upstream integration status (2026-11-06)

This is an **experimental integration branch**, not a release. It combines the Android fork with the newer original `seedds/nxnative` code history. **It has not been compiled or tested on an Android device. Do not merge it into `main` solely because GitHub marks the PR mergeable.**

## Source commits
- Android fork: `d23d7a31080cbb82c2bb1c2c34098b27fd6ba83d`
- Upstream nxnative: `5636e7a7cfe67745f2eacedabc02c298290b44ab`
- Shared historical base: `e34fe2719869cc2983aba1b049b386a3e4062825`

## Merge approach
- Start with the upstream snapshot, including new Switch 2 profiles, NUP and dump utilities, and updated core tools.
- Overlay the Android fork's 175 changed files so its Android launcher, Vulkan renderer, Android NDK build, compatibility layer and tests are retained.
- Reconcile a small set of the shared files directly (Android-target rewrite marker, source-cache fingerprinting, Windows filesystem compatibility, shader zstd fallback, Android builder's updated extract argument and README).
- Other files changed on both sides **remain at the Android fork's older version**. This is deliberately conservative and is *not* full conflict resolution; review and port relevant upstream changes before treating the integration as complete.

## Required follow-up: rebase shared Android changes into current upstream
Review in particular:
- `dm/host/dm.h`, `dm/host/loader.c`, `dm/host/hle_*.c`, `dm/host/thread.c` — preserve Android ABI/compatibility while importing upstream Switch 2 SDK and host changes.
- `dm/gfx/nvn/hle_nvn.c`, `dm/gfx/nvn/nvn_obj.h`, `dm/gfx/tools/shader_split.py` — sync newer NVN2/shader functionality without breaking the Vulkan backend.
- `dm/games/game.py`, `dm/tools/patch_nso.py`, `scripts/build_ipa_from_nsp.py` — audit changed function signatures, rewrite metadata and package handling.
- `dm/android/gfx_vk.c`, `dm/android/gfx_vk_draw.inc`, `dm/android/tools/spv_build.py` — implement/test backend functionality needed by newer Switch 2 titles.
- `scripts/build_android_from_nsp.py` and `scripts/nxnative_android.py` — add NUP/dump handling. The Android entry point still assumes NSP/XCI input; copying upstream `pack_nup.py` is **not** sufficient.

## Verification checklist
1. Run a syntax/import scan and the existing Python and C host tests.
2. Build the Android NDK targets for a known original-Switch title.
3. Confirm APK installation, launch, Vulkan rendering, input, save/load, and audio on a real device.
4. Reconcile remaining host and graphics changes and re-run those tests.
5. Add NUP/dump extraction and validate profile selection for Switch 2 titles.
6. Use legally obtained game data and verify Switch 2 startup and real gameplay on Android; don't claim compatibility based only on an APK build.

## Current limitations
- No Android compile, CI, or device verification has been performed for this integration.
- No confirmed Switch 2 gameplay on Android.
- The upstream-reported Mac results for Pokopia do **not** establish Android compatibility.
- Windows on ARM is not a supported game runtime in this Android fork.
