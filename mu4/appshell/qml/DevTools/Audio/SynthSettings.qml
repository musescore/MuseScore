import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    SynthsSettingsModel {
        id: settingsModel
    }

    Component.onCompleted: {
        settingsModel.load()
    }

    Item {
        id: synthListPanel

        property int currentIndex: 0

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 120

        Column {

            Button {
                text: "Fluid"
                checkable: true
                checked: synthListPanel.currentIndex === 0
                onClicked: synthListPanel.currentIndex = 0
            }

            Button {
                text: "Zerberus"
                checkable: true
                checked: synthListPanel.currentIndex === 1
                onClicked: synthListPanel.currentIndex = 1
            }

        }
    }

    Rectangle {
        id: separator
        anchors.top: parent.top
        anchors.bottom: bottomPanel.top
        anchors.left: synthListPanel.right
        width: 4
        color: "#666666"
    }

    StackLayout {
        anchors.top: parent.top
        anchors.bottom: bottomPanel.top
        anchors.left: separator.right
        anchors.right: parent.right
        currentIndex: synthListPanel.currentIndex

        SoundFontsPanel {
            id: fluidPanel
            selectedSoundFonts: settingsModel.selectedSoundFonts("Fluid")
            avalaibleSoundFonts: settingsModel.avalaibleSoundFonts("Fluid")
            onSelectedUpClicked: settingsModel.soundFontUp(index, "Fluid")
            onSelectedDownClicked: settingsModel.soundFontDown(index, "Fluid")
            onSelectedRemoveClicked: settingsModel.removeSoundFont(index, "Fluid")
            onAddClicked: settingsModel.addSoundFont(index, "Fluid")

            Connections {
                target: settingsModel
                onSelectedChanged: {
                    if (name === "Fluid") {
                        fluidPanel.selectedSoundFonts = []
                        fluidPanel.selectedSoundFonts = settingsModel.selectedSoundFonts("Fluid")
                    }
                }

                onAvalaibleChanged: {
                    if (name === "Fluid") {
                        fluidPanel.avalaibleSoundFonts = []
                        fluidPanel.avalaibleSoundFonts = settingsModel.avalaibleSoundFonts("Fluid")
                    }
                }
            }
        }

        SoundFontsPanel {
            id: zerberusPanel
            selectedSoundFonts: settingsModel.selectedSoundFonts("Zerberus")
            avalaibleSoundFonts: settingsModel.avalaibleSoundFonts("Zerberus")
            onSelectedUpClicked: settingsModel.soundFontUp(index, "Zerberus")
            onSelectedDownClicked: settingsModel.soundFontDown(index, "Zerberus")
            onSelectedRemoveClicked: settingsModel.removeSoundFont(index, "Zerberus")
            onAddClicked: settingsModel.addSoundFont(index, "Zerberus")

            Connections {
                target: settingsModel
                onSelectedChanged: {
                    if (name === "Zerberus") {
                        zerberusPanel.selectedSoundFonts = []
                        zerberusPanel.selectedSoundFonts = settingsModel.selectedSoundFonts("Zerberus")
                    }
                }

                onAvalaibleChanged: {
                    if (name === "Zerberus") {
                        zerberusPanel.avalaibleSoundFonts = []
                        zerberusPanel.avalaibleSoundFonts = settingsModel.avalaibleSoundFonts("Zerberus")
                    }
                }
            }
        }
    }

    Item {
        id: bottomPanel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 64

        Rectangle {
            id: sep
            anchors.left: parent.left
            anchors.right: parent.right
            height: 4
            color: "#666666"
        }

        Button {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8
            height: 40
            font.bold: true
            text: "Apply"
            onClicked: settingsModel.apply()
        }
    }
}
