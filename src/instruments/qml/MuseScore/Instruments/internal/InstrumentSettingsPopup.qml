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
import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopupView {
    id: root

    contentHeight: contentColumn.childrenRect.height

    navigation.name: "InstrumentSettingsPopup"
    navigation.direction: NavigationPanel.Vertical

    onOpened: {
        instrNameField.ensureActiveFocus()
    }

    function load(instrument) {
        settingsModel.load(instrument)
    }

    InstrumentSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        anchors.fill: parent
        spacing: 12

        StyledTextLabel {
            text: qsTrc("instruments", "Name on main score")
        }

        TextInputField {
            id: instrNameField
            objectName: "InstrNameField"
            navigation.panel: root.navigation
            navigation.row: 1
            currentText: settingsModel.instrumentName
            onCurrentTextEdited: {
                settingsModel.instrumentName = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Abbreviated name")
        }

        TextInputField {
            objectName: "AbbreviatureField"
            navigation.panel: root.navigation
            navigation.row: 2
            currentText: settingsModel.abbreviature
            onCurrentTextEdited: {
                settingsModel.abbreviature = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Part name")
        }

        TextInputField {
            objectName: "PartNameField"
            navigation.panel: root.navigation
            navigation.row: 3
            currentText: settingsModel.partName
            onCurrentTextEdited: {
                settingsModel.partName = newTextValue
            }
        }

        SeparatorLine {
            anchors.leftMargin: -root.leftPadding + root.borderWidth
            anchors.rightMargin: -root.rightPadding + root.borderWidth
        }

        FlatButton {
            width: parent.width
            navigation.panel: root.navigation
            navigation.row: 4
            text: qsTrc("instruments", "Replace instrument")

            onClicked: {
                root.close()
                Qt.callLater(settingsModel.replaceInstrument)
            }
        }
    }
}
