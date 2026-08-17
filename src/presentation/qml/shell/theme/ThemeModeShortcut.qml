import QtQuick
import Player.Presentation.Theme

Shortcut {
    sequence: "Ctrl+Shift+D"
    context: Qt.ApplicationShortcut
    autoRepeat: false

    onActivated: ThemeMode.toggle()
}
