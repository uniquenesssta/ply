pragma Singleton

import QtQuick

QtObject {
    readonly property string latinFamily: "Inter"
    readonly property string cjkFamily: "Noto Sans SC"
    readonly property string monoFamily: "Geist Mono"

    readonly property string regularStyle: "Regular"
    readonly property string mediumStyle: "Medium"
    readonly property string semiBoldStyle: "Semi Bold"

    readonly property int regularWeight: Font.Normal
    readonly property int mediumWeight: Font.Medium
    readonly property int semiBoldWeight: Font.DemiBold

    readonly property int size10: 10
    readonly property int size11: 11
    readonly property int size12: 12
    readonly property int size13: 13
    readonly property int size14: 14
    readonly property int size22: 22
    readonly property int size32: 32

    readonly property real tracking0: 0
}
