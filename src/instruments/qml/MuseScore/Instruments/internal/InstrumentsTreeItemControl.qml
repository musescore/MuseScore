import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property bool isHighlighted: false

    property int keynavRow: 0
    property KeyNavigationSubSection keynavSubSection: null

    Rectangle {
        anchors.fill: parent

        color: root.isHighlighted ? ui.theme.accentColor : ui.theme.textFieldColor
        opacity: root.isHighlighted ? 0.5 : 1

        border.color: ui.theme.focusColor
        border.width: keynavItem.active ? 2 : 0
    }

    signal clicked()

    KeyNavigationControl {
        id: keynavItem
        name: "InstrumentsTreeItemControl"
        subsection: root.keynavSubSection
        row: root.keynavRow
        column: 0
        enabled: visible

        onActiveChanged: {
            if (active) {
                root.focusActived()
            }
        }
    }


    RowLayout {
        anchors.top: parent.top
        anchors.topMargin: 16
        anchors.bottom: separator.top
        anchors.bottomMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 2

        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 90 * styleData.depth
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                id: addButton

                anchors.left: parent.left

                width: 24
                height: width

                objectName: "InstrumentsAddStaffBtn"
                keynav.subsection: root.keynavSubSection
                keynav.row: root.keynavRow
                keynav.column: 1

                icon: IconCode.PLUS
            }

            StyledTextLabel {
                anchors {
                    left: addButton.right
                    leftMargin: 8
                    right: parent.right
                    rightMargin: 8
                    verticalCenter: addButton.verticalCenter
                }
                horizontalAlignment: Text.AlignLeft

                text: model ? model.itemRole.title : ""
            }
        }
    }

    SeparatorLine { id: separator; anchors.bottom: parent.bottom; }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.clicked()
        }
    }
}
