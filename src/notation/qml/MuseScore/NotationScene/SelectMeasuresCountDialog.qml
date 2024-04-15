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

StyledDialogView {
    id: root

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight
    margins: 16

    modal: true

    property int measuresCount: 1

    ColumnLayout {
        id: content
        anchors.fill: parent
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            NavigationPanel {
                id: measuresCountNavigationPanel
                name: "MeasuresCountNavigationPanel"
                enabled: content && content.enabled && content.visible
                section: root.navigationSection
                order: 1
                direction: NavigationPanel.Horizontal
            }

            StyledTextLabel {
                id: hintLabel
                Layout.fillWidth: true
                text: qsTrc("notation", "Number of measures to insert:")
                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                id: countMeasuresInputField

                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: 132

                navigation.name: "MeasuresCountInputField"
                navigation.panel: measuresCountNavigationPanel
                navigation.order: 0
                navigation.accessible.name: hintLabel.text + " " + currentValue

                currentValue: root.measuresCount
                step: 1
                decimals: 0
                minValue: 1
                maxValue: 999

                onValueEdited: function(newValue) {
                    root.measuresCount = newValue
                }
            }
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            buttons: [ ButtonBoxModel.Cancel ]

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            FlatButton {
                text: qsTrc("global", "OK")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Ok
                enabled: root.measuresCount > 0
                accentButton: true

                onClicked: {
                    root.ret = { errcode: 0, value: root.measuresCount }
                    root.hide()
                }
            }

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.reject()
                }
            }
        }
    }
}
