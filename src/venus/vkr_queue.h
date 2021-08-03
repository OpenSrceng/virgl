/*
 * Copyright 2020 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VKR_QUEUE_H
#define VKR_QUEUE_H

#include "vkr_common.h"

struct vkr_queue_sync {
   VkFence fence;

   uint32_t flags;
   void *fence_cookie;

   struct list_head head;
};

struct vkr_queue {
   struct vkr_object base;

   struct vkr_context *context;
   struct vkr_device *device;

   VkDeviceQueueCreateFlags flags;
   uint32_t family;
   uint32_t index;

   /* Submitted fences are added to pending_syncs first.  How submitted fences
    * are retired depends on VKR_RENDERER_THREAD_SYNC and
    * VKR_RENDERER_ASYNC_FENCE_CB.
    *
    * When VKR_RENDERER_THREAD_SYNC is not set, the main thread calls
    * vkGetFenceStatus and retires signaled fences in pending_syncs in order.
    *
    * When VKR_RENDERER_THREAD_SYNC is set but VKR_RENDERER_ASYNC_FENCE_CB is
    * not set, the sync thread calls vkWaitForFences and moves signaled fences
    * from pending_syncs to signaled_syncs in order.  The main thread simply
    * retires all fences in signaled_syncs.
    *
    * When VKR_RENDERER_THREAD_SYNC and VKR_RENDERER_ASYNC_FENCE_CB are both
    * set, the sync thread calls vkWaitForFences and retires signaled fences
    * in pending_syncs in order.
    */
   int eventfd;
   thrd_t thread;
   mtx_t mutex;
   cnd_t cond;
   bool join;
   struct list_head pending_syncs;
   struct list_head signaled_syncs;

   struct list_head busy_head;
};

struct vkr_fence {
   struct vkr_object base;
};

struct vkr_semaphore {
   struct vkr_object base;
};

struct vkr_event {
   struct vkr_object base;
};

void
vkr_context_init_queue_dispatch(struct vkr_context *ctx);

void
vkr_context_init_fence_dispatch(struct vkr_context *ctx);

void
vkr_context_init_semaphore_dispatch(struct vkr_context *ctx);

void
vkr_context_init_event_dispatch(struct vkr_context *ctx);

struct vkr_queue_sync *
vkr_device_alloc_queue_sync(struct vkr_device *dev,
                            uint32_t fence_flags,
                            void *fence_cookie);

void
vkr_device_free_queue_sync(struct vkr_device *dev, struct vkr_queue_sync *sync);

void
vkr_queue_retire_syncs(struct vkr_queue *queue,
                       struct list_head *retired_syncs,
                       bool *queue_empty);

void
vkr_queue_destroy(struct vkr_context *ctx, struct vkr_queue *queue);

#endif /* VKR_QUEUE_H */