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
import QtQuick.Controls 2.15
import QtQuick.Layouts

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "internal"

StyledFlickable {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 0

    enabled: selectionFilterModel.enabled

    contentWidth: root.width
    contentHeight: contentColumn.implicitHeight

    Component.onCompleted: {
        voicesModel.load()
        elementsModel.load()
    }

    Column {
        id: contentColumn

        anchors {
            fill: parent
            leftMargin: 12
            rightMargin: 12
        }

        spacing: 10

        SelectionFilterSection {
            id: voicesSection

            sectionTitle: qsTrc("notation", "Voices")

            navigation {
                section: root.navigationSection
                order: root.navigationOrderStart + 1
                direction: NavigationPanel.Horizontal
            }

            RowLayout {
                id: voiceButtonsRow

                width: parent.width
                spacing: voicesSection.spacing

                Repeater {
                    model: VoicesSelectionFilterModel {
                        id: voicesModel
                    }

                    delegate: FlatButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24

                        text: model.title

                        navigation.panel: voicesSection.navigation
                        navigation.order: model.index

                        accentButton: model.isSelected
                        onClicked: {
                            model.isSelected = !model.isSelected
                        }
                    }
                }
            }
        }

        SeparatorLine {}

        SelectionFilterSection {
            id: notationElementsSection

            sectionTitle: qsTrc("notation", "Notation elements")

            navigation {
                section: root.navigationSection
                order: root.navigationOrderStart + 2
                direction: NavigationPanel.Vertical
            }

            StyledListView {
                width: parent.width
                height: contentHeight

                spacing: notationElementsSection.spacing
                clip: false

                model: ElementsSelectionFilterModel {
                    id: elementsModel
                }

                interactive: false

                delegate: CheckBox {
                    width: ListView.view.width

                    text: model.title

                    navigation.panel: notationElementsSection.navigation
                    navigation.order: model.index

                    checked: model.isSelected
                    isIndeterminate: model.isIndeterminate
                    onClicked: {
                        model.isSelected = !checked
                    }
                }
            }
        }
    }
}
