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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

FlatButton {
    id: root

    property var model: null
    property var mode: bar.currentIndex === 0 ? "major" : "minor"
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

        padding: 8
        margins: 20

        contentWidth: 688
        contentHeight: 242

        navigationParentControl: root.navigation

        onOpened: {
            majorTab.navigation.requestActive()
        }

        ColumnLayout {
            id: content

            anchors.fill: parent

            TabBar {
                id: bar

                Layout.alignment: Qt.AlignHCenter

                implicitHeight: 28

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "KeySignatureTabPanel"
                    section: popup.navigationSection
                    direction: NavigationPanel.Horizontal
                    order: 1

                    onNavigationEvent: {
                        if (event.type === NavigationEvent.AboutActive) {
                            event.setData("controlIndex", bar.currentItemNavigationIndex)
                        }
                    }
                }

                property var currentItemNavigationIndex: []

                StyledTabButton {
                    id: majorTab
                    text: qsTrc("project", "Major")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 0

                    navigation.name: "MajorTab"
                    navigation.panel: bar.navigationPanel
                    navigation.column: 0
                    onNavigationTriggered: {
                        bar.currentItemNavigationIndex = [navigation.row, navigation.column]
                        bar.currentIndex = 0
                    }
                }

                StyledTabButton {
                    text: qsTrc("appshell", "Minor")
                    sideMargin: 22
                    isCurrent: bar.currentIndex === 1

                    navigation.name: "MinorTab"
                    navigation.panel: bar.navigationPanel
                    navigation.column: 1
                    onNavigationTriggered: {
                        bar.currentItemNavigationIndex = [navigation.row, navigation.column]
                        bar.currentIndex = 1
                    }
                }
            }

            StackLayout {
                id: pagesStack

                Layout.fillHeight: true
                Layout.fillWidth: true

                Layout.topMargin: 20

                currentIndex: bar.currentIndex

                KeySignatureListView {
                    model: root.model.keySignatureList()
                    currentSignature: root.model.keySignature
                    mode: "major"

                    navigationPanel.name: "KeySignatureMajorPanel"
                    navigationPanel.section: popup.navigationSection
                    navigationPanel.order: 2

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }

                KeySignatureListView {
                    model: root.model.keySignatureList()
                    currentSignature: root.model.keySignature
                    mode: "minor"

                    navigationPanel.name: "KeySignatureMinorPanel"
                    navigationPanel.section: popup.navigationSection
                    navigationPanel.order: 2

                    onSignatureSelected: {
                        root.model.keySignature = signature
                    }
                }
            }
        }
    }
}
