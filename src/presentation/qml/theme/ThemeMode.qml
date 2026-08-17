pragma Singleton

import QtQuick

QtObject {
    readonly property int lightMode: 0
    readonly property int darkMode: 1

    property int mode: lightMode
    readonly property bool isDark: mode === darkMode

    function setMode(requestedMode) {
        if (requestedMode !== lightMode && requestedMode !== darkMode) {
            return false
        }
        mode = requestedMode
        return true
    }
}
