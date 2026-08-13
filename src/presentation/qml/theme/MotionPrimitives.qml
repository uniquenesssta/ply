pragma Singleton

import QtQuick

QtObject {
    readonly property int instant: 0
    readonly property int fast: 120
    readonly property int shortDuration: 160
    readonly property int close: 180
    readonly property int standard: 200
    readonly property int deliberate: 240

    readonly property var easingOutBezier: [0.2, 0.0, 0.0, 1.0, 1.0, 1.0]
    readonly property var easingInBezier: [0.4, 0.0, 1.0, 1.0, 1.0, 1.0]
}
