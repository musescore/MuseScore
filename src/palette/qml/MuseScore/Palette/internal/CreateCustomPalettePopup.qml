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

StyledPopup {
    id: root

    signal addCustomPaletteRequested(var paletteName)

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 14

        StyledTextLabel {
            text: qsTrc("palette", "Name your custom palette")
        }

        TextInputField {
            id: paletteNameField

            width: parent.width

            property string name: ""

            onCurrentTextEdited: {
                name = newTextValue
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height

            spacing: 4

            function close() {
                paletteNameField.clear()
                root.close()
            }

            FlatButton {
                text: qsTrc("global", "Cancel")

                width: parent.width / 2

                onClicked: {
                    parent.close()
                }
            }

            FlatButton {
                text: qsTrc("pallette", "Create")

                width: parent.width / 2

                enabled: Boolean(paletteNameField.name)

                onClicked: {
                    root.addCustomPaletteRequested(paletteNameField.name)
                    parent.close()
                }
            }
        }
    }
}
