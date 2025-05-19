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
import QtQuick.Controls

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyledFlickable {
    id: root

    signal goToTextStylePage(string s)

    contentWidth: column.width
    contentHeight: column.height

    HammerOnPullOffTappingPageModel {
        id: hopoPage
    }

    ColumnLayout {
        id: column
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Visibility")

            ColumnLayout {
                width: parent.width
                spacing: 10

                StyledTextLabel {
                    text: qsTrc("notation/editstyle/hammeronpulloff", "Show 'H' and 'P' symbols on")
                }

                RowLayout {
                    ToggleButton {
                        checked: hopoPage.showOnStandardStaves.value === true
                        onToggled: {
                            hopoPage.showOnStandardStaves.value = !hopoPage.showOnStandardStaves.value
                        }
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: "Standard staves"
                    }
                }

                RowLayout {
                    ToggleButton {
                        checked: hopoPage.showOnTabStaves.value === true
                        onToggled: {
                            hopoPage.showOnTabStaves.value = !hopoPage.showOnTabStaves.value
                        }
                    }

                    StyledTextLabel {
                        id : toggleText
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                        text: "Tablature staves"
                    }
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Case")

            RowLayout {
                width: parent.width
                spacing: 10

                RadioButtonGroup {
                    orientation: ListView.Horizontal
                    spacing: 6

                    model: [
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "HP"), value: true },
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "hp"), value: false },
                    ]

                    delegate: FlatRadioButton {
                        width: 40
                        height: 30
                        text: modelData.text
                        checked: hopoPage.hopoUpperCase.value === modelData.value
                        onToggled: hopoPage.hopoUpperCase.value = modelData.value
                    }
                }

                SeparatorLine {
                    orientation: Qt.Vertical
                }

                FlatButton {
                    text: qsTrc("notation", "Edit text style")

                    onClicked: {
                        root.goToTextStylePage("hammer-ons-pull-offs-and-tapping")
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/hammeronpulloff", "Consecutive hammer-on/pull-offs")

            ColumnLayout {
                width: parent.width
                spacing: 10

                RadioButtonGroup {
                    orientation: ListView.Vertical
                    spacing: 6

                    model: [
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show 'H' or 'P' between each pair of notes"), value: true },
                        {text: qsTrc("notation/editstyle/hammeronpulloff", "Show 'H' or 'P' once per group of ascending/descending notes"), value: false },
                    ]

                    delegate: RoundedRadioButton {
                        text: modelData.text
                        checked: hopoPage.hopoShowAll.value === modelData.value
                        onToggled: hopoPage.hopoShowAll.value = modelData.value
                    }
                }

                StyledImage {
                    forceHeight: 180
                    source: hopoPage.hopoShowAll.value === true ? "hammerOnPullOffImages/hopoShowAll" : "hammerOnPullOffImages/hopoNotShowAll"
                }
            }
        }
    }
}
