import QtQuick 2.15
import QtQuick.Layouts 1.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    color: ui.theme.backgroundSecondaryColor

    property string lastClickedInfo: ""

    Rectangle {
        id: infoPanel

        anchors.left: parent.left
        anchors.right: parent.right
        height: 64

        StyledTextLabel {
            anchors.fill: parent
            anchors.margins: 8
            verticalAlignment: Text.AlignVCenter
            text: "Last clicked: " + root.lastClickedInfo
        }
    }

    KeyNavSection {
        id: mainMenu
        anchors.top: infoPanel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#fce94f"

        sectionName: "mainMenu"
        sectionOrder: 101

        RowLayout {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 3
                KeyNavSubSection {
                    Layout.fillWidth: true
                    anchors.verticalCenter: parent.verticalCenter
                    keynavSection: mainMenu.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
                    onClicked: root.lastClickedInfo = "sec: " + mainMenu.sectionName + ", " + info
                }
            }
        }
    }

    KeyNavSection {
        id: topTools
        anchors.top: mainMenu.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#e9b96e"

        sectionName: "topTools"
        sectionOrder: 102

        Row {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2
                KeyNavSubSection {
                    anchors.verticalCenter: parent.verticalCenter
                    keynavSection: topTools.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
                    onClicked: root.lastClickedInfo = "sec: " + topTools.sectionName + ", " + info
                }
            }
        }
    }

    KeyNavSection {
        id: leftPanel
        anchors.left: parent.left
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        width: 120
        color: "#729fcf"

        sectionName: "leftPanel"
        sectionOrder: 103

        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2
                KeyNavSubSection {
                    keynavSection: leftPanel.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
                    onClicked: root.lastClickedInfo = "sec: " + leftPanel.sectionName + ", " + info
                }
            }
        }
    }

    KeyNavSection {
        id: rightPanel
        anchors.right: parent.right
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        width: 120
        color: "#8ae234"

        sectionName: "rightPanel"
        sectionOrder: 105

        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2
                KeyNavSubSection {
                    keynavSection: rightPanel.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
                    onClicked: root.lastClickedInfo = "sec: " + rightPanel.sectionName + ", " + info
                }
            }
        }
    }

    KeyNavSection {
        id: centerPanel
        anchors.left: leftPanel.right
        anchors.right: rightPanel.left
        anchors.top: topTools.bottom
        anchors.bottom: parent.bottom
        color: "#ef2929"

        sectionName: "centerPanel"
        sectionOrder: 104

        KeyNavSubSection {
            keynavSection: centerPanel.keynavSection
            subsectionName: "subsec0"
            subsectionOrder: 0
            onClicked: root.lastClickedInfo = "sec: " + centerPanel.sectionName + ", " + info
        }
    }
}
