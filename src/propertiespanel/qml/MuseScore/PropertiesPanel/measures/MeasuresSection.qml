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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.PropertiesPanel

import "../common"
import "."

PropertiesPanelSection {
    id: root

    required property MeasuresSettingsModel model

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        RowLayout {
            width: parent.width
            spacing: 4

            PropertiesPanelPopupButton {
                id: insertMeasuresPopupButton

                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "InsertMeasures"
                navigation.row: root.navigationRowStart + 1

                text: qsTrc("propertiespanel", "Insert measures")

                popupContent: InsertMeasuresPopup {
                    model: root.model

                    navigationPanel: insertMeasuresPopupButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }
            }

            FlatButton {
                id: deleteButton

                navigation.panel: root.navigationPanel
                navigation.name: "DeleteMeasures"
                navigation.row: insertMeasuresPopupButton.navigation.row + 1

                toolTipTitle: qsTrc("propertiespanel", "Delete selected measures")

                icon: IconCode.DELETE_TANK

                onClicked: {
                    root.model?.deleteSelectedMeasures?.()
                }
            }
        }
    }
}
