import QtQuick 2.9

import MuseScore.UiComponents 1.0

FocusableItem {
    id: root

    property alias icon: buttonIcon.iconCode
    property string text: ""
    property string progressTitle: ""
    property int iconPixelSize: buttonIcon.isEmpty ? 0 : 16

    property real from: 0.0
    property real to: 1.0
    property real value: 0.0

    property bool indeterminate: false

    signal clicked()

    function setProgress(status, indeterminate, current, total) {
        root.progressTitle = status
        root.indeterminate = indeterminate
        root.value = 0.0
        if (!indeterminate) {
            root.value = current
            root.to = total
        }
    }

    function resetProgress() {
        indeterminate = false
    }

    height: contentWrapper.implicitHeight + 16
    width: 132

    opacity: root.enabled ? 1.0 : 0.3

    QtObject {
        id: privateProperties

        property bool inProgress: (from < value && value < to) || indeterminate
    }

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        color: privateProperties.inProgress ? ui.theme.backgroundPrimaryColor : ui.theme.accentColor
        opacity: ui.theme.buttonOpacityNormal
        border.width: 0
        radius: 3
    }

    Rectangle {
        id: progressRect

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        width: privateProperties.inProgress ? parent.width * (value / to) : 0

        color: ui.theme.accentColor
    }

    Column {
        id: contentWrapper

        anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined

        height: implicitHeight

        spacing: 4

        StyledIconLabel {
            id: buttonIcon

            anchors.horizontalCenter: parent.horizontalCenter
        }

        StyledTextLabel {
            id: textLabel

            anchors.horizontalCenter: parent.horizontalCenter
            height: text === "" ? 0 : implicitHeight

            horizontalAlignment: Text.AlignHCenter

            text: privateProperties.inProgress ? progressTitle : root.text
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        hoverEnabled: true

        enabled: !privateProperties.inProgress

        onReleased: {
            root.clicked()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHit
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        }
    ]
}
