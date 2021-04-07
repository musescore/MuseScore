import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {

    id: root

    property alias keynavSection: keynavsec.section
    property alias subsectionName: keynavsec.name
    property alias subsectionOrder: keynavsec.order

    height: 40
    width: btns.childrenRect.width

    border.color: "#75507b"
    border.width: keynavsec.active ? 4 : 0

    KeyNavigationSubSection {
        id: keynavsec

        onActiveChanged: {
            if (keynavsec.active) {
                root.forceActiveFocus()
            }
        }
    }

    Row {
        id: btns
        anchors.fill: parent
        spacing: 8

        FlatButton {
            id: btn1
            keynav.subsection: keynavsec
            keynav.order: 1
            anchors.verticalCenter: parent.verticalCenter
            height: 24
            width: 24
            text: "C1"
            onClicked: {
                console.log("onClicked " + text)
                ui.tooltip.show(this, text)
            }
        }

        FlatButton {
            id: btn2
            keynav.subsection: keynavsec
            keynav.order: 2
            anchors.verticalCenter: parent.verticalCenter
            height: 24
            width: 24
            text: "C2"
            onClicked: {
                console.log("onClicked " + text)
                ui.tooltip.show(this, text)
            }
        }

        FlatButton {
            id: btn3
            keynav.subsection: keynavsec
            keynav.order: 3
            anchors.verticalCenter: parent.verticalCenter
            height: 24
            width: 24
            text: "C3"
            onClicked: {
                console.log("onClicked " + text)
                ui.tooltip.show(this, text)
            }
        }
    }
}
