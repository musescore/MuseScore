/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Playback 1.0

Item {
    id: root

    visible: model.hasOnlineSounds

    width: contentRow.childrenRect.width

    OnlineSoundsStatusModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    Row {
        id: contentRow

        anchors.verticalCenter: parent.verticalCenter

        spacing: 4

        Loader {
            sourceComponent: {
                switch(model.status) {
                case OnlineSoundsStatusModel.Processing: return busyIndicator
                case OnlineSoundsStatusModel.Success: return successIndicator
                case OnlineSoundsStatusModel.Error: return errorIndicator
                }

                return undefined
            }
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: qsTrc("playback", "Online sounds")
            horizontalAlignment: Text.AlignLeft
        }
    }

    Component {
        id: successIndicator

        Rectangle {
            width: 20
            height: 20

            radius: width / 2
            color: "#32b647"

            StyledIconLabel {
                anchors.centerIn: parent

                iconCode: IconCode.TICK_RIGHT_ANGLE
                color: "white"
            }
        }
    }

    Component {
        id: errorIndicator

        StyledIconLabel {
            width: 20
            height: 20

            iconCode: IconCode.WARNING
        }
    }

    Component {
        id: busyIndicator

        StyledBusyIndicator {}
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        enabled: model.hasOnlineSounds
        hoverEnabled: model.status === OnlineSoundsStatusModel.Error

        onHoveredChanged: {
            if (mouseArea.containsMouse) {
                ui.tooltip.show(root, qsTrc("playback", "Some online sounds aren't ready yet"),
                                qsTrc("global", "Please check your internet connection or try again later."))
            } else {
                ui.tooltip.hide(root)
            }
        }
    }
}
