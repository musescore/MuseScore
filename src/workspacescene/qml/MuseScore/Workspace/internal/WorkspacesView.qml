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
import QtQuick.Controls 2.2

import Muse.Ui 1.0
import Muse.UiComponents 1.0

RadioButtonGroup {
    id: root

    property string firstWorkspaceTitle: Boolean(prv.selectedWorkspace) ? prv.selectedWorkspace.title : ""

    property int leftPadding: 0

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "WorkspacesViewPanel"
        direction: NavigationPanel.Vertical

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlIndex", prv.currentItemNavigationIndex)
                //! NOTE If we removed workspace, then control with saved index may not be
                event.setData("controlOptional", true)
            }
        }
    }

    spacing: 0
    clip: true
    orientation: Qt.Vertical

    interactive: height < contentHeight

    function focusOnSelected() {
        if (prv.selectedWorkspace) {
            prv.selectedWorkspace.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        property var currentItemNavigationIndex: []

        property var selectedWorkspace: {
            for (var i = 0; i < root.count; i++) {
                var item = root.itemAtIndex(i)
                if (item && item.isSelected) {
                    return item
                }
            }
        }
    }

    Connections {
        target: root.model

        function onSelectedWorkspaceChanged(selectedWorkspace) {
            if (Boolean(selectedWorkspace)) {
                root.positionViewAtIndex(selectedWorkspace.index, ListView.Contain)
            }
        }
    }

    delegate: ListItemBlank {
        property string title: model.name

        width: ListView.view.width
        height: 46

        isSelected: model.isSelected

        navigation.panel: root.navigationPanel
        navigation.row: model.index
        navigation.accessible.ignored: true
        navigation.accessible.name: title
        navigation.onActiveChanged: {
            if (!navigation.active) {
                navigation.accessible.ignored = false
            } else {
                positionViewAtIndex(model.index, ListView.Contain)
                prv.currentItemNavigationIndex = [navigation.row, navigation.column]
            }
        }

        RoundedRadioButton {
            anchors.fill: parent
            leftPadding: root.leftPadding

            ButtonGroup.group: root.radioButtonGroup

            spacing: 12

            text: model.name
            font: model.isSelected ? ui.theme.bodyBoldFont : ui.theme.bodyFont

            checked: model.isSelected

            onClicked: {
                root.model.selectWorkspace(model.index)
            }
        }

        SeparatorLine { anchors.bottom: parent.bottom }

        onClicked: {
            root.model.selectWorkspace(model.index)
        }
    }
}
