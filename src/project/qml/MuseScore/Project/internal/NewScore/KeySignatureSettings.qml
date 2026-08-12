/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

PopupButton {
    id: root

    property var model: null
    property string mode: "major"
    property string currentValueAccessibleName: title.text

    height: 96

    KeySignature {
        id: title
        icon: root.model.keySignature.icon
        text: root.mode === "major" ? root.model.keySignature.titleMajor : root.model.keySignature.titleMinor
    }

    popupComponent: StyledPopupView {
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

                currentIndex: root.mode === "major" ? 0 : 1

                onCurrentIndexChanged: {
                    root.mode = currentIndex === 0 ? "major" : "minor"
                }

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
