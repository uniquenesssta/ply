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
    property bool popupOpen: false
    property bool errorVisible: false
    property bool fullScreen: false

    property int visibilityState: PlayerChromeVisibilityController.Rest

    readonly property bool visibilityLocked: !root.autoHideEnabled
                                             || !root.playing
                                             || root.scrubbing
                                             || root.popupOpen
                                             || root.errorVisible
    readonly property bool chromeVisible: root.visibilityState
                                          !== PlayerChromeVisibilityController.Hidden

    objectName: "playerChromeVisibilityController"

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
            "R6-09 OSC visibility",
            root.stateName(nextState),
            "reason=" + reason,
            "playing=" + root.playing,
            "scrubbing=" + root.scrubbing,
            "popup=" + root.popupOpen,
            "error=" + root.errorVisible,
            "fullscreen=" + root.fullScreen)
    }

    function scheduleHide() {
        if (root.visibilityLocked) {
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
        if (root.visibilityLocked) {
            root.reevaluatePolicy("timeout-locked")
            return
        }
        root.setVisibilityState(
            PlayerChromeVisibilityController.Hidden,
            "inactivity-timeout")
    }

    function reevaluatePolicy(reason) {
        if (root.visibilityLocked) {
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

    Timer {
        id: inactivityTimer

        interval: root.fullScreen
                  ? MotionTokens.oscFullscreenHideDelay
                  : MotionTokens.oscHideDelay
        repeat: false

        onTriggered: root.hideIfEligible()
    }

    onPlayingChanged: root.notifyActivity("playback-state")
    onScrubbingChanged: {
        if (root.scrubbing) {
            root.notifyActivity("scrub-lock")
        } else {
            root.reevaluatePolicy("scrub-release")
        }
    }
    onPopupOpenChanged: {
        if (root.popupOpen) {
            root.notifyActivity("popup-lock")
        } else {
            root.reevaluatePolicy("popup-release")
        }
    }
    onErrorVisibleChanged: {
        if (root.errorVisible) {
            root.notifyActivity("error-lock")
        } else {
            root.reevaluatePolicy("error-release")
        }
    }
    onFullScreenChanged: root.notifyActivity("window-mode")
    onAutoHideEnabledChanged: root.reevaluatePolicy("auto-hide-policy")

    Component.onCompleted: root.reevaluatePolicy("initial")
}
