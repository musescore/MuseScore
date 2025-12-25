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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents

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

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }

        Connections {
            target: root.model

            function onPartAdded(index) {
                view.positionViewAtIndex(index, ListView.Contain)
                view.currentIndex = index
                Qt.callLater(function() {
                    if (view.currentItem) {
                        view.currentItem.startEditTitle()
                    }
                })
            }
        }

        delegate: PartDelegate {
            required title
            required isSelected
            required property bool isInited
            required property bool isCustom
            required property int index

            sideMargin: prv.sideMargin

            currentPartIndex: view.currentIndex
            canReset: isInited
            canDelete: isCustom

            navigation.name: title + index
            navigation.panel: view.navigationPanel
            navigation.row: index
            navigation.onActiveChanged: {
                if (navigation.active) {
                    prv.currentItemNavigationName = navigation.name
                    view.positionViewAtIndex(index, ListView.Contain)
                }
            }

            onPartClicked: {
                root.model.selectPart(index)
                view.currentIndex = index
            }

            onResetPartRequested: {
                root.model.resetPart(index)
            }

            onRemovePartRequested: {
                root.model.removePart(index)
            }

            onTitleEdited: function(newTitle) {
                incorrectTitleWarning = root.model.validatePartTitle(index, newTitle)
            }

            onTitleEditingFinished: function(newTitle) {
                root.model.setPartTitle(index, newTitle)
            }

            onCopyPartRequested: {
                root.model.copyPart(index)
            }
        }
    }
}
