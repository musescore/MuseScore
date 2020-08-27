import QtQuick 2.7
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatRadioButton {
    id: root

    property string title: ""

    signal closeRequested()

    width: 100
    radius: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: root.title
        }

        Item {
            Layout.alignment: Qt.AlignRight
            Layout.fillHeight: true
            Layout.minimumWidth: 16

            StyledIconLabel {
                anchors.centerIn: parent
                iconCode: IconCode.CLOSE_X_ROUNDED
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.closeRequested()
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1

            color: ui.theme.strokeColor
        }
    }
}
