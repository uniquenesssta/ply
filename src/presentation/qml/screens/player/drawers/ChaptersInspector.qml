import QtQuick

Item {
    id: root

    property bool opened: false
    property var chapterModel: null
    property var navigationViewModel: null
    property Item backdropSource: null
    property real backdropMappingRevision: 0

    signal closeRequested()

    objectName: "chaptersInspector"

    PlayerInspectorShell {
        anchors.fill: parent
        opened: root.opened
        backdropSource: root.backdropSource
        backdropMappingRevision: root.backdropMappingRevision
        title: qsTr("Chapters")
        subtitle: qsTr("%1 chapters").arg(root.chapterModel !== null
                                           ? root.chapterModel.count
                                           : 0)

        searchContent: [
            ChapterModeIndicator {
                anchors.fill: parent
                count: root.chapterModel !== null ? root.chapterModel.count : 0
            }
        ]

        bodyContent: [
            ChapterContent {
                anchors.fill: parent
                chapterModel: root.chapterModel
                navigationViewModel: root.navigationViewModel
            }
        ]

        footerContent: [
            ChapterFooter {
                anchors.fill: parent
                navigationViewModel: root.navigationViewModel
            }
        ]

        onCloseRequested: root.closeRequested()
    }
}
