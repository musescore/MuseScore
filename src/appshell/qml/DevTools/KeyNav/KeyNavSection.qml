import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0


Rectangle {
    id: root

    property alias sectionName: keynavsec.name
    property alias sectionOrder: keynavsec.order

    property alias subsecCount: subsecrepeater.model

    height: 48

    border.color: "#75507b"
    border.width: keynavsec.active ? 4 : 0

    KeyNavigationSection {
        id: keynavsec
    }

    Text {
        id: title
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 8
        verticalAlignment: Text.AlignVCenter
        width: 48
        text: root.sectionName
    }

    Row {
        id: subs
        anchors.left: title.right
        anchors.right: btn.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Repeater {
            id: subsecrepeater
            KeyNavSubSection {
                keynavSection: keynavsec
                subsectionName: "subsec" + model.index
                subsectionOrder: model.index
            }
        }
    }

    FlatButton {
        id: btn
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 8
        width: 40
        text: keynavsec.enabled ? "ON" : "OFF"
        onClicked: keynavsec.enabled = !keynavsec.enabled
    }
}
