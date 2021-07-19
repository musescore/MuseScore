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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

Rectangle {
    id: root

    property alias navigation: navPanel

    color: ui.theme.backgroundPrimaryColor

    NavigationPanel {
        id: navPanel
        name: "PublishToolBar"
        enabled: root.enabled && root.visible
    }

    PublishToolBarModel {
        id: toolBarModel
    }

    Component.onCompleted: {
        toolBarModel.load()
    }

    ListView {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 4

        height: contentItem.childrenRect.height
        width: contentItem.childrenRect.width

        orientation: Qt.Horizontal
        interactive: false

        spacing: 8

        model: toolBarModel

        delegate: FlatButton {
            text: model.title
            icon: model.icon
            //enabled: model.enabled
            toolTipTitle: model.title
            toolTipDescription: model.description
            toolTipShortcut: model.shortcut

            orientation: Qt.Horizontal
            normalStateColor: "transparent"

            navigation.panel: navPanel
            navigation.order: model.index

            iconFont: ui.theme.toolbarIconsFont
            height: 36

            onClicked: {
                toolBarModel.handleAction(model.code)
            }
        }
    }
}
