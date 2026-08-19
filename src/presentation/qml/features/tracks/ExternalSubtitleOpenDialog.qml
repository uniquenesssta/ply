import QtQuick
import QtQuick.Dialogs

FileDialog {
    id: root

    property var externalSubtitleLoader: null

    signal loadRejected(string errorKey)

    title: qsTr("Add subtitle")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Subtitle files (*.srt *.ass)")]

    onAccepted: {
        if (root.externalSubtitleLoader !== null
                && root.externalSubtitleLoader.loadLocalSubtitle(root.selectedFile)) {
            return
        }

        root.loadRejected(root.externalSubtitleLoader !== null
                          ? root.externalSubtitleLoader.lastErrorKey
                          : "submission-failed")
    }
}
