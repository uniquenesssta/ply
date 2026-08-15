import QtQuick
import QtQuick.Dialogs

FileDialog {
    id: root

    signal localFileSelected(url sourceUrl)

    title: qsTr("Open media")
    fileMode: FileDialog.OpenFile

    onAccepted: root.localFileSelected(root.selectedFile)
}
