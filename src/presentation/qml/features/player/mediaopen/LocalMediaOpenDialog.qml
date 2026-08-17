import QtQuick
import QtQuick.Dialogs

FileDialog {
    id: root

    signal localFilesSelected(var sourceUrls)

    title: qsTr("Open media")
    fileMode: FileDialog.OpenFiles

    onAccepted: root.localFilesSelected(root.selectedFiles)
}
