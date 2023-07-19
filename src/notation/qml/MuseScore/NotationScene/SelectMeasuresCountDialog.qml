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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

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
                enabled: parent && parent.enabled && parent.visible
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

        Row {
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            spacing: 12

            NavigationPanel {
                id: buttonsNavigationPanel
                name: "ButtonsNavigationPanel"
                enabled: parent && parent.enabled && parent.visible
                section: root.navigationSection
                order: 2
                direction: NavigationPanel.Horizontal
            }

            FlatButton {
                text: qsTrc("global", "Cancel")

                navigation.name: "CancelButton"
                navigation.panel: buttonsNavigationPanel
                navigation.order: 2

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                text: qsTrc("global", "OK")
                enabled: root.measuresCount > 0
                accentButton: enabled

                navigation.name: "OkButton"
                navigation.panel: buttonsNavigationPanel
                navigation.order: 1

                onClicked: {
                    root.ret = { errcode: 0, value: root.measuresCount }
                    root.hide()
                }
            }
        }
    }
}
