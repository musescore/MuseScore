import QtQuick

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

    FlatButton {
        text: "Open dialog"
        onClicked: {
            console.log("onClicked")
            itest.openDialog()
        }
    }
}
