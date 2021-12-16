import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "../"

Row {
    id: root

    property bool windowIsMiximized: false

    spacing: 8

    signal showWindowMinimizedRequested()
    signal toggleWindowMaximizedRequested()
    signal closeWindowRequested()

    FlatButton {
        id: minimizeButton

        icon: IconCode.APP_MINIMIZE
        transparent: true
        drawFocusBorderInsideRect: true

        backgroundItem: AppButtonBackground {
            mouseArea: minimizeButton.mouseArea
        }

        onClicked: {
            root.showWindowMinimizedRequested()
        }
    }

    FlatButton {
        id: maximizeButton

        icon: !root.windowIsMiximized ? IconCode.APP_MAXIMIZE : IconCode.APP_UNMAXIMIZE
        transparent: true
        drawFocusBorderInsideRect: true

        backgroundItem: AppButtonBackground {
            mouseArea: maximizeButton.mouseArea
        }

        onClicked: {
            root.toggleWindowMaximizedRequested()
        }
    }

    FlatButton {
        id: closeButton

        icon: IconCode.APP_CLOSE
        transparent: true
        drawFocusBorderInsideRect: true

        backgroundItem: AppButtonBackground {
            mouseArea: closeButton.mouseArea
        }

        onClicked: {
            root.closeWindowRequested()
        }
    }
}
