pragma Singleton

import QtQuick

QtObject {
    readonly property real hidden: OpacityPrimitives.hidden
    readonly property real controlIdle: OpacityPrimitives.idle
    readonly property real controlHover: OpacityPrimitives.visible
    readonly property real controlPressed: OpacityPrimitives.pressed
    readonly property real controlDisabled: OpacityPrimitives.disabled
    readonly property real dialogScrim: OpacityPrimitives.scrim
    readonly property real visible: OpacityPrimitives.visible
}
