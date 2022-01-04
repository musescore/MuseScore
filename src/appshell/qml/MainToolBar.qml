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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    width: radioButtonList.width
    height: radioButtonList.height

    property alias navigation: navCtrl

    property string currentUri: "musescore://home"
    property var items: [
        {
            title: qsTrc("appshell", "Home"),
            uri: "musescore://home"
        },
        {
            title: qsTrc("appshell", "Score"),
            uri: "musescore://notation"
        },
        {
            title: qsTrc("appshell", "Publish"),
            uri: "musescore://publish"
        },
        {
            title: qsTrc("appshell", "DevTools"),
            uri: "musescore://devtools"
        }
    ]

    signal selected(string uri)

    function select(uri) {
        root.selected(uri)
    }

    NavigationPanel {
        id: navCtrl
        name: "MainToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Main tool bar") + " " + navCtrl.directionInfo
    }

    RadioButtonGroup {
        id: radioButtonList
        spacing: 0

        model: root.items

        width: contentItem.childrenRect.width
        height: contentItem.childrenRect.height

        delegate: PageTabButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            spacing: 0
            leftPadding: 12

            navigation.name: modelData["title"]
            navigation.panel: navCtrl
            navigation.order: model.index

            checked: modelData["uri"] === root.currentUri
            title: modelData["title"]

            onToggled: {
                root.selected(modelData["uri"])
            }
        }
    }
}
