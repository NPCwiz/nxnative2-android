# Upstream sync status

The Android fork (this repository) against the original project, [seedds/nxnative](https://github.com/seedds/nxnative). Updated 2026-11-07.

## Current sync

| | Commit |
|---|---|
| Fork `main` before the sync | `d23d7a31` (Merge pull request #1 from RogelioLB/imgbot) |
| Upstream integrated | `5636e7a7` (seedds/nxnative `main`, "A GPU whose access macOS revoked stops the game…") |
| Common base | `e34fe271` ("Fix type checks in complemented jump-table recognition") |
| Upstream commits integrated | 551 |
| Fork commits on top of the base | 10 (the Android port, RogelioLB/nxnative-android) |
| Branch | `sync/upstream-5636e7a` |

### Commits on the branch

1. `dbca4a72` **Merge** — a real three-way merge; only the 17 conflicted files carry hand resolutions (listed in the commit message).
2. `f6772068` Android: build the host code upstream added (gate stubs, cheats, JIT, AV1, guest files).
3. `1075057a` Android: answer the replay interface calls the shared host gained.
4. `28765f8f` Android packer: `extract`'s application argument; Wonder's patch binding; env switches.
5. `91f50350` Android: `host_compile_check.py`.
6. Documentation (this file and the three beside it).

## Why not the earlier integration branch (PR #1)

`integration/upstream-nxnative-2026-11-06` (PR #1) is a merge commit whose tree is an overlay: for every file changed on both sides it kept the fork's version. Git records upstream as merged, so a later merge would never bring those upstream changes back. Compared with upstream on shared code (C, headers, assembly, Python; Android-only files excluded) its tree is missing about 2,900 lines of upstream code, among them `hle_nvn.c` (−791), `hle_svc.c` (−464), `hle_audren.c`, `loader.c` and `hle_thread.c` (−200 to −360 each). This branch replaces it; PR #1 is left as it was (not deleted, not force-pushed).

A real three-way merge conflicts in only 17 files; every other file merged cleanly, so the merged tree differs from upstream in 33 shared code files by about 490 lines, all of them Android portability.

## Classification of the differences

| Class | What | Where |
|---|---|---|
| Imported directly | 534 files that merged cleanly: host services, NVN / NVN2 interface, shader chain, rewriter, game profiles (150, 11 of them Switch 2), tools, tests, docs | the merge |
| Merged with Android modifications | the 17 conflicted files | merge commit message |
| Needed an Android implementation | 10 shared files that use Apple-only APIs; 18 interface functions the Android backends lacked | commits 2 and 3 |
| Changed interface | `build_ipa_from_nsp.extract(…, app)`; profile `patches.py` must declare `BINDING` | commit 4 |
| Kept as before (Metal-only feature, no Android equivalent yet) | per-queue bound state, first-submission prewarm, layer / screen shape, residency report, frame capture, Metal memory paths | `dm/android/gfx_iface_upstream.inc` |
| Needs investigation | everything in [Remaining work](#remaining-work) | |

### Android-specific behaviour chosen in the port

- **Thread slots:** the four keys the rewritten guest code reads (K18, KTLS, KTLR, KGATE) live in Android's x18 slot block. Upstream's new per-thread gate stacks write KGATE per thread; they now go through `dm_slot_set`, which is `pthread_setspecific` on Apple.
- **Gate stubs:** `asm_elf.py` translates Mach-O `.zerofill` so `dm_gate_top` stays page-aligned (each stub reaches its entry with `adrp` + an immediate).
- **Audio pacing:** Android's `DM_AUDIO_PACE=game` now works per audio renderer (upstream made renderers plural).
- **Cheats:** no alias mapping of `main` on Linux, so cheats write data but not code — the same limit upstream has on iOS.
- **JIT (`hle_jit.c`):** an RWX mapping; Linux has no per-thread write/execute switch.
- **AV1 movies:** dav1d (with NEON) instead of VideoToolbox.
- **Frame captures (`DM_NVN_CAPTURE`):** not built into the Android replay; captures written elsewhere with LZFSE cannot be read on Android.

## Verification

Levels as defined in the task: *implemented* (source changed), *build-verified* (compiled or packaged), *runtime-verified* (executed a defined test), *gameplay-verified* (play observed).

| Check | Result | Level |
|---|---|---|
| Android host sources compiled for `aarch64-linux-android30` with the build's flags, Bionic headers, no NDK (`host_compile_check.py`) | `main`: 56 of 60 compile. After the merge, before porting: 54 of 68 (10 shared files broken). Final: 136 of 140, including all 71 dav1d files. The 4 not compiled are the same Android files on both sides; they need NDK-only headers (AAudio, JNI, MediaNDK, bitmap) | build-verified, partly |
| Unresolved symbols across those objects, compared with `main` | 18 new host functions unresolved after the merge; final: none new, except `mavc_bt709`, which is defined in `media_android.c` (one of the 4 files above). No duplicate definitions | build-verified, partly |
| Rewriter on Android's synthetic module (`dm/android/tests/selftest_nso.py`, built with clang; `DM_TARGET=android patch_nso.py`) | output byte-identical to `main`'s rewriter; `# TARGET android`; 18 slot reads as `mov xN, x18`, no `mrs tpidrro_el0` left | runtime-verified (tool) |
| `mergesort` shim: 2,000 random arrays | sorted and stable | runtime-verified (unit) |
| Dispatch queue shim on the build machine: 500 serial items, `dispatch_sync_f` drain, 8 global items | order kept, drain complete, items ran concurrently | runtime-verified (unit) |
| `mavc_bt709`: 4 cases | as specified | runtime-verified (unit) |
| Python: `compileall`, ruff with the repository's rules | no errors; ruff findings identical to `main`'s, one fewer | build-verified |
| Python tests, `dm/tools/tests` + `dm/android/tests` (Linux) | 1,033 pass, 111 fail, 94 skip. Compared test by test with pristine upstream on the same machine: every failure also fails on upstream (macOS tools such as `cp -X`, Xcode, Oodle) except 3, which this branch fixed (`BINDING`, env switches, patch index). Android tests: 11 pass, 3 skip, 0 fail | runtime-verified (tests) |
| Android NDK build, APK, install, launch | done by the repository owner on their own machine and phone (2026-11-06); device and build log not recorded here yet | build-verified |
| Launcher *Self test* on the phone: the synthetic module through the rewritten x18 slots, TLS, the SVC gate (upstream's new per-thread gate stubs) and a second guest thread | **passed** (reported by the repository owner, 2026-11-06) | runtime-verified |
| Original Switch games on an Android device | **not done** | — |

Nothing on this branch is gameplay-verified.

## Remaining work

In order:

1. ~~Build the APK with the NDK~~ and ~~run the self test on a phone~~: done, passed (2026-11-06).
2. Record the device (model, GPU, Android version) and the self test's log lines.
3. Regression-test the four games in `docs/ANDROID_COMPATIBILITY.md`.
4. Per-queue bound state in the Vulkan replay (`mtl_queue_activate`), which upstream added for games that submit on two queues.
5. The Switch 2 work in `docs/ANDROID_SWITCH2_IMPLEMENTATION.md`.

## Reproducing the sync

```sh
git remote add upstream https://github.com/seedds/nxnative
git fetch upstream main
git checkout -b sync/upstream-<commit> origin/main
git merge --no-ff upstream/main          # resolve the conflicts; never take one side wholesale
git clone --depth 1 --filter=blob:none --sparse https://github.com/aosp-mirror/platform_bionic bionic
(cd bionic && git sparse-checkout set libc/include libc/kernel/uapi libc/kernel/android/uapi)
python3 dm/android/tools/host_compile_check.py <old tree> out-old --bionic bionic
python3 dm/android/tools/host_compile_check.py . out-new --bionic bionic
# compare out-*/results.json and out-*/undefined.txt
```
