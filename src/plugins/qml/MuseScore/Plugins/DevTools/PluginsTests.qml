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
import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Plugins 1.0

Rectangle {
    color: "#fea534"

    PluginsTestModel {
        id: pluginsModel
    }

    Component.onCompleted: {
        pluginsModel.load()
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16

        text: "Installed plugins"

        font: ui.theme.headerBoldFont
    }

    Grid {
        anchors.top: pageTitle.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16

        spacing: 16
        columns: 3

        Repeater {
            anchors.fill: parent

            model: pluginsModel.installedPluginsNames

            delegate: FlatButton {
                width: 200

                text: modelData

                onClicked: {
                    pluginsModel.run(index)
                }
            }
        }
    }
}
