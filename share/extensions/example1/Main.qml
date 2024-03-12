import QtQuick 2.15

import Muse.Controls 1.0

Rectangle {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        api.log.info("Component.onCompleted from ext1")
    }

    StyledTextLabel {
        id: label1
        text: "Main 1"
    }

    FlatButton {
        id: btn1
        anchors.top: label1.bottom

        text: "Click me"

        onClicked: {
            api.interactive.info("Ext 1", "Clicked on Btn1")
        }
    }

}
