import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.WasmTest 1.0

Window {
    id: root

    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    MainWindowBridge {
        id: bridge
        window: root
    }

    ToolTipProvider { }

    InteractiveProvider {
        id: interactiveProvider
        topParent: root
    }

    InteractiveTestModel {
        id: itest
    }

    function openSyncDialog() {
        console.log("onClicked openSyncDialog")
        itest.openSyncDialog()
    }

    Column {
        anchors.fill: parent
        spacing: 16

        FlatButton {
            text: "Open async dialog"
            onClicked: {
                console.log("onClicked")
                itest.openAsyncDialog()
            }
        }

        FlatButton {
            text: "Open sync dialog"
            onClicked: {
                //Qt.callLater(root.openSyncDialog)
                root.openSyncDialog()
            }
        }

        FlatButton {
            text: "runLoop"
            onClicked: {
                console.log("onClicked runLoop")
                var ret = itest.runLoop()
                console.log("Proccess ret: ", ret)
            }
        }

        FlatButton {
            text: "exitLoop"
            onClicked: {
                console.log("onClicked exitLoop")
                itest.exitLoop()
            }
        }

        FlatButton {
            text: "runSleep"
            onClicked: {
                console.log("onClicked runSleep")
                var ret = itest.runSleep()
                console.log("Proccess ret: ", ret)
            }
        }

        FlatButton {
            text: "exitSleep"
            onClicked: {
                console.log("onClicked exitSleep")
                itest.exitSleep()
            }
        }
    }
}
