/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

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
            onSelectedUpClicked: function(index) {
                settingsModel.soundFontUp(index, "Fluid")
            }
            onSelectedDownClicked: function(index) {
                settingsModel.soundFontDown(index, "Fluid")
            }
            onSelectedRemoveClicked: function(index) {
                settingsModel.removeSoundFont(index, "Fluid")
            }
            onAddClicked: function(index) {
                settingsModel.addSoundFont(index, "Fluid")
            }

            Connections {
                target: settingsModel
                onSelectedChanged: function(name) {
                    if (name === "Fluid") {
                        fluidPanel.selectedSoundFonts = []
                        fluidPanel.selectedSoundFonts = settingsModel.selectedSoundFonts("Fluid")
                    }
                }

                onAvalaibleChanged: function(name) {
                    if (name === "Fluid") {
                        fluidPanel.avalaibleSoundFonts = []
                        fluidPanel.avalaibleSoundFonts = settingsModel.avalaibleSoundFonts("Fluid")
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

        FlatButton {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 8
            height: 40
            text: "Apply"
            onClicked: settingsModel.apply()
        }
    }
}
