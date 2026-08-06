# ADR-0002: OpenGL-first Qt Quick rendering

## Status

Accepted for the Windows-first baseline.

## Decision

The application selects the Qt Quick OpenGL graphics API before creating `QGuiApplication`. The first libmpv renderer will target an OpenGL framebuffer integrated with Qt Quick.

## Consequence

Application startup must fail explicitly when the required graphics path cannot be created. Alternative Qt RHI backends are future research, not parallel implementations in the MVP.
