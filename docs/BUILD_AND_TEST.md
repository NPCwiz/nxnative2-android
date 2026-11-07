# Building and testing the Android port

How to build and check this fork's Android host and APK. Packing a game, playing it and the desktop app are in [README.md](../README.md) and [dm/android/README.md](../dm/android/README.md); the shared host's internals are in [development.md](development.md).

## Requirements

- Android SDK with build tools and the NDK (the packer finds them: `python scripts/build_android_from_nsp.py --check`)
- Python 3.10 or later; `capstone`, `lz4`, `fonttools` (the `--check` output lists what is missing)
- .NET SDK (to build `nx_extract` and run the shader translator)
- An ARM64 Android phone with Vulkan 1.1 for anything beyond building
- Your own game files and keys. Nothing from a game goes into this repository

## Building

The packer builds the host for one game profile, packs the game and installs the APK:

```sh
python scripts/build_android_from_nsp.py <NSP folder> -k <prod.keys> [--serial <adb device>] [--no-push]
python scripts/build_android_from_nsp.py --apk-only      # the app with every host built on this computer
```

The host alone, for a profile, with CMake (what the packer runs):

```sh
cmake -G Ninja -S dm/android -B build-android -DGAME=010015100B514000 \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29
ninja -C build-android dmhost_app nxloader
```

Since the November 2026 sync the host also builds dav1d (AV1 movies) from `dm/third_party/dav1d`, with `dm/android/dav1d_config/config.h`.

## Checks without a phone

| Check | Command |
|---|---|
| Python syntax | `python3 -m compileall -q scripts dm` |
| Python lint (the repository's rules) | `ruff check scripts dm/tools dm/gfx/tools dm/games/*.py dm/android/tools` |
| Python tests | `python3 -m pytest -q dm/tools/tests dm/android/tests` (many need macOS; compare with the same run on upstream) |
| Android-specific tests | `python3 -m pytest -q dm/tools/tests/test_android_cli.py dm/tools/tests/test_spv_readonly.py dm/android/tests` |
| Rewriter on the synthetic module | `python3 dm/android/tests/selftest_nso.py clang llvm-objcopy /tmp/main.nso` then `DM_TARGET=android DM_GAME=010015100B514000 python3 dm/tools/patch_nso.py /tmp/main.nso /tmp/patched main`; the output's `main.fixups.tsv` carries `# TARGET android` |
| Host compile + link symbols without the NDK | `python3 dm/android/tools/host_compile_check.py . <out> --bionic <platform_bionic checkout>` |

### Checking a sync without the NDK

`dm/android/tools/host_compile_check.py` compiles what `dm/android/CMakeLists.txt` compiles, with its flags, for `aarch64-linux-android30`, using a host clang and Bionic's headers (`git clone https://github.com/aosp-mirror/platform_bionic`; `libc/include` and `libc/kernel/{uapi,android/uapi}` are enough). It writes `results.json` (each source: compiled or its errors) and `undefined.txt` (names referenced that no object defines). Four Android files need NDK-only headers (AAudio, JNI, MediaNDK, bitmap) and never compile there. Run it on the tree before and after a merge: a source that stops compiling, or a new undefined name that is not a libc or NDK library function, is what the merge broke.

It does not replace an NDK build: it does not link, and it does not check the four files.

## Checks on a phone

With the phone connected over ADB:

1. **Self test**: open the launcher's *Self test*, or `adb shell am start -n org.nxnative.android/.Launcher` and pick it. It runs a synthetic module through the rewritten x18 slots, TLS, the SVC gate and a second guest thread. Pass is shown on screen and logged under the tag `nxnative`.
2. **Logs**: `adb logcat -s nxnative` (the host's `[dm]` lines). `[RED]` lines are things not implemented or wrong.
3. **Crash logs**: `adb logcat -b crash`, and the tombstone with `adb bugreport`.
4. **Graphics diagnostics**: `DM_VK_DUMP=<frame>` or the file `files/dump_now` writes every pass's colour targets to `DM_DUMP_DIR` (`files/dump`); `files/shot_now` saves the next present as `shot_<frame>.png` (see [rendering](reference/rendering.md#android-vulkan-replay)).
5. **A game**: pack it with the script and open it from the launcher; regressions targets are in [ANDROID_COMPATIBILITY.md](ANDROID_COMPATIBILITY.md).

Record each result in [ANDROID_COMPATIBILITY.md](ANDROID_COMPATIBILITY.md) with its level (build, runtime or gameplay verified).
