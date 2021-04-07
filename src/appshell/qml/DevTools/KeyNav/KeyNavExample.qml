import QtQuick 2.15
import MuseScore.Ui 1.0
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    color: ui.theme.backgroundSecondaryColor


    KeyNavSection {
        id: mainMenu
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#fce94f"

        sectionName: "mainMenu"
        sectionOrder: 1

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
        sectionOrder: 2

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
        sectionOrder: 3

        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2
                KeyNavSubSection {
                    keynavSection: leftPanel.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
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
        sectionOrder: 5
        Column {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: 2
                KeyNavSubSection {
                    keynavSection: rightPanel.keynavSection
                    subsectionName: "subsec" + model.index
                    subsectionOrder: model.index
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
        sectionOrder: 4

        KeyNavSubSection {
            keynavSection: centerPanel.keynavSection
            subsectionName: "subsec0"
            subsectionOrder: 0
        }
    }
}
