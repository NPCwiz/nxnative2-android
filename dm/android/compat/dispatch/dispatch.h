// Android stand-in for libdispatch: counting semaphores and asynchronous work on a global queue (the host's other dispatch uses are under #ifdef __APPLE__)
#pragma once
#include "dm_android.h"
typedef uint64_t dispatch_time_t;
#define DISPATCH_TIME_NOW 0ull
#define DISPATCH_TIME_FOREVER (~0ull)
typedef struct dm_dispatch_sem { pthread_mutex_t m; pthread_cond_t c; long v; } *dispatch_semaphore_t;
dispatch_time_t dispatch_time(dispatch_time_t when, int64_t delta);    // plat_android.c: CLOCK_MONOTONIC nanoseconds
dispatch_semaphore_t dispatch_semaphore_create(long value);
long dispatch_semaphore_signal(dispatch_semaphore_t s);
long dispatch_semaphore_wait(dispatch_semaphore_t s, dispatch_time_t timeout);   // 0 when taken, non-zero on timeout
void dispatch_release(void *o);
// Queues (no blocks: the NDK has no blocks runtime, so only the _f forms that take a function and its context).
// A global queue (dispatch_get_global_queue) is concurrent: each item runs on a detached thread of its own at the nice level of its QoS (dm_qos_nice); hle_audio.c au_request_start
// starts an output's device this way. A queue from dispatch_queue_create is serial: its items run one at a time, in the order they were queued, on one worker thread the queue
// starts at its first item (hle_cheats.c files_queue). dispatch_sync_f waits until the item has run (behind everything queued before it). Queues are never freed
typedef struct dm_dispatch_queue *dispatch_queue_t;
typedef void (*dispatch_function_t)(void *);
typedef const struct dm_dispatch_queue_attr *dispatch_queue_attr_t;
#define DISPATCH_QUEUE_SERIAL ((dispatch_queue_attr_t)0)
dispatch_queue_t dispatch_get_global_queue(long identifier, unsigned long flags);
dispatch_queue_t dispatch_queue_create(const char *label, dispatch_queue_attr_t attr);
void dispatch_async_f(dispatch_queue_t queue, void *context, dispatch_function_t work);
void dispatch_sync_f(dispatch_queue_t queue, void *context, dispatch_function_t work);
