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
import MuseScore.NotationScene 1.0

StyledDialogView {
    id: root

    contentWidth: 284
    contentHeight: 146
    margins: 16

    EditGridSizeDialogModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 20

        Column {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                text: qsTrc("notation", "Edit grid")
                font: ui.theme.bodyBoldFont
            }

            Item {
                width: parent.width
                height: horizontalGridSizeControl.height

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("notation", "Horizontal grid:")
                }

                Row {
                    anchors.right: parent.right
                    height: horizontalGridSizeControl.height

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter

                        text: "1 / "
                    }

                    IncrementalPropertyControl {
                        id: horizontalGridSizeControl

                        width: 100

                        currentValue: model.horizontalGridSizeSpatium
                        step: 1
                        decimals: 0
                        maxValue: 20
                        minValue: 1

                        measureUnitsSymbol: qsTrc("global", "sp")

                        onValueEdited: function(newValue) {
                            model.horizontalGridSizeSpatium = newValue
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: verticalGridSizeControl.height

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    text: qsTrc("notation", "Vertical grid:")
                }

                Row {
                    anchors.right: parent.right
                    height: verticalGridSizeControl.height

                    StyledTextLabel {
                        anchors.verticalCenter: parent.verticalCenter

                        text: "1 / "
                    }

                    IncrementalPropertyControl {
                        id: verticalGridSizeControl

                        width: 100

                        currentValue: model.verticalGridSizeSpatium
                        step: 1
                        decimals: 0
                        maxValue: 20
                        minValue: 1

                        measureUnitsSymbol: qsTrc("global", "sp")

                        onValueEdited: function(newValue) {
                            model.verticalGridSizeSpatium = newValue
                        }
                    }
                }
            }
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            buttons: [ ButtonBoxModel.Cancel, ButtonBoxModel.Ok ]

            onStandardButtonClicked: function(buttonId) {
                if (buttonId === ButtonBoxModel.Cancel) {
                    root.reject()
                } else if (buttonId === ButtonBoxModel.Ok) {
                    model.apply()
                    root.hide()
                }
            }
        }
    }
}
