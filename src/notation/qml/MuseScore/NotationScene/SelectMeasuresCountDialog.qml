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

    contentWidth: 534
    contentHeight: 146
    margins: 16

    modal: true

    property string operation: ""
    property int measuresCount: 1

    QtObject {
        id: privateProperties

        function capitalizeFirstLetter(string) {
            return string.charAt(0).toUpperCase() + string.slice(1);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        Column {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                text: privateProperties.capitalizeFirstLetter(root.operation) + " " + qsTrc("notation", "empty measures")
                font: ui.theme.bodyBoldFont
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 12

                StyledTextLabel {
                    Layout.fillWidth: true
                    text: qsTrc("notation", "Number of measures to ") + root.operation.toLowerCase() + ":"
                }

                IncrementalPropertyControl {
                    id: countMeasuresInputField

                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 100

                    currentValue: root.measuresCount
                    step: 1
                    decimals: 0
                    maxValue: 1000
                    minValue: 1

                    onValueEdited: function(newValue) {
                        root.measuresCount = newValue
                    }
                }
            }
        }

        Row {
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            spacing: 12

            FlatButton {
                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                text: qsTrc("global", "OK")

                onClicked: {
                    root.ret = {errcode: 0, value: root.measuresCount}
                    root.hide()
                }
            }
        }
    }
}
