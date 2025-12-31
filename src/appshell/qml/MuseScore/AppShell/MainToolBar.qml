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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

Item {
    id: root

    width: radioButtonList.width
    height: radioButtonList.height

    property alias navigation: navPanel

    property string currentUri: "musescore://home"

    signal selected(string uri)

    function select(uri) {
        root.selected(uri)
    }

    function focusOnFirst() {
        var btn = radioButtonList.itemAtIndex(0) as PageTabButton
        if (btn) {
            btn.navigation.requestActive()
        }
    }

    MainToolBarModel {
        id: toolBarModel
    }

    Component.onCompleted: {
        toolBarModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "MainToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Main toolbar") + " " + navPanel.directionInfo
    }

    RadioButtonGroup {
        id: radioButtonList
        spacing: 0

        model: toolBarModel

        width: Math.max(1, contentItem.childrenRect.width)
        height: Math.max(1, contentItem.childrenRect.height)

        delegate: PageTabButton {
            required property bool isTitleBold
            required title
            required property string uri
            required property int index

            ButtonGroup.group: radioButtonList.radioButtonGroup

            spacing: 0
            leftPadding: 12

            normalStateFont: isTitleBold ? ui.theme.largeBodyBoldFont : ui.theme.largeBodyFont

            navigation.name: title
            navigation.panel: navPanel
            navigation.order: index

            checked: uri === root.currentUri

            onToggled: {
                root.selected(uri)
            }
        }
    }
}
