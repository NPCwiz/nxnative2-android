#!/usr/bin/env python3
"""Compile and link-symbol check of the Android host without the NDK, for upstream syncs (docs/BUILD_AND_TEST.md "Checking a sync without the NDK").
Compiles what dm/android/CMakeLists.txt compiles (dmcore: dm/host/*.c, dm/gfx/**/*.c, the Android files, the converted .S; dav1d) with the Android build's
flags, using a host clang (--target=aarch64-linux-android30) and Bionic's own headers from AOSP (git clone https://github.com/aosp-mirror/platform_bionic,
libc/include and libc/kernel/{uapi,android/uapi} are enough). The NDK framework headers (AAudio, MediaNDK, JNI, android/*) are not in that repository: the
four files that include them are reported as not compiled, the same on both sides of a comparison. Then the symbols the objects reference and none defines.

usage: host_compile_check.py REPO OUTDIR --bionic BIONIC_REPO [--game TITLE_ID]
Writes OUTDIR/results.json ({source: {"ok": bool, "errors": [...]}}) and OUTDIR/undefined.txt. Run it on the old and the new tree and compare both files:
a source that compiled before and does not now, or a new undefined name that is not a libc / NDK library function, is what the sync broke."""
import argparse, glob, json, os, subprocess, sys, concurrent.futures as cf

ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
ap.add_argument('repo'); ap.add_argument('out'); ap.add_argument('--bionic', required=True, help='a checkout of aosp-mirror/platform_bionic')
ap.add_argument('--game', default='010015100B514000', help='the profile whose game_profile.h the host is compiled with')
args = ap.parse_args()
repo, out, game = os.path.abspath(args.repo), os.path.abspath(args.out), args.game


DM = f'{repo}/dm'; A = f'{DM}/android'
gen = f'{out}/gen'; os.makedirs(f'{gen}/{game}', exist_ok=True); os.makedirs(f'{gen}/asm', exist_ok=True)
stubs = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'cc_stubs')   # <vulkan/vulkan.h>: the core API only (gfx_vk.c uses no Android-platform Vulkan symbol)

subprocess.run([sys.executable, f'{DM}/games/game.py', 'header', game, f'{gen}/{game}/game_profile.h'], check=True, cwd=repo)
asm = []
for s in (f'{DM}/host/entry.S', f'{DM}/gfx/nvn/nvn_entry.S', f'{DM}/gfx/nvn/nvn_thunks.S'):
    o = f'{gen}/asm/{os.path.basename(s)}'
    subprocess.run([sys.executable, f'{A}/tools/asm_elf.py', s, o], check=True)
    asm.append(o)

host = [f for f in sorted(glob.glob(f'{DM}/host/*.c')) if not f.endswith('hle_movie_avc.c')]
gfx = sorted(glob.glob(f'{DM}/gfx/*.c') + glob.glob(f'{DM}/gfx/nvn/*.c') + glob.glob(f'{DM}/gfx/tex/*.c'))
andr = [f'{A}/{n}' for n in ('plat_android.c', 'coreaudio_aaudio.c', 'media_android.c', 'haptics_android.c', 'apple_stubs.c', 'gfx_vk.c', 'gfx_null.c', 'front_android.c', 'front_cli.c', 'nvn_demo.c')]
inc = [f'{A}/compat', f'{DM}/host', f'{DM}/gfx/nvn/include', f'{DM}/gfx/nvn', f'{DM}/gfx/tex', f'{DM}/third_party/vulkan-headers/include',
       f'{gen}/{game}', f'{DM}/third_party/libvpx', f'{DM}/third_party/stb_vorbis', f'{DM}/third_party/dav1d/include', stubs]
B = os.path.join(os.path.abspath(args.bionic), 'libc')
RES = subprocess.run(['clang', '-print-resource-dir'], capture_output=True, text=True).stdout.strip()
sysinc = ['-nostdinc', '-isystem', f'{RES}/include', '-isystem', f'{B}/include', '-isystem', f'{B}/kernel/uapi/asm-arm64', '-isystem', f'{B}/kernel/uapi', '-isystem', f'{B}/kernel/android/uapi']
base = ['clang', '--target=aarch64-linux-android30', '-c'] + sysinc + [
        '-DDM_ANDROID=1', '-fsigned-char', '-ffixed-x18', '-march=armv8.2-a+lse',
        '-std=gnu11', '-O1', '-fPIC', '-fvisibility=hidden', '-Wall', '-Wno-unused-function', '-Wno-unused-variable', '-Wno-unused-but-set-variable',
        '-Wno-format', '-Wno-deprecated-declarations', '-Wno-gnu-folding-constant', '-Wno-multichar', '-Wno-four-char-constants', '-Wno-unknown-pragmas',
        '-Wno-missing-braces', '-ferror-limit=50', '-fno-sanitize=all']
def one(src):
    o = f'{out}/obj/' + src.replace('/', '_') + '.o'; os.makedirs(os.path.dirname(o), exist_ok=True)
    cmd = base + ['-include', 'dm_android.h'] * src.endswith('.c') + sum((['-I', i] for i in inc), []) + [src, '-o', o]
    if src.endswith('.S'): cmd = [c for c in cmd if c not in ('-std=gnu11',)]
    r = subprocess.run(cmd, capture_output=True, text=True)
    errs = [l.replace(repo + '/', '') for l in r.stderr.splitlines() if ': error:' in l or 'fatal error' in l]
    return os.path.relpath(src, repo), {'ok': r.returncode == 0, 'errors': errs}
D = f'{DM}/third_party/dav1d'
dav1d = []
if os.path.isdir(f'{A}/dav1d_config'):
    dav1d = sorted(f for f in glob.glob(f'{D}/src/*.c') if not f.endswith('_tmpl.c')) + [f'{D}/src/arm/cpu.c'] + sorted(glob.glob(f'{D}/config/tmpl/*.c')) + \
            sorted(f for f in glob.glob(f'{D}/src/arm/64/*.S') if not (f.endswith('_tmpl.S') or f.endswith('/util.S')))
def one_dav1d(src):
    o = f'{out}/obj/' + src.replace('/', '_') + '.o'; os.makedirs(os.path.dirname(o), exist_ok=True)
    cmd = ['clang', '--target=aarch64-linux-android30', '-c'] + sysinc + ['-O2', '-w', '-fvisibility=hidden', '-fomit-frame-pointer', '-fPIC',
           '-I', f'{A}/dav1d_config', '-I', D, '-I', f'{D}/include', '-I', f'{D}/config', src, '-o', o]
    r = subprocess.run(cmd, capture_output=True, text=True)
    return os.path.relpath(src, repo), {'ok': r.returncode == 0, 'errors': [l for l in r.stderr.splitlines() if 'error' in l]}
with cf.ThreadPoolExecutor(os.cpu_count()) as ex:
    res = dict(ex.map(one, host + gfx + andr + asm))
    res.update(dict(ex.map(one_dav1d, dav1d)))
json.dump(res, open(f'{out}/results.json', 'w'), indent=1, sort_keys=True)
bad = {k: v for k, v in res.items() if not v['ok']}
print(f'{len(res)} sources, {len(res) - len(bad)} compiled, {len(bad)} failed')
for k, v in sorted(bad.items()): print(f'  {k}: {len(v["errors"])} errors; first: {v["errors"][0] if v["errors"] else "(no error line)"}'[:300])

# Referenced and defined by none of the objects (dav1d's own references aside): libc / NDK library functions, and anything the build would fail to link
d, u = set(), set()
for o in glob.glob(f'{out}/obj/*.o'):
    if '_third_party_dav1d_' in o: continue
    for l in subprocess.run(['llvm-nm', '--extern-only', o], capture_output=True, text=True).stdout.splitlines():
        p = l.split()
        if len(p) == 2 and p[0] == 'U': u.add(p[1])
        elif len(p) == 3 and p[1] not in 'Uw': d.add(p[2])
open(f'{out}/undefined.txt', 'w').write('\n'.join(sorted(u - d)) + '\n')
print(f'{len(u - d)} names referenced and not defined: {out}/undefined.txt')
