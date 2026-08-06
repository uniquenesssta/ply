# ADR-0004: QML presentation boundaries

## Status

Accepted and represented by the scaffold UI.

## Decision

`App.qml` owns only the QML root, `MainWindow.qml` owns only the application window shell, and `PlayerScreen.qml` composes player features. Video presentation and player chrome are separate feature components.

## Split rule

When a QML file gains a second independently evolving responsibility, the file is moved into a responsibility-named directory together with the new files. No `Old`, `New`, `V2`, `Final`, or copied replacement modules are allowed.
