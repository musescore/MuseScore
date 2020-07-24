import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    Row {
        anchors.fill: parent

        Repeater {
            anchors.fill: parent
            model: toolModel
            delegate: ToolButton {
                id: control
                text: titleRole
                enabled: enabledRole
                down: checkedRole
                onClicked: toolModel.click(nameRole)

                contentItem: StyledTextLabel {
                    text: control.text
                    font: control.font
                    opacity: enabled ? 1.0 : 0.5
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitWidth: 40
                    implicitHeight: 40
                    color: control.enabled && (control.checked || control.highlighted) ? ui.theme.accentColor : ui.theme.buttonColor
                    opacity: enabled ? ui.theme.accentOpacityNormal : 0.3
                    visible: control.down || (control.enabled && (control.checked || control.highlighted))
                }
            }
        }
    }

    NotationToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }
}
