# ADR-0002: Use OpenGL and QQuickFramebufferObject for the first Qt Quick renderer

## Status

Accepted for the Windows-first MVP and frozen by Atomic Task R0-05 on 2026-08-07.

## Context

Qt Quick in Qt 6 can use multiple graphics APIs, while `QQuickFramebufferObject` functions only when the scene graph uses OpenGL. The task book selects a libmpv OpenGL Render API path so video can be composited with QML controls and overlays.

This is a deliberate first-release constraint. `QQuickFramebufferObject` is not treated as a generic cross-API abstraction.

## Decision

The application must call:

```cpp
QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
```

before constructing `QGuiApplication`.

The first video presentation implementation will use:

```text
MpvVideoItem : QQuickFramebufferObject        # GUI thread
        ↓ synchronize()
MpvVideoRenderer : QQuickFramebufferObject::Renderer
        ↓ mpv_render_context_render()
Qt-owned QOpenGLFramebufferObject             # render thread
        ↓
Qt Quick scene graph composition
```

`MpvVideoItem` and `MpvVideoRenderer` are separate classes with separate responsibilities. They must not be implemented as one class that shares mutable render state across GUI and render threads.

## Item responsibility

The item may own or expose:

- logical width and height;
- visibility;
- device-pixel-ratio-related presentation inputs;
- placeholder/error presentation hooks;
- queued update requests;
- a non-owning, lifecycle-safe connection to the render coordinator.

The item does not own:

- `mpv_render_context`;
- an OpenGL context;
- an FBO;
- OpenGL procedure lookup;
- frame rendering;
- libmpv client commands or properties.

QML never receives a C pointer, FBO identifier, OpenGL object, or raw mpv property.

## Renderer responsibility

The renderer owns:

- creation and recreation of the Qt FBO;
- physical-pixel render dimensions;
- the render-thread side of the libmpv render context;
- render-parameter construction;
- explicit Y-orientation handling;
- render update consumption;
- render-thread cleanup.

The size supplied by Qt to `createFramebufferObject()` is already device-pixel-ratio aware and is the authoritative FBO size. The renderer must not multiply DPR a second time.

The renderer must not assume default or preserved OpenGL state. Before returning from `render()`, it restores the state required by Qt Quick using the approved Qt OpenGL reset mechanism.

## Synchronization

`synchronize()` is the only direct item/renderer data-exchange point. It runs on the render thread while the GUI thread is blocked.

The synchronization payload is a small immutable snapshot, for example:

- item size and visibility;
- current media/render generation;
- clear color or empty-frame state;
- shutdown state;
- whether the FBO must be recreated.

The renderer does not retain a raw pointer to the item for use outside `synchronize()`.

Queued messages may be used when required, but they must preserve thread affinity and object lifetime. Direct cross-thread reads of QML properties or renderer fields are prohibited.

## OpenGL and FBO rules

- The OpenGL context used by `mpv_render_*` is the current Qt Quick render-thread context and is the same context used when the mpv render context was created.
- Rendering targets the FBO currently bound by `QQuickFramebufferObject::Renderer::render()`.
- FBO width and height are physical pixels and remain positive before rendering.
- Y orientation is defined once through libmpv render parameters or the item mirroring policy; double flipping is prohibited.
- The implementation does not bind a native mpv window behind or above the QML scene.
- The render callback schedules work; it never renders directly.
- Context loss or invalid FBO state produces an explicit render failure and controlled recreation or shutdown path.

## Failure policy

The application must fail explicitly when:

- Qt Quick did not start with OpenGL;
- the required OpenGL context or functions are unavailable;
- FBO creation fails;
- the libmpv OpenGL render context cannot be created;
- the graphics context is lost and safe recreation cannot be completed.

There is no automatic fallback to Vulkan, Direct3D, Metal, software frame copies, `wid`, or an external process in the MVP.

## Alternatives not selected

### Native-window embedding

Rejected by ADR-0001 because it creates a separate native composition and lifecycle boundary and prevents the selected unified Qt Quick rendering model.

### QQuickRhiItem or private QRhi integration

Not selected for the MVP. A future cross-API renderer requires a separate feasibility task, public/private API compatibility review, performance evidence, and a replacement ADR.

### QQuickRenderControl

Not selected because the application is not rendering an offscreen Qt Quick scene into an externally owned presentation system. Qt Quick remains the top-level compositor.

### CPU frame extraction and texture upload

Rejected for normal playback because it adds copies and bypasses libmpv's intended embedded rendering path.

## Lifecycle relationship

The renderer and FBO are render-thread resources. Their cleanup participates in the shutdown protocol defined by ADR-0001:

1. stop new rendering work and invalidate callback targets;
2. wait for render quiescence;
3. free the libmpv render context while the matching OpenGL context is current;
4. release renderer/FBO resources;
5. destroy the mpv core afterward.

Scene-graph invalidation, window destruction, and application exit must not destroy the required OpenGL context before render-context cleanup has completed.

## Consequences

- Qt Quick must stay on OpenGL for the first release.
- The current `GraphicsBackendBootstrap` ordering is an architectural requirement, not an implementation convenience.
- Driver/OpenGL capability failures must be diagnosable before presenting a permanently black player window.
- Renderer implementation and tests belong in the dedicated render infrastructure and presentation video modules, not in `PlayerScreen.qml`.
- Future platform work may replace this path only through a separately accepted ADR.

## Verification gates

Implementation tasks must verify:

- OpenGL selection occurs before `QGuiApplication`;
- the actual Qt Quick graphics API is OpenGL at runtime;
- item and renderer thread-affinity assertions hold;
- DPR and resize recreate the FBO at the expected physical size;
- frame orientation is correct and not double-flipped;
- OpenGL state is restored before returning to Qt Quick;
- minimize/restore and visibility changes do not create a render storm;
- unsupported graphics environments fail clearly instead of displaying a silent black viewport;
- shutdown releases the render context before the Qt OpenGL context and mpv core disappear.
