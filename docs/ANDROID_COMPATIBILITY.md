# Android compatibility

What has been tested on Android, on which build, and how far. Levels: **build** (packed and compiled), **runtime** (the host started and passed a defined test), **gameplay** (play observed and tested). A game is listed as working only at the gameplay level.

## After the November 2026 upstream sync (branch `sync/upstream-5636e7a`)

Nothing has been tested on a phone yet: the sync was done where neither the NDK nor a device was available. Every game below needs re-testing on this branch before the branch is merged. See [UPSTREAM_SYNC_STATUS.md](UPSTREAM_SYNC_STATUS.md) for what was verified instead.

## Regression targets

The games the Android port (RogelioLB/nxnative-android) was brought up on, as [README.md](../README.md) reports them from before the sync. These are the first targets after the sync; they are not claims about this branch.

| Game | Title ID | Before the sync (as reported) | This branch |
|---|---|---|---|
| Super Mario Bros. Wonder 1.2.1 | `010015100B514000` | playable: about 30 fps at render scale 0.8 on a Snapdragon 695 phone | not tested |
| Super Mario 3D World + Bowser's Fury 1.2.2 | `010028600EBDA000` | Bowser's Fury runs with correct colours; one launch stopped at a black loading screen (open) | not tested |
| Danganronpa V3: Killing Harmony Anniversary Edition 1.0.2 | — | runs from a provisional profile: menus, movies, the start of the story; not played through | not tested |
| The Binding of Isaac: Afterbirth+ 1.7.9b with Repentance | `010021C000B6A000` | runs from a provisional profile with its DLC; shaders compiled at run time, captured on the phone | not tested |

What to check for each (record the level reached):

- [ ] packs and installs; shows in the launcher's library
- [ ] the host starts and the guest's code runs (log: modules loaded, the first frame)
- [ ] Vulkan replay renders correctly
- [ ] audio plays
- [ ] controller and touch input
- [ ] saves are written and read back
- [ ] the game closes and opens again

## Known limitations introduced or exposed by the sync

- **Cheats** write data only, never code (no alias mapping of `main` on Linux; as on iOS).
- **Frame captures** (`DM_NVN_CAPTURE`) are not built into the Android replay.
- **AV1 movies** decode with dav1d on the CPU (no hardware AV1 path).
- **Per-queue bound state**: upstream's Metal replay keeps each NVN queue's state; the Vulkan replay keeps one state for all queues, as before. A game that leaves different state bound on two queues may draw wrong.
- **`DM_SCREEN_ASPECT`** pictures (taller than 16:9) are letterboxed as their texture is; the Android front end does not reshape its surface.
