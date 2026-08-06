# ADR-0001: Use libmpv as an embedded playback engine

## Status

Accepted for the project baseline.

## Decision

The production player will link to libmpv and use its public client and render APIs. It will not use an external `mpv.exe` process as the primary architecture and will not modify mpv source code for application UI or product workflows.

## Boundary

Application commands, state, persistence, and QML-facing models remain project-owned. Raw mpv properties and C handles remain inside the libmpv infrastructure adapter that will be introduced in R2.
