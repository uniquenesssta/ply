pragma Singleton

import QtQuick

QtObject {
    readonly property int video: ZOrderPrimitives.z0
    readonly property int atmosphere: ZOrderPrimitives.z10
    readonly property int mediaContent: ZOrderPrimitives.z20
    readonly property int contrastSupport: ZOrderPrimitives.z25
    readonly property int floatingHeader: ZOrderPrimitives.z30
    readonly property int overlay: ZOrderPrimitives.z35
    readonly property int osc: ZOrderPrimitives.z40
    readonly property int inspector: ZOrderPrimitives.z50
    readonly property int popover: ZOrderPrimitives.z60
    readonly property int hud: ZOrderPrimitives.z70
    readonly property int toast: ZOrderPrimitives.z80
    readonly property int dialogScrim: ZOrderPrimitives.z90
    readonly property int dialog: ZOrderPrimitives.z100
}
