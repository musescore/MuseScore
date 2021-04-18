import QtQuick 2.9
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    height: parent ? parent.height : implicitHeight
    width: parent ? parent.width : implicitWidth

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 6
        anchors.rightMargin: 6

        spacing: 16

        FlatButton {
            Layout.alignment: Qt.AlignLeft

            normalStateColor: "transparent"
            pressedStateColor: ui.theme.accentColor

            icon: Boolean(itemRole) && itemRole.checked ? IconCode.VISIBILITY_ON : IconCode.VISIBILITY_OFF

            onClicked: {
                itemRole.checked = !itemRole.checked
            }
        }

        StyledIconLabel {
            Layout.alignment: Qt.AlignLeft

            width: 36
            height: width

            iconCode: Boolean(itemRole) ? itemRole.icon : IconCode.NONE
        }

        StyledTextLabel {
            Layout.fillWidth: true

            horizontalAlignment: Qt.AlignLeft
            text: Boolean(itemRole) ? itemRole.title : ""
        }
    }
}
