# ADR-0003: One authoritative playback-state owner

## Status

Accepted; implementation begins in later playback Atomic Tasks.

## Decision

One application-layer playback session will own the authoritative playback snapshot. UI objects may expose projections of that state but must not maintain an independent competing playback truth.

## Consequence

Commands flow toward the playback session; decoded engine events flow back through the reducer/state publisher. QML never calls libmpv directly.
