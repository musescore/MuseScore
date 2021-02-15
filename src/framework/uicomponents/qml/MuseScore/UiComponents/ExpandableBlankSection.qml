import QtQuick 2.9
import QtGraphicalEffects 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FocusableItem {
    id: root

    property alias title: titleLabel.text

    property alias menuItemComponent: menuLoader.sourceComponent

    property bool isExpanded: true

    anchors.left: parent.left
    anchors.leftMargin: -expandButtonIcon.width / 2
    anchors.right: parent.right

    implicitHeight: expandSectionRow.height

    Row {
        id: expandSectionRow

        spacing: 4

        Rectangle {
            id: expandButton

            height: expandButtonIcon.height * 1.2
            width: expandButtonIcon.width * 1.2

            color: "transparent"

            StyledIconLabel {
                id: expandButtonIcon

                rotation: root.isExpanded ? 0 : -90

                iconCode: IconCode.SMALL_ARROW_DOWN

                Behavior on rotation {
                    NumberAnimation {
                        easing.type: Easing.OutQuad
                        duration: 50
                    }
                }
            }
        }

        StyledTextLabel {
            id: titleLabel

            anchors.verticalCenter: expandButton.verticalCenter

            font: ui.theme.bodyBoldFont
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: expandSectionRow

        hoverEnabled: true

        onClicked: {
            root.isExpanded = !root.isExpanded
        }
    }

    Loader {
        id: menuLoader

        property bool isMenuButtonVisible: root.isExpanded
                                           || mouseArea.containsMouse
        anchors {
            right: root.right
            rightMargin: 48
            top: expandSectionRow.top
        }

        height: childrenRect.height
        width: childrenRect.width
    }

    states: [
        State {
            name: "DISABLED"
            when: !root.enabled

            PropertyChanges { target: root; isExpanded: false
                                            opacity: ui.theme.itemOpacityDisabled }
        }
    ]
}
