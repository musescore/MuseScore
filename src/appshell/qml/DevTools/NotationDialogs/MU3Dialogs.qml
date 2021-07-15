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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 20

        spacing: 20

        FlatButton {
            width: 220

            text: "Measure properties"

            onClicked: {
                api.launcher.open("musescore://notation/measureproperties?index=0")
            }
        }

        FlatButton {
            width: 220

            text: "Add/Remove System Breaks"

            onClicked: {
                api.launcher.open("musescore://notation/breaks")
            }
        }

        FlatButton {
            width: 220

            text: "Score properties"

            onClicked: {
                api.launcher.open("musescore://notation/properties")
            }
        }

        FlatButton {
            width: 220

            text: "Style dialog"

            onClicked: {
                api.launcher.open("musescore://notation/style")
            }
        }

        FlatButton {
            width: 220

            text: "Transpose dialog"

            onClicked: {
                api.launcher.open("musescore://notation/transpose")
            }
        }

        FlatButton {
            width: 220

            text: "Staff/part properties"

            onClicked: {
                api.launcher.open("musescore://notation/staffproperties?staffIdx=0")
            }
        }

        FlatButton {
            width: 220

            text: "Master palette"

            onClicked: {
                //! NOTE: it is important to launch master palette with sync = false
                // for an ability to use drag & drop from this dialog
                api.launcher.open("musescore://palette/masterpalette?sync=false")
            }
        }
    }
}
