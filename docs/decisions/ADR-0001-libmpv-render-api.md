# ADR-0001: Use libmpv Render API instead of native-window embedding

## Status

Accepted and frozen by Atomic Task R0-05 on 2026-08-07.

## Context

The product needs video, subtitles, QML controls, HUDs, drawers, dialogs, and animations to participate in one Qt Quick composition. Native child-window embedding through mpv's `wid` option would introduce a separate platform window and a second composition/lifecycle boundary. An external `mpv.exe` process would add process, IPC, focus, input, and shutdown ownership that do not belong in the selected architecture.

libmpv provides a public render API in `<mpv/render.h>` for application-owned graphics surfaces. It is the authoritative embedding boundary for the first release.

## Decision

The production player will:

- link to libmpv in-process;
- use the public libmpv client API for playback commands, properties, and events;
- use the public libmpv Render API for video output;
- create no mpv-owned child window;
- render video into a Qt-owned OpenGL framebuffer and let Qt Quick composite the video with QML content;
- maintain at most one `mpv_render_context` for one active mpv core;
- create the render context before media playback is allowed to create video output.

The following paths are explicitly rejected for the MVP:

- `--wid`, `MPV_FORMAT_INT64` window IDs, HWND child-window embedding, or any equivalent native-window parent path;
- launching `mpv.exe` as the primary playback engine;
- the deprecated `opengl-cb` API;
- copying decoded frames through a CPU image buffer for normal playback;
- maintaining a second hidden renderer as a fallback.

A future renderer change requires a new ADR and a migration task. It must not be added as an automatic runtime fallback.

## Ownership and thread model

### GUI thread

The GUI thread owns:

- the QML object tree;
- the `QQuickFramebufferObject` item;
- UI-facing properties, geometry, visibility, and device-pixel-ratio inputs;
- queued requests that schedule a Qt Quick update.

The GUI thread does not call `mpv_render_*`, access OpenGL resources, or mutate playback-core state.

### Playback thread

The playback thread owns:

- `mpv_handle`;
- normal libmpv client API calls;
- command submission and command replies;
- event draining and property observation;
- playback state reduction;
- the wakeup callback bridge.

The playback thread does not render, access the Qt Quick framebuffer, or wait for the render thread while holding a lock required by libmpv.

### Qt Quick render thread

The render thread owns:

- `QQuickFramebufferObject::Renderer`;
- the FBO and render-only OpenGL state;
- `mpv_render_context` creation, update, rendering, and destruction;
- the OpenGL procedure resolver used by libmpv.

All `mpv_render_*` calls for the active context are serialized on this thread while the same OpenGL context used for creation is current. The render thread does not call normal libmpv client APIs and does not access QML objects directly.

### Synchronization boundary

`QQuickFramebufferObject::Renderer::synchronize()` is the authoritative item-to-renderer synchronization point. It copies the immutable geometry, DPR, visibility, and render-generation inputs needed by the next frame. The item and renderer do not share writable fields across frames.

Playback-to-render coordination uses explicit queued messages, atomics, or narrowly scoped synchronization owned by the render infrastructure. No callback is allowed to take an application lock and then call libmpv.

## Callback contract

The render update callback and the normal libmpv wakeup callback are signal-only bridges.

The render update callback may:

- set an atomic pending-update flag;
- enqueue or request a Qt Quick redraw through a callback target that has first been proven alive.

It may not:

- call `mpv_render_context_update()`;
- call `mpv_render_context_render()`;
- call normal libmpv APIs;
- access QML;
- block;
- free a render context;
- mutate application or playback state.

The wakeup callback may only wake the playback event loop. It does not drain events inside the C callback.

Callback bridge objects must have an explicit alive/shutdown state so a callback racing with teardown becomes a no-op.

## Initialization order

Initialization is ordered as follows:

1. select the Qt Quick OpenGL graphics API before constructing `QGuiApplication`;
2. create and initialize `mpv_handle` on the playback thread;
3. create the Qt Quick framebuffer renderer on the render thread with its OpenGL context current;
4. create `mpv_render_context` with `MPV_RENDER_API_TYPE_OPENGL` and the approved OpenGL procedure resolver;
5. install the render update callback and establish the render-ready state;
6. only after render readiness, allow a media load that can create video output.

Failure at any step produces an explicit dependency, startup, or render error. The application does not silently switch to `wid`, a software renderer, another Qt graphics API, or an external process.

## Frame update flow

1. libmpv invokes the render update callback.
2. The callback schedules a render update without doing rendering itself.
3. On the Qt Quick render thread, the renderer calls `mpv_render_context_update()`.
4. When a frame or redraw is required, the renderer calls `mpv_render_context_render()` with the current Qt FBO, physical-pixel dimensions, and explicit Y-orientation parameters.
5. The renderer restores the OpenGL state expected by Qt Quick before returning.
6. Qt Quick composites the FBO texture with the rest of the QML scene.

The render thread never waits for a normal libmpv client call to finish. The playback thread never waits for a frame while holding a lock needed by the renderer.

## Shutdown and destruction order

Shutdown is an application-level protocol, not incidental destructor order:

1. mark playback and rendering as shutting down and reject new media loads;
2. invalidate the render update callback target and normal wakeup callback target;
3. unregister callbacks where the libmpv API permits and ensure late callbacks observe the invalid state;
4. request scene-graph/render-thread cleanup;
5. wait with a bounded protocol until any active render critical section has exited;
6. with the matching OpenGL context current on the render thread, free `mpv_render_context`;
7. release renderer-owned FBO and OpenGL resources;
8. stop and drain the playback event loop, cancel pending application requests, and destroy `mpv_handle`;
9. only then finish QML/window/application teardown.

`mpv_render_context_free()` must complete before the mpv core is destroyed. No render callback or render operation may reference the context after step 6.

If bounded shutdown cannot prove render quiescence, the failure is logged as a lifecycle error; the implementation must not guess that destruction is safe.

## Consequences

Benefits:

- video and QML overlays are composited in one Qt Quick scene;
- the application owns focus, input, DPI, clipping, transforms, overlays, and fullscreen behavior;
- raw mpv state remains behind the infrastructure adapter;
- the render lifecycle is independently testable.

Costs:

- the first release is tied to Qt Quick's OpenGL path;
- render-thread affinity and callback teardown require explicit coordination;
- OpenGL context loss, resize, minimize/restore, DPI changes, and shutdown races require dedicated tests;
- a future non-OpenGL renderer is a migration, not a small configuration toggle.

## Verification gates

Implementation tasks must prove:

- no production source sets or consumes mpv `wid`;
- no production source uses `opengl-cb`;
- normal libmpv client APIs execute only on the playback thread;
- `mpv_render_*` calls execute only in the serialized render path with the correct current context;
- callbacks contain no rendering, blocking, QML access, or state mutation;
- video cannot start before render-context readiness;
- render-context creation failure is explicit;
- repeated create/render/free/destroy cycles preserve the required order;
- rapid close, minimize/restore, resize, fullscreen, and context-loss paths contain no use-after-free or deadlock.
