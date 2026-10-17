/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.Inspector
import MuseScore.NotationScene

import "../../common"

AbstractElementPopup {
    id: root

    //! Note: the navigation order does not match the order of the components in the file
    property alias notationViewNavigationSection: textStyleSettingsNavPanel.section
    property alias navigationOrderStart: textStyleSettingsNavPanel.order
    readonly property alias navigationOrderEnd: textStyleSettingsNavPanel.order

    readonly property int elementSize: 30
    readonly property int elementSpacing: 4
    readonly property int controlSpacing: 12
    readonly property int rowSpacing: 8

    contentWidth: contentRow.implicitWidth
    contentHeight: contentRow.implicitHeight

    margins: root.controlSpacing

    showArrow: false

    focusPolicies: PopupView.DefaultFocus & ~PopupView.ClickFocus
    placementPolicies: textStyleModel.placeAbove ? PopupView.PreferAbove : PopupView.PreferBelow

    model: TextStylePopupModel {
        id: textStyleModel
    }

    function updatePosition() {
        root.x = 0
        Qt.callLater(root.repositionWindowIfNeed)
    }

    RowLayout {
        id: contentRow
        spacing: root.controlSpacing

        NavigationPanel {
            id: textStyleSettingsNavPanel
            name: "TextStyleSettings"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("inspector", "Text style settings buttons")
        }

        TextStyleFormatting {
            id:formatting
            settingsModel:textStyleModel.textSettingsModel
            navigationPanel:textStyleSettingsNavPanel
            styleNavigationStart:textStylePopupButton.navigation.row+1
        }

        RowLayout {
            spacing: root.rowSpacing

            FlatButton { // 4
                id: addSymbolsButton

                Layout.preferredWidth: 3 * root.elementSize
                Layout.fillHeight: true

                navigation.name: "addSymbolsButton"
                navigation.panel: textStyleSettingsNavPanel
                navigation.row: formatting.verticalNavigationEnd + 1

                icon: IconCode.FLAT
                text: qsTrc("inspector", "Add symbols")
                maximumLineCount: 2

                visible: textStyleModel.textSettingsModel.isSpecialCharactersInsertionAvailable

                onClicked: {
                    textStyleModel.textSettingsModel.insertSpecialCharacters();
                }
            }

            ColumnLayout {
                spacing: root.rowSpacing
                Layout.preferredWidth: 3 * root.elementSize

                FlatButton { // 5
                    id: textStylePopupButton

                    Layout.fillWidth: true

                    navigation.name: "textStylePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: addSymbolsButton.navigation.row + 1

                    text: qsTrc("inspector", "Text style")

                    onClicked: {
                        textStyleSubPopup.toggleOpened();
                    }

                    TextStyleSubPopup {
                        id: textStyleSubPopup

                        textSettingsModel: textStyleModel.textSettingsModel
                    }
                }

                FlatButton { // 10
                    id: framePopupButton

                    Layout.fillWidth: true

                    navigation.name: "framePopupButton"
                    navigation.panel: textStyleSettingsNavPanel
                    navigation.row: formatting.lineNavigationRow + 1

                    visible: !textStyleModel.textSettingsModel.isDynamicSpecificSettings

                    text: qsTrc("inspector", "Frame")

                    onClicked: {
                        frameSubPopup.toggleOpened();
                    }

                    FrameSubPopup {
                        id: frameSubPopup

                        textSettingsModel: textStyleModel.textSettingsModel
                    }
                }
            }
        }
    }
}
