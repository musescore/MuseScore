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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.height

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 12

        RowLayout {
            width: parent.width
            spacing: 4

            PopupViewButton {
                id: insertMeasuresPopupButton

                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "InsertMeasures"
                navigation.row: root.navigationRowStart + 1

                text: qsTrc("inspector", "Insert measures")

                popupContent: InsertMeasuresPopup {
                    model: root.model

                    navigationPanel: insertMeasuresPopupButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }

                onPopupOpened: {
                    root.popupOpened(insertMeasuresPopupButton.popup)
                }
            }

            FlatButton {
                id: deleteButton

                navigation.panel: root.navigationPanel
                navigation.name: "DeleteMeasures"
                navigation.row: root.navigationRowStart + 1

                toolTipTitle: qsTrc("inspector", "Delete selected measures")

                icon: IconCode.DELETE_TANK

                onClicked: model.deleteSelectedMeasures()
            }
        }
    }
}
