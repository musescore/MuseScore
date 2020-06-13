import QtQuick 2.9
import QtGraphicalEffects 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FocusableItem {
    id: root

    property alias icon: expandButtonIcon.iconCode
    property int iconPixelSize: 16

    property alias title: titleLabel.text

    property alias menuItemComponent: menuLoader.sourceComponent

    property bool isExpanded: true

    anchors.left: parent.left
    anchors.leftMargin: -iconPixelSize/2
    anchors.right: parent.right

    implicitHeight: expandSectionRow.height

    Row {
        id: expandSectionRow

        spacing: 4

        Rectangle {
            id: expandButton

            height: iconPixelSize * 1.2
            width: iconPixelSize * 1.2

            color: "transparent"

            StyledIconLabel {
                id: expandButtonIcon

                anchors.fill: parent
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

            font.bold: true
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
                                            opacity: 0.3 }
        }
    ]
}
