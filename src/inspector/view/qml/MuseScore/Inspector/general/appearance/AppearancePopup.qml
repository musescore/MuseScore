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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopupView {
    id: root

    property QtObject model: undefined

    contentHeight: contentColumn.implicitHeight

    navigation.name: "AppearancePopup"
    navigation.direction: NavigationPanel.Vertical

    onOpened: {
        horizontalSpacingSection.focusOnFirst()
    }

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        HorizontalSpacingSection {
            id: horizontalSpacingSection
            leadingSpace: model ? model.leadingSpace : null
            barWidth: model ? model.barWidth : null

            navigationPanel: root.navigation
            navigationRowOffset: 100
        }

        SeparatorLine { anchors.margins: -10 }

        VerticalSpacingSection {
            navigationPanel: root.navigation
            navigationRowOffset: 200
            minimumDistance: model ? model.minimumDistance : null
        }

        SeparatorLine { anchors.margins: -10 }

        OffsetSection {
            navigationPanel: root.navigation
            navigationRowOffset: 300
            horizontalOffset: model ? model.horizontalOffset : null
            verticalOffset: model ? model.verticalOffset : null
            isSnappedToGrid: model ? model.isSnappedToGrid : null

            onSnapToGridToggled: {
                if (model) {
                    model.isSnappedToGrid = snap
                }
            }

            onConfigureGridRequested: {
                if (model) {
                    model.configureGrid()
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        ArrangeSection {
            navigationPanel: root.navigation
            navigationRowOffset: 400
            onPushBackRequested: {
                if (root.model) {
                    root.model.pushBackInOrder()
                }
            }

            onPushFrontRequested: {
                if (root.model) {
                    root.model.pushFrontInOrder()
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        ColorSection {
            navigationPanel: root.navigation
            navigationRowOffset: 500
            color: root.model ? root.model.color : null
        }
    }
}
