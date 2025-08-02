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

import QtQuick 2.12
import QtQuick.Controls 2.12

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property var model

    property alias navigationPanel: view.navigationPanel

    QtObject {
        id: prv

        readonly property int sideMargin: 36
        property string currentItemNavigationName: ""
    }

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: prv.sideMargin

        spacing: 16

        StyledTextLabel {
            width: parent.width

            text: qsTrc("notation", "Name")

            horizontalAlignment: Qt.AlignLeft
            font.capitalization: Font.AllUppercase
        }

        SeparatorLine { anchors.margins: -prv.sideMargin }
    }

    StyledListView {
        id: view

        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        spacing: 0

        model: root.filteredModel()

        interactive: height < contentHeight

        property NavigationPanel navigationPanel: NavigationPanel {
            name: "PartsView"
            enabled: root.enabled && root.visible
            direction: NavigationPanel.Both
            accessible.name: qsTrc("notation", "Parts view")
            onActiveChanged: function(active) {
                if (active) {
                    root.forceActiveFocus()
                }
            }

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", prv.currentItemNavigationName)
                }
            }
        }

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }

        Connections {
            target: root.model

            function onPartAdded(index) {
                view.positionViewAtIndex(index, ListView.Contain)
                view.currentIndex = index
                view.currentItem.startEditTitle()
            }
        }

        delegate: PartDelegate {
            sideMargin: prv.sideMargin

            title: model.title
            currentPartIndex: view.currentIndex
            isSelected: model.isSelected
            canReset: model.isInited
            canDelete: model.isCustom

            navigation.name: model.title + model.index
            navigation.panel: root.navigationPanel
            navigation.column: (model.isSelected && root.model.hasSelection) ? 4 : -1
            navigation.row: model.index
            navigation.onActiveChanged: {
                if (navigation.active) {
                    prv.currentItemNavigationName = navigation.name
                    view.positionViewAtIndex(index, ListView.Contain)
                }
            }

            onPartClicked: {
                root.model.selectPart(model.index)
                view.currentIndex = model.index
            }

            onResetPartRequested: {
                root.model.resetPart(model.index)
            }

            onRemovePartRequested: {
                root.model.removePart(model.index)
            }

            onTitleEdited: function(newTitle) {
                incorrectTitleWarning = root.model.validatePartTitle(model.index, newTitle)
            }

            onTitleEditingFinished: function(newTitle) {
                root.model.setPartTitle(model.index, newTitle)
            }

            onCopyPartRequested: {
                root.model.copyPart(model.index)
            }
        }
    }
}
