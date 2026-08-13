import QtQuick
import QtQuick.Effects
import Player.Presentation.Theme

Item {
    id: root

    property string iconId: ""
    property color color: IconCatalog.colorRoleFor(iconId) === "secondary"
                          ? ColorTokens.iconSecondary
                          : ColorTokens.iconPrimary

    readonly property bool known: IconCatalog.contains(iconId)
    readonly property url source: IconCatalog.sourceFor(iconId)
    readonly property int loadStatus: sourceImage.status
    readonly property bool ready: known && loadStatus === Image.Ready
    readonly property string diagnostic: {
        if (iconId.length === 0) {
            return "Icon id is required"
        }
        if (!known) {
            return "Unknown icon id: " + iconId
        }
        if (loadStatus === Image.Error) {
            return "Failed to load icon asset: " + source
        }
        return ""
    }

    implicitWidth: sourceImage.implicitWidth > 0
                   ? sourceImage.implicitWidth
                   : LayoutTokens.controlIcon
    implicitHeight: sourceImage.implicitHeight > 0
                    ? sourceImage.implicitHeight
                    : LayoutTokens.controlIcon

    Image {
        id: sourceImage

        anchors.fill: parent
        source: root.known ? root.source : ""
        fillMode: Image.PreserveAspectFit
        visible: false
    }

    MultiEffect {
        anchors.fill: sourceImage
        source: sourceImage
        colorization: 1.0
        colorizationColor: root.color
        visible: root.ready
    }
}
