import QtQuick 2.15

import MuseApi.Controls 1.0

Rectangle {

    id: root

    implicitHeight: 400
    implicitWidth: 400

    color: api.theme.backgroundPrimaryColor

    Component.onCompleted: {
        api.log.info("Component.onCompleted from ext3")
    }

    StyledTextLabel {
        id: label1
        text: "Example 3 Configure"
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
