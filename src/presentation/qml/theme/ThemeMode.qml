pragma Singleton

import QtQuick

QtObject {
    enum Mode {
        Light,
        Dark
    }

    property int mode: ThemeMode.Light
    readonly property bool dark: mode === ThemeMode.Dark

    function setMode(requestedMode) {
        if (requestedMode !== ThemeMode.Light
                && requestedMode !== ThemeMode.Dark) {
            return false
        }
        mode = requestedMode
        return true
    }
}
