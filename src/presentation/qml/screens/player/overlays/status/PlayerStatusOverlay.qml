pragma ComponentBehavior: Bound

import QtQuick
import Player.Presentation.Controls
import Player.Presentation.Feedback

Item {
    id: root

    property var viewModel: null
    property bool suppressBuffering: false

    readonly property string statusKey: root.viewModel !== null
                                        ? root.viewModel.statusKey
                                        : ""
    readonly property int bufferingPercent: root.viewModel !== null
                                            ? root.viewModel.bufferingPercent
                                            : -1
    readonly property bool bufferingSuppressed: root.statusKey === "buffering"
                                                && root.suppressBuffering
    readonly property bool statusVisible: root.viewModel !== null
                                          && root.viewModel.visible
                                          && !root.bufferingSuppressed

    signal openMediaRequested()
    signal openUrlRequested()
    signal cancelMediaOpenRequested()

    objectName: "playerStatusOverlay"
    visible: root.statusVisible

    Loader {
        id: statusLoader
        objectName: "playerStatusLoader"
        anchors.centerIn: parent
        active: root.statusVisible
        sourceComponent: {
            switch (root.statusKey) {
            case "empty":
                return emptyComponent
            case "loading":
                return loadingComponent
            case "buffering":
                return bufferingComponent
            case "ended":
                return endedComponent
            case "error":
                return errorComponent
            default:
                return null
            }
        }
    }

    Component {
        id: emptyComponent

        EmptyFeedback {
            id: emptyFeedback
            objectName: "playerEmptyFeedback"
            title: qsTr("No media")
            detail: qsTr("Open a local file or direct media URL to start playback")
            actionText: qsTr("Open media")

            onActionRequested: root.openMediaRequested()

            TextButton {
                parent: emptyFeedback.contentColumn
                objectName: "playerOpenUrlAction"
                text: qsTr("Open URL")
                onClicked: root.openUrlRequested()
            }
        }
    }

    Component {
        id: loadingComponent

        LoadingFeedback {
            id: loadingFeedback
            objectName: "playerLoadingFeedback"
            title: qsTr("Loading media")
            detail: qsTr("Preparing playback")

            TextButton {
                parent: loadingFeedback.contentColumn
                objectName: "playerCancelMediaOpenAction"
                text: qsTr("Cancel")
                onClicked: root.cancelMediaOpenRequested()
            }
        }
    }

    Component {
        id: bufferingComponent

        BufferingFeedback {
            objectName: "playerBufferingFeedback"
            title: qsTr("Buffering")
            detail: root.bufferingPercent >= 0
                    ? qsTr("%1% buffered").arg(root.bufferingPercent)
                    : qsTr("Waiting for more data")
        }
    }

    Component {
        id: endedComponent

        EndedFeedback {
            objectName: "playerEndedFeedback"
            title: qsTr("Playback finished")
            detail: qsTr("The media has ended")
        }
    }

    Component {
        id: errorComponent

        ErrorFeedback {
            id: errorFeedback
            objectName: "playerErrorFeedback"
            title: qsTr("Playback unavailable")
            detail: qsTr("The media could not be played")
            actionText: qsTr("Open media")

            onActionRequested: root.openMediaRequested()

            TextButton {
                parent: errorFeedback.contentColumn
                objectName: "playerErrorOpenUrlAction"
                text: qsTr("Open URL")
                onClicked: root.openUrlRequested()
            }
        }
    }
}
