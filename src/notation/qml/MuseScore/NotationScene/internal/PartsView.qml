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
import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property var model

    property alias navigationPanel: view.navigationPanel

    function focusOnFirst() {
        root.model.selectPart(0)
    }

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

            text: qsTrc("notation", "NAME")

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

        model: root.model

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

        ScrollBar.vertical: StyledScrollBar {}

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
            isCreated: model.isCreated

            navigation.name: model.title + model.index
            navigation.panel: view.navigationPanel
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

            onTitleEdited: function(newTitle) {
                root.model.setPartTitle(model.index, newTitle)
            }

            onTitleEditingFinished: {
                root.model.validatePartTitle(model.index)
            }

            onRemovePartRequested: {
                root.model.removePart(model.index)
            }

            onCopyPartRequested: {
                root.model.copyPart(model.index)
            }

            onRemoveSelectionRequested: {
                root.model.removeSelectedParts()
            }
        }
    }
}
