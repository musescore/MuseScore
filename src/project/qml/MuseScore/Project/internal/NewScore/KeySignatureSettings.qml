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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

FlatButton {
    id: root

    property var model: null
    readonly property var mode: bar.currentIndex === 0 ? "major" : "minor"
    property string currentValueAccessibleName: title.text

    property alias popupAnchorItem: popup.anchorItem

    height: 96
    accentButton: popup.isOpened

    KeySignature {
        id: title
        icon: model.keySignature.icon
        text: mode === "major" ? model.keySignature.titleMajor : model.keySignature.titleMinor
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }

    StyledPopupView {
        id: popup

        margins: 20

        contentWidth: 684
        contentHeight: 236

        onOpened: {
            majorTab.navigation.requestActive()
        }

        ColumnLayout {
            id: content
            anchors.fill: parent
            spacing: 20

            StyledTabBar {
                id: bar
                Layout.alignment: Qt.AlignHCenter

                onCurrentItemChanged: {
                    if (currentItem && currentItem.navigation) {
                        currentItemNavigationIndex = [currentItem.navigation.row, currentItem.navigation.column]
                    }
                }

                property var currentItemNavigationIndex: []

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "KeySignatureTabPanel"
                    section: popup.navigationSection
                    direction: NavigationPanel.Horizontal
                    order: 1

                    onNavigationEvent: function(event) {
                        if (event.type === NavigationEvent.AboutActive) {
                            event.setData("controlIndex", bar.currentItemNavigationIndex)
                        }
                    }
                }

                StyledTabButton {
                    id: majorTab
                    //: mode of a key signature, not an interval
                    text: qsTrc("project/newscore", "Major", "key signature mode")

                    navigation.name: "MajorTab"
                    navigation.panel: bar.navigationPanel
                    navigation.column: 0
                }

                StyledTabButton {
                    //: mode of a key signature, not an interval
                    text: qsTrc("project/newscore", "Minor", "key signature mode")

                    navigation.name: "MinorTab"
                    navigation.panel: bar.navigationPanel
                    navigation.column: 1
                }
            }

            KeySignatureListView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                model: root.model.keySignatureList()
                currentSignature: root.model.keySignature
                mode: root.mode

                navigationPanel.name: "KeySignaturesPanel"
                navigationPanel.section: popup.navigationSection
                navigationPanel.order: 2

                onSignatureSelected: function(signature) {
                    root.model.keySignature = signature
                }
            }
        }
    }
}
