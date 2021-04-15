/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

StyledPopup {
    id: root

    height: Math.max(contentColumn.implicitHeight + topPadding + bottomPadding, implicitHeight)
    width: parent.width

    implicitHeight: 290

    function load(instrument) {
        settingsModel.load(instrument)
    }

    InstrumentSettingsModel {
        id: settingsModel
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        StyledTextLabel {
            text: qsTrc("instruments", "Name on main score")
        }

        TextInputField {
            currentText: settingsModel.instrumentName

            onCurrentTextEdited: {
                settingsModel.instrumentName = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Abbreviated name")
        }

        TextInputField {
            currentText: settingsModel.abbreviature

            onCurrentTextEdited: {
                settingsModel.abbreviature = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("instruments", "Part name")
        }

        TextInputField {
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
            text: qsTrc("instruments", "Replace instrument")

            onClicked: {
                root.close()
                Qt.callLater(settingsModel.replaceInstrument)
            }
        }
    }
}
