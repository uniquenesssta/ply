import QtQuick
import Player.Presentation.Theme

Item {
    id: root

    enum VisibilityState {
        Hidden,
        Rest,
        Active
    }

    property bool autoHideEnabled: true
    property bool playing: false
    property bool scrubbing: false
    property bool controlsHovered: false
    property bool controlsFocused: false
    property bool popupOpen: false
    property bool menuOpen: false
    property bool drawerOpen: false
    property bool modalActive: false
    property bool errorVisible: false
    property bool fullScreen: false

    property int visibilityState: PlayerChromeVisibilityController.Rest

    readonly property bool visibilityLocked: root.isVisibilityLocked()
    readonly property bool chromeVisible: root.visibilityState
                                          !== PlayerChromeVisibilityController.Hidden

    objectName: "playerChromeVisibilityController"

    function isVisibilityLocked() {
        return !root.autoHideEnabled
                || !root.playing
                || root.scrubbing
                || root.controlsHovered
                || root.controlsFocused
                || root.popupOpen
                || root.menuOpen
                || root.drawerOpen
                || root.modalActive
                || root.errorVisible
    }

    function stateName(state) {
        if (state === PlayerChromeVisibilityController.Hidden) {
            return "Hidden"
        }
        if (state === PlayerChromeVisibilityController.Active) {
            return "Active"
        }
        return "Rest"
    }

    function setVisibilityState(nextState, reason) {
        if (root.visibilityState === nextState) {
            return
        }

        root.visibilityState = nextState
        console.info(
            "R6-14 OSC visibility",
            root.stateName(nextState),
            "reason=" + reason,
            "playing=" + root.playing,
            "scrubbing=" + root.scrubbing,
            "controlsHovered=" + root.controlsHovered,
            "controlsFocused=" + root.controlsFocused,
            "popup=" + root.popupOpen,
            "menu=" + root.menuOpen,
            "drawer=" + root.drawerOpen,
            "modal=" + root.modalActive,
            "error=" + root.errorVisible,
            "fullscreen=" + root.fullScreen)
    }

    function scheduleHide() {
        if (root.isVisibilityLocked()) {
            inactivityTimer.stop()
            return
        }
        inactivityTimer.restart()
    }

    function notifyActivity(reason) {
        root.setVisibilityState(
            PlayerChromeVisibilityController.Active,
            reason || "interaction")
        root.scheduleHide()
    }

    function hideIfEligible() {
        if (root.isVisibilityLocked()) {
            root.reevaluatePolicy("timeout-locked")
            return
        }
        root.setVisibilityState(
            PlayerChromeVisibilityController.Hidden,
            "inactivity-timeout")
    }

    function reevaluatePolicy(reason) {
        if (root.isVisibilityLocked()) {
            inactivityTimer.stop()
            if (!root.chromeVisible) {
                root.setVisibilityState(
                    PlayerChromeVisibilityController.Rest,
                    reason || "visibility-lock")
            }
            return
        }

        if (root.chromeVisible) {
            root.scheduleHide()
        }
    }

    function updateVisibilityLock(active, lockReason, releaseReason) {
        if (active) {
            root.notifyActivity(lockReason)
        } else {
            root.reevaluatePolicy(releaseReason)
        }
    }

    Timer {
        id: inactivityTimer
        objectName: "oscInactivityTimer"

        interval: root.fullScreen
                  ? MotionTokens.oscFullscreenHideDelay
                  : MotionTokens.oscHideDelay
        repeat: false

        onTriggered: root.hideIfEligible()
    }

    onPlayingChanged: root.notifyActivity("playback-state")
    onScrubbingChanged: root.updateVisibilityLock(
        root.scrubbing,
        "scrub-lock",
        "scrub-release")
    onControlsHoveredChanged: root.updateVisibilityLock(
        root.controlsHovered,
        "controls-hover-lock",
        "controls-hover-release")
    onControlsFocusedChanged: root.updateVisibilityLock(
        root.controlsFocused,
        "controls-focus-lock",
        "controls-focus-release")
    onPopupOpenChanged: root.updateVisibilityLock(
        root.popupOpen,
        "popup-lock",
        "popup-release")
    onMenuOpenChanged: root.updateVisibilityLock(
        root.menuOpen,
        "menu-lock",
        "menu-release")
    onDrawerOpenChanged: root.updateVisibilityLock(
        root.drawerOpen,
        "drawer-lock",
        "drawer-release")
    onModalActiveChanged: root.updateVisibilityLock(
        root.modalActive,
        "modal-lock",
        "modal-release")
    onErrorVisibleChanged: root.updateVisibilityLock(
        root.errorVisible,
        "error-lock",
        "error-release")
    onFullScreenChanged: root.notifyActivity("window-mode")
    onAutoHideEnabledChanged: root.reevaluatePolicy("auto-hide-policy")

    Component.onCompleted: root.reevaluatePolicy("initial")
}
