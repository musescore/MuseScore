import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MuseScore.Ui 1.0

MenuItem {
    id: root

    property Action actionItem: null

    implicitHeight: 30
    implicitWidth: parent.width

    enabled: actionItem ? actionItem.enabled : false

    background: Rectangle {
        anchors.fill: parent

        color: mouseArea.containsMouse ? ui.theme.accentColor : ui.theme.buttonColor

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            propagateComposedEvents: true
            hoverEnabled: true

            onClicked: {
                if (actionItem) {
                    actionItem.trigger()
                }
            }
        }
    }

    contentItem: RowLayout {
        id: contentRow

        implicitHeight: root.implicitHeight
        implicitWidth: root.implicitWidth

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.preferredWidth: (parent.width / 3) * 2
            Layout.preferredHeight: parent.height

            text: root.text
            font: root.font
            color: ui.theme.fontPrimaryColor
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }

        StyledIconLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 3
            Layout.preferredHeight: parent.height

            visible: root.actionItem ? root.actionItem.checkable && root.actionItem.checked : false
            horizontalAlignment: Text.AlignRight

            iconCode: IconCode.TICK
        }
    }
}
