// The replay interface calls the shared host gained from the upstream sync of 2026-11 (seedds/nxnative 5636e7a7) that the Android backends answer the same way:
// included by gfx_vk.c and gfx_null.c, after their own state, so it may use g_presents. Each says what the Metal backend (nvn_mtl.m) does with the call and what
// Android does instead. Most are features the Metal backend has and these backends do not: the call then changes nothing, as before the sync, and the line
// says so; docs/ANDROID_SWITCH2_IMPLEMENTATION.md lists them as open work. mtl_prog_forget is each backend's own (it touches its bindings).

// The present queue's progress (hle_vi.c ListFrameHistory, dm_vi_frame_queued): Metal hands a frame to its present queue and the drawable later. mtl_present here
// hands the frame on before it returns its ticket, so every ticket issued is also done
void mtl_present_progress(uint64_t *queued, uint64_t *done) { *queued = *done = atomic_load(&g_presents); }

// A sized vi layer (hle_vi.c) and a screen taller than 16:9 (loader.c, DM_SCREEN_ASPECT): Metal shapes its drawable by them. The Android front end fits the window
// texture's own crop to the display (front_android_present), so it needs neither; a 1.43:1 DM_SCREEN_ASPECT picture is letterboxed as its texture is. Open
void mtl_present_layer(uint32_t w, uint32_t h) { (void)w; (void)h; }
void mtl_present_screen(uint32_t w, uint32_t h) { (void)w; (void)h; }

// The first submission's prepare step: Metal prewarms its shader libraries and prebuilds pipelines there. The Vulkan backend makes its listed pipelines from the
// 30th present on (gfx_vk_draw.inc prewarm_start), and loads SPIR-V when a program is first drawn with, so there is nothing to do before the first submission
void mtl_prepare(void) {}
void mtl_prog_prefetch(const prog_t *p) { (void)p; }

// Per-queue state: Metal keeps each queue's bound state and restores it when the queue submits (Fire Emblem: Three Houses submits on two queues). These backends
// keep one bound state for every queue, as before the sync. Open: a game whose queues leave different state bound may draw with the other queue's state
void mtl_queue_activate(uint64_t serial) { (void)serial; }
void mtl_queue_forget(uint64_t serial) { (void)serial; }

// Metal-only memory paths: raw global memory reaching an addressed pool, a sparse pool's physical pages going away, and taking uncompressed textures straight
// from their storage unless the GPU wrote it. The Vulkan backend has none of the three, so the calls change nothing
void mtl_pool_addressed(const void *mem, uint64_t size) { (void)mem; (void)size; }
void mtl_sp_phys_gone(const void *mem, uint64_t size) { (void)mem; (void)size; }
void mtl_tex_gpu_written(const tex_t *t) { (void)t; }

// DM_FSTEP_PROBE_RAMP (hle_nvn.c frame step): a probe's grace is renewed while its frames keep getting cheaper, the time the GPU clock takes to rise. Mobile GPUs
// ramp their clocks too, so the default is Metal's (on); 0 turns it off ("ab" is the Mac's A/B mode and reads as on here)
int mtl_probe_ramp_on(void) { static int on = -1; if (on < 0) { const char *e = getenv("DM_FSTEP_PROBE_RAMP"); on = !(e && !strcmp(e, "0")); } return on; }

// Statistics of Metal-only mechanisms (framebuffer snapshots, geometry meshing and hoisting, pool builds): none here
void mtl_fb_stats(uint64_t *snaps, uint64_t *reused, uint64_t *tile, uint64_t *wait) { *snaps = *reused = *tile = *wait = 0; }
void mtl_geomesh_stats(uint64_t *n, uint64_t *wait, uint64_t *run_draws, uint64_t *runs, uint64_t ends[4]) { *n = *wait = *run_draws = *runs = 0; memset(ends, 0, 4 * sizeof *ends); }
void mtl_geohoist_stats(uint64_t *moved, uint64_t *runs, uint64_t *stops) { *moved = *runs = 0; memset(stops, 0, 6 * sizeof *stops); }
void mtl_pool_stats(uint64_t s[6]) { memset(s, 0, 6 * sizeof *s); }

// DM_CTL "restop" (hle_hid.c): what Metal's residency set holds. No residency set here
void dm_gfx_restop(void) { LOG("[RED] DM_CTL restop: the Android replay has no residency set to report"); }

// DM_NVN_CAPTURE (nvn_capture.inc, which nvn_mtl.m includes): a frame capture for the Mac replayer. Not built into the Android backends
void nvn_cap_submit(int n, const uint64_t *handles) {
    (void)n; (void)handles;
    static int once; if (!once && getenv("DM_NVN_CAPTURE")) { once = 1; LOG("[RED] DM_NVN_CAPTURE: frame capture is not built into the Android replay"); }
}
void nvn_cap_present(const tex_t *t, const int crop[4]) { (void)t; (void)crop; }
