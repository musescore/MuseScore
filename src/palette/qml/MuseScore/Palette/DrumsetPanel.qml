/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import MuseScore.Palette 1.0

Item {
    id: root

    property alias navigationSection: navPanel.section
    property alias contentNavigationPanelOrderStart: navPanel.order

    NavigationPanel {
        id: navPanel
        name: "DrumsetSection"
        direction: NavigationPanel.Vertical
        enabled: root.enabled && root.visible
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8

        spacing: 12

        Column {
            Layout.alignment: Qt.AlignVCenter

            spacing: 6

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                height: 20
                width: customizeKitButton.width

                text: drumsetView.pitchName
            }

            FlatButton {
                id: customizeKitButton

                text: qsTrc("palette", "Customize kit")

                onClicked: {
                    drumsetView.customizeKit()
                }
            }
        }

        DrumsetPanelView {
            id: drumsetView

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
