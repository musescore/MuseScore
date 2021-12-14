import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "../../"

Row {
    id: root

    property bool windowIsMiximized: false

    property NavigationPanel navigationPanel : NavigationPanel {
        name: "AppControl"
        accessible.name: qsTrc("appshell", "App control")
    }

    signal showWindowMinimizedRequested()
    signal toggleWindowMaximizedRequested()
    signal closeWindowRequested()

    FlatButton {
        id: minimizeButton

        icon: IconCode.APP_MINIMIZE
        transparent: true
        drawFocusBorderInsideRect: true

        navigation.name: "AppControl"
        navigation.panel: root.navigationPanel
        navigation.order: 1
        accessible.name: qsTrc("appshell", "Minimize")

        backgroundItem: ButtonBackground {
            navCtrl: minimizeButton.navigation
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

        navigation.name: "AppControl"
        navigation.panel: root.navigationPanel
        navigation.order: 2
        accessible.name: !root.windowIsMiximized ? qsTrc("appshell", "Maximize") : qsTrc("appshell", "Unmaximize")

        backgroundItem: ButtonBackground {
            navCtrl: maximizeButton.navigation
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

        navigation.name: "AppControl"
        navigation.panel: root.navigationPanel
        navigation.order: 3
        accessible.name: qsTrc("appshell", "Quit")

        backgroundItem: ButtonBackground {
            navCtrl: closeButton.navigation
            mouseArea: closeButton.mouseArea
        }

        onClicked: {
            root.closeWindowRequested()
        }
    }

    component ButtonBackground: Rectangle {
        id: background

        property var navCtrl: null
        property var mouseArea: null

        color: "transparent"

        border.width: ui.theme.borderWidth
        border.color: ui.theme.strokeColor

        NavigationFocusBorder {
            navigationCtrl: background.navCtrl
            drawOutsideParent: false
        }

        states: [
            State {
                name: "PRESSED"
                when: background.mouseArea.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: 1
                }
            },

            State {
                name: "HOVERED"
                when: background.mouseArea.containsMouse && !background.mouseArea.pressed

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: 0.5
                }
            }
        ]
    }
}
