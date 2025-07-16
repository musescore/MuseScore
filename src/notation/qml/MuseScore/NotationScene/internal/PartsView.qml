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

    property string filterText: ""

    property alias navigationPanel: view.navigationPanel

    QtObject {
        id: prv

        readonly property int sideMargin: 36
        property string currentItemNavigationName: ""
    }

    // Timer pour appliquer le filtre après un court délai (debounce)
    Timer {
        id: debounceTimer
        interval: 300
        repeat: false
        onTriggered: {
            root.filterText = searchField.text
        }
    }

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: prv.sideMargin

        spacing: 16

        // New part search field
        TextInputField {
            id: searchField
            placeholderText: qsTr("Search part...")
            text: root.filterText
            onTextChanged: debounceTimer.restart()
            Layout.fillWidth: true
        }

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

        // Apply the filter to the model
        model: root.model && root.filterText.length > 0
            ? root.model.filter(function(part) {
                if (!part.title)
                    return false;

                // Remove accents from the title for comparison
                function removeAccents(str) {
                    return str.normalize("NFD").replace(/[\u0300-\u036f]/g, "");
                }

                // Detect if a term has accents
                function hasAccent(str) {
                    return /[àâäéèêëîïôöùûüç]/i.test(str);
                }

                let title = part.title.toLowerCase();
                let titleNoAccents = removeAccents(title);
                let terms = root.filterText.toLowerCase().trim().split(/\s+/);

                return terms.every(function(term) {
                    if (hasAccent(term)) {
                        // If the term has accents, search in the title with accents
                        return title.indexOf(term) !== -1;
                    } else {
                        // If the term has no accents, search in the title terms with and without accents
                        return titleNoAccents.indexOf(term) !== -1;
                    }
                });
            })
            : root.model


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
