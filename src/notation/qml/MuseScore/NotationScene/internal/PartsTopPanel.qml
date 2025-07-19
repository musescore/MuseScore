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

Item {
    id: root

    property int sideMargin: 0

    property var model

    // Added search filter
    property string filterText: ""

    // Timer to limit the time between each term written by the user
    Timer {
        id: debounceTimer
        interval: 300
        repeat: false
        onTriggered: {
            root.filterText = searchField.text
        }
    }

    signal createNewPartRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PartsControlPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Horizontal

        //: Accessibility description of the button group at the top of the "Parts" dialog
        accessible.name: qsTrc("notation", "Parts actions")

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: root.sideMargin
        spacing: 8

        StyledTextLabel {
            text: qsTrc("notation", "Parts")
            font: ui.theme.headerBoldFont
            Layout.alignment: Qt.AlignVCenter
        }

        // Search bar for filtering parts
        TextInputField {
            id: searchField
            placeholderText: qsTr("Search part...")
            text: root.filterText
            onTextChanged: debounceTimer.restart()
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            navigation.panel: root.navigationPanel
            navigation.column: 0
        }

        FlatButton {
            text: qsTrc("notation", "Create new part")
            Layout.alignment: Qt.AlignVCenter

            navigation.name: "CreateNewPartButton"
            navigation.panel: root.navigationPanel
            navigation.column: 1

            onClicked: {
                root.createNewPartRequested()
            }
        }
    }

    // Search filter
    function filteredModel() {
        if (root.model && root.filterText.length > 0) {
            return root.model.filter(function(part) {
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
            });
        } else {
            return root.model;
        }
    }
}
