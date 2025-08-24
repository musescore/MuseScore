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

import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 8

        RowLayout {
            width: parent.width
            spacing: 4

            VisibilityBox {
                Layout.fillWidth: true
                isVisible: root.model ? !root.model.hideEmptyStaves : true
                text: qsTrc("inspector", "Empty staves")

                navigation.name: "EmptyStaves"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(1)

                onVisibleToggled: {
                    if (root.model) {
                        root.model.hideEmptyStaves = !!isVisible
                    }
                }
            }

            PopupViewButton {
                id: hideEmptyStavesSettingsPopupButton
                Layout.fillWidth: false
                Layout.minimumWidth: implicitWidth

                icon: IconCode.SETTINGS_COG
                transparent: !isOpened

                anchorItem: root.anchorItem

                navigation.name: "HideEmptyStavesSettings"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(2)
                accessible.name: qsTrc("inspector", "Show options for hiding empty staves")

                popupContent: HideEmptyStavesSettings {
                    model: root.model
                    navigationPanel: hideEmptyStavesSettingsPopupButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }

                onPopupOpened: function(popup, control) {
                    root.popupOpened(popup, control)
                }
            }
        }

        FlatButton {
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "Page settings"
            navigation.row: root.navigationRow(3)

            text: qsTrc("inspector", "Page settings")

            onClicked: {
                if (root.model) {
                    root.model.showPageSettings()
                }
            }
        }

        FlatButton {
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "Style settings"
            navigation.row: root.navigationRow(4)

            text: qsTrc("inspector", "Style settings")

            onClicked: {
                if (root.model) {
                    root.model.showStyleSettings()
                }
            }
        }
    }
}
