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
import MuseScore.PropertiesPanel

import "../common"

PropertiesPanelSection {
    id: root

    required property NotationSettingsProxyModel model

    implicitHeight: grid.implicitHeight

    GridLayout {
        id: grid

        width: parent.width

        columns: 2
        columnSpacing: 4

        Repeater {
            model: root.model ? root.model.models : []

            delegate: PropertiesPanelPopupButton {
                id: button

                required property PropertiesPanelAbstractModel modelData
                required property int index

                popupAvailableWidth: parent ? parent.width : 0
                anchorItem: root.anchorItem

                text: modelData?.title ?? ""
                icon: modelData?.icon ?? IconCode.NONE

                visible: modelData && !modelData.isEmpty

                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRow(index)

                popupContent: NotationSectionLoader {
                    id: loader

                    width: parent.width

                    model: button.modelData

                    navigationPanel: button.popupNavigationPanel

                    Component.onCompleted: {
                        button.navigation.name = loader.viewObjectName
                    }
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }
            }
        }
    }
}
