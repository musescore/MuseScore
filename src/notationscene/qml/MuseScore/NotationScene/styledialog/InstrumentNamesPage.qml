/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledFlickable {
    id: root

    signal goToTextStylePage(int index)

    contentWidth: Math.max(column.implicitWidth, root.width)
    contentHeight: column.implicitHeight

    InstrumentNamesPageModel {
        id: instrumentNamesModel
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/instrumentnames", "Instrument & staff names alignment")

            ColumnLayout {
                width: parent.width
                spacing: 12

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Long names")
                    }

                    Row {
                        spacing: 8

                        Repeater {
                            model: [
                                { source: "instrumentNamesImages/long-right-right.png", value: 0 },
                                { source: "instrumentNamesImages/long-center-right.png", value: 1 },
                                { source: "instrumentNamesImages/long-center-center.png", value: 2 },
                                { source: "instrumentNamesImages/long-left-right.png", value: 3 },
                            ]

                            SelectableStyledImage {
                                required property var modelData
                                forceHeight: 120

                                source: modelData.source
                                checked: instrumentNamesModel.instrumentNamesAlignLong.value === modelData.value
                                onClicked: instrumentNamesModel.instrumentNamesAlignLong.value = modelData.value
                            }
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Abbreviated names")
                    }

                    Row {
                        spacing: 8

                        Repeater {
                            model: [
                                { source: "instrumentNamesImages/short-right-right.png", value: 0 },
                                { source: "instrumentNamesImages/short-center-right.png", value: 1 },
                                { source: "instrumentNamesImages/short-center-center.png", value: 2 },
                                { source: "instrumentNamesImages/short-left-right.png", value: 3 },
                            ]

                            SelectableStyledImage {
                                required property var modelData
                                forceHeight: 120

                                source: modelData.source
                                checked: instrumentNamesModel.instrumentNamesAlignShort.value === modelData.value
                                onClicked: instrumentNamesModel.instrumentNamesAlignShort.value = modelData.value
                            }
                        }
                    }
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8
                    enabled: instrumentNamesModel.instrumentNamesAlignLong.value === 0 || instrumentNamesModel.instrumentNamesAlignShort.value === 0

                    StyledTextLabel {
                        text: qsTrc("notation/editstyle/instrumentnames", "Overlapping names")
                    }

                    Row {
                        spacing: 8

                        Repeater {
                            model: [
                                { source: "instrumentNamesImages/stack-horizontally.png", value: false },
                                { source: "instrumentNamesImages/stack-vertically.png", value: true },
                            ]

                            SelectableStyledImage {
                                required property var modelData
                                forceHeight: 170

                                source: modelData.source
                                checked: instrumentNamesModel.instrumentNamesStackVertically.value === modelData.value
                                onClicked: instrumentNamesModel.instrumentNamesStackVertically.value = modelData.value
                            }
                        }
                    }
                }
            }
        }
    }
}
