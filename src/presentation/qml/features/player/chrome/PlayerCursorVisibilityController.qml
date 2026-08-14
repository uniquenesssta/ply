import QtQuick

Item {
    id: root

    property bool playing: false
    property bool oscVisible: true
    property bool scrubbing: false
    property bool popupOpen: false
    property bool errorVisible: false
    property bool cursorHideSuppressed: false

    readonly property bool cursorHidden: root.shouldHideCursor()

    objectName: "playerCursorVisibilityController"

    function shouldHideCursor() {
        return root.playing
                && !root.oscVisible
                && !root.scrubbing
                && !root.popupOpen
                && !root.errorVisible
                && !root.cursorHideSuppressed
    }

    onCursorHiddenChanged: {
        console.info(
            "R6-10 cursor visibility",
            root.cursorHidden ? "Hidden" : "Visible",
            "playing=" + root.playing,
            "oscVisible=" + root.oscVisible,
            "scrubbing=" + root.scrubbing,
            "popup=" + root.popupOpen,
            "error=" + root.errorVisible,
            "suppressed=" + root.cursorHideSuppressed)
    }
}
