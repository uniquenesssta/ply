pragma Singleton

import QtQuick

QtObject {
    readonly property var iconIds: [
        "close",
        "fullscreen",
        "next",
        "play",
        "playlist",
        "previous",
        "search",
        "subtitles",
        "volume"
    ]

    function contains(iconId) {
        return iconIds.indexOf(iconId) >= 0
    }

    function sourceFor(iconId) {
        switch (iconId) {
        case "close":
            return Qt.resolvedUrl("assets/icons/close.svg")
        case "fullscreen":
            return Qt.resolvedUrl("assets/icons/fullscreen.svg")
        case "next":
            return Qt.resolvedUrl("assets/icons/next.svg")
        case "play":
            return Qt.resolvedUrl("assets/icons/play.svg")
        case "playlist":
            return Qt.resolvedUrl("assets/icons/playlist.svg")
        case "previous":
            return Qt.resolvedUrl("assets/icons/previous.svg")
        case "search":
            return Qt.resolvedUrl("assets/icons/search.svg")
        case "subtitles":
            return Qt.resolvedUrl("assets/icons/subtitles.svg")
        case "volume":
            return Qt.resolvedUrl("assets/icons/volume.svg")
        default:
            return ""
        }
    }

    function colorRoleFor(iconId) {
        switch (iconId) {
        case "close":
        case "search":
            return "secondary"
        default:
            return "primary"
        }
    }
}
