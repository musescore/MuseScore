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
import Muse.Ui 1.0
import Muse.UiComponents 1.0    
import Muse.Diagnostics 1.0

Rectangle {

    anchors.fill: parent

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        pathsModel.load()
    }

    DiagnosticsPathsModel {
        id: pathsModel
    }

    ListView {
        anchors.fill: parent
        anchors.margins: 16
        clip: true
        model: pathsModel
        delegate: ListItemBlank {

            id: dgate

            property var item: itemData

            StyledTextLabel {
                id: nameLabel
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 200
                horizontalAlignment: Text.AlignLeft
                text: dgate.item.name + ": "
            }

            StyledTextLabel {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: nameLabel.right
                anchors.right: parent.right
                horizontalAlignment: Text.AlignLeft
                text: dgate.item.path
            }

            onDoubleClicked: {
                pathsModel.openPath(dgate.item.path)
            }
        }
    }
}
