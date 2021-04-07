import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RadioDelegate {
    id: root

    property Component iconComponent: null
    property string title: ""

    property int orientation: Qt.Vertical

    property var normalStateFont: ui.theme.tabFont
    property var selectedStateFont: ui.theme.tabBoldFont

    property alias keynav: keynavCtrl

    height: 48

    spacing: 30
    leftPadding: 0
    rightPadding: 0

    onToggled: {
        if (!keynavCtrl.active) {
            keynavCtrl.forceActive()
        }
    }

    KeyNavigationControl {
        id: keynavCtrl
        name: root.objectName

        onActiveChanged: {
            if (keynavCtrl.active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.toggled()
    }

    background: Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundRect
            anchors.fill: parent

            color: ui.theme.backgroundPrimaryColor
            opacity: ui.theme.buttonOpacityNormal

            border.color: ui.theme.focusColor
            border.width: keynavCtrl.active ? 2 : 0
            radius: 2
        }

        Item {
            id: backgroundGradientRect
            anchors.fill: parent
            anchors.margins: 2 //! NOTE margin needed to show focus border

            property bool isVertical: orientation === Qt.Vertical
            visible: false

            LinearGradient {
                id: backgroundGradient
                anchors.fill: parent

                start: Qt.point(0, 0)
                end: backgroundGradientRect.isVertical ? Qt.point(0, root.width) : Qt.point(root.width, 0)
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 1.0; color: Qt.rgba(ui.theme.accentColor.r,
                                                                 ui.theme.accentColor.g,
                                                                 ui.theme.accentColor.b,
                                                                 backgroundGradientRect.isVertical ? 0.2 : 0.1) }
                }
            }

            Rectangle {
                id: line
                color: ui.theme.accentColor

                states: [
                    State {
                        when: backgroundGradientRect.isVertical
                        AnchorChanges {
                            target: line
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                        }

                        PropertyChanges {
                            target: line
                            width: parent.width
                            height: 2
                        }
                    },
                    State {
                        when: !backgroundGradientRect.isVertical
                        AnchorChanges {
                            target: line
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                        }

                        PropertyChanges {
                            target: line
                            width: 2
                            height: parent.height
                        }
                    }
                ]
            }
        }
    }

    contentItem: Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        spacing: root.spacing
        leftPadding: root.leftPadding
        rightPadding: root.rightPadding

        Loader {
            anchors.verticalCenter: parent.verticalCenter

            sourceComponent: iconComponent
            visible: Boolean(iconComponent)
        }

        StyledTextLabel {
            id: textLabel

            anchors.verticalCenter: parent.verticalCenter
            width: implicitWidth

            visible: Boolean(title)

            horizontalAlignment: Text.AlignLeft
            font: normalStateFont
            text: title
        }
    }

    indicator: Item {}

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !root.checked && !root.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: root.pressed && !root.checked

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHit
            }
        },

        State {
            name: "SELECTED"
            when: root.checked

            PropertyChanges {
                target: backgroundGradientRect
                visible: true
            }

            PropertyChanges {
                target: textLabel
                font: selectedStateFont
            }
        }
    ]
}
