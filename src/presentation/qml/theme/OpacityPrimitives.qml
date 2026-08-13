pragma Singleton

import QtQuick

QtObject {
    // Figma interaction opacity is expressed as percentages. QML Item.opacity
    // uses 0.0-1.0, so the translation is normalized here once.
    readonly property real hidden: 0.0
    readonly property real scrim: 0.18
    readonly property real disabled: 0.38
    readonly property real idle: 0.72
    readonly property real pressed: 0.84
    readonly property real visible: 1.0
}
