import QtQuick

Item {
    id: root

    property bool playing: false
    property bool oscVisible: true
    property bool scrubbing: false
    property bool dragActive: false
    property bool popupOpen: false
    property bool menuOpen: false
    property bool drawerOpen: false
    property bool modalActive: false
    property bool errorVisible: false
    property bool cursorHideSuppressed: false
    property bool windowActive: true
    property bool pointerInside: false

    readonly property bool cursorHidden: root.shouldHideCursor()

    objectName: "playerCursorVisibilityController"

    QtObject {
        id: cursorState

        property bool restoreVisibleUntilActivity: false
    }

    function shouldHideCursor() {
        return root.playing
                && !root.oscVisible
                && root.windowActive
                && root.pointerInside
                && !cursorState.restoreVisibleUntilActivity
                && !root.scrubbing
                && !root.dragActive
                && !root.popupOpen
                && !root.menuOpen
                && !root.drawerOpen
                && !root.modalActive
                && !root.errorVisible
                && !root.cursorHideSuppressed
    }

    function requireVisibleRestore(reason) {
        cursorState.restoreVisibleUntilActivity = true
        console.info("R6-15 cursor restore", "required", "reason=" + reason)
    }

    function notifyActivity(reason) {
        if (!cursorState.restoreVisibleUntilActivity) {
            return
        }

        cursorState.restoreVisibleUntilActivity = false
        console.info(
            "R6-15 cursor restore",
            "released",
            "reason=" + (reason || "interaction"))
    }

    onWindowActiveChanged: root.requireVisibleRestore(
        root.windowActive ? "window-activated" : "window-deactivated")
    onPointerInsideChanged: root.requireVisibleRestore(
        root.pointerInside ? "pointer-enter" : "pointer-leave")

    onCursorHiddenChanged: {
        console.info(
            "R6-15 cursor visibility",
            root.cursorHidden ? "Hidden" : "Visible",
            "playing=" + root.playing,
            "oscVisible=" + root.oscVisible,
            "windowActive=" + root.windowActive,
            "pointerInside=" + root.pointerInside,
            "scrubbing=" + root.scrubbing,
            "dragActive=" + root.dragActive,
            "popup=" + root.popupOpen,
            "menu=" + root.menuOpen,
            "drawer=" + root.drawerOpen,
            "modal=" + root.modalActive,
            "error=" + root.errorVisible,
            "suppressed=" + root.cursorHideSuppressed)
    }
}
