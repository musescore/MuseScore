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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import MuseScore.Ui 1.0

TabView {
    id: root

    width: parent.width

    readonly property int tabBarHeight: 32

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "TabViewPanel"
        direction: NavigationPanel.Horizontal

        onActiveChanged: {
            if (navigation.active) {
                root.forceActiveFocus()
            }
        }
    }

    function focusOnTab(index) {
        var tabItem = root.getTab(index)
        if (tabItem && tabItem.navigation) {
            tabItem.navigation.requestActive()
        }
    }

    Rectangle {
        id: selectionHighlighting

        y: 4
        x: {
            if (root.currentIndex < 0) {
                return
            }

            root.currentIndex * (root.width / count)
        }

        height: 3
        width: root.width / count
        radius: 2
        color: ui.theme.accentColor

        Behavior on x {
            NumberAnimation {
                duration: 150
            }
        }
    }

    style: TabViewStyle {

        id: style

        frameOverlap: 1

        tab: Item {
            id: tab

            property var tabItem: root.getTab(styleData.index)

            implicitWidth: styleData.availableWidth / count
            implicitHeight: tabBarHeight

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor
                radius: 4
                anchors.margins: 1

                NavigationFocusBorder { navigationCtrl: tab.tabItem.navigation ?? null }

                border.width: ui.theme.borderWidth
                border.color: ui.theme.strokeColor
                opacity: styleData.selected ? ui.theme.buttonOpacityHit : ui.theme.buttonOpacityNormal

                StyledTextLabel {
                    id: titleLabel
                    anchors.fill: parent
                    text: styleData.title
                    font: ui.theme.bodyBoldFont
                }
            }
        }

        frame: Rectangle {
            id: backgroundRect
            anchors.fill: parent
            color: ui.theme.backgroundPrimaryColor
        }
    }
}
