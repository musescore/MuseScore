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
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    property NavigationSection navigationSection: null

    height: 30
    visible: notationsView.count > 0
    color: ui.theme.backgroundSecondaryColor

    function ensureActive() {
        var item = notationsView.itemAtIndex(notationsView.currentIndex)
        item.navigation.requestActive()
    }

    NotationSwitchListModel {
        id: notationSwitchModel

        onCurrentNotationIndexChanged: {
            notationsView.currentIndex = index
        }
    }

    Component.onCompleted: {
        notationSwitchModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "NotationViewTabs"
        section: root.navigationSection
        direction: NavigationPanel.Horizontal
        order: 1
    }

    RadioButtonGroup {
        id: notationsView

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 1

        width: Math.min(contentWidth, parent.width)

        model: notationSwitchModel
        currentIndex: 0
        spacing: 0
        interactive: width < contentWidth
        boundsBehavior: Flickable.StopAtBounds

        delegate: NotationSwitchButton {
            id: button

            navigation.name: "NotationTab" + model.index
            navigation.panel: navPanel
            navigation.row: 1
            navigation.column: model.index + 1

            text: model.title
            needSave: model.needSave

            ButtonGroup.group: notationsView.radioButtonGroup
            checked: model.index === notationsView.currentIndex

            function resolveNextNotationIndex() {
                var nextIndex = model.index - 1
                if (nextIndex < 0) {
                    return 0
                }

                return nextIndex
            }

            onToggled: {
                notationSwitchModel.setCurrentNotation(model.index)
            }

            onCloseRequested: {
                if (model.index !== notationsView.currentIndex) {
                    notationSwitchModel.closeNotation(model.index)
                    return
                }

                var index = button.resolveNextNotationIndex()
                notationSwitchModel.closeNotation(model.index)
                notationSwitchModel.setCurrentNotation(index)
            }
        }
    }
}
