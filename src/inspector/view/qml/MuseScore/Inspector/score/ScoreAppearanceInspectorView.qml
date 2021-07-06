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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width

        spacing: 24

        Column {
            width: parent.width

            spacing: 8

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "Page settings"
                navigation.row: root.navigationRow(6)

                text: qsTrc("inspector", "Page settings")

                onClicked: {
                    if (root.model) {
                        root.model.showPageSettings()
                    }
                }
            }

            FlatButton {
                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.name: "Style settings"
                navigation.row: root.navigationRow(7)

                text: qsTrc("inspector", "Style settings")

                onClicked: {
                    if (root.model) {
                        root.model.showStyleSettings()
                    }
                }
            }
        }
    }
}
