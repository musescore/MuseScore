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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ColumnLayout {
    id: root

    spacing: 16

    property string appVersion: ""
    property bool isAskAgain: false

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "MigrationPanel"
        accessible.role: MUAccessible.Dialog
    }

    function activateNavigation() {
        applyBtn.navigation.requestActive()
        accessibleInfo.readInfo()
    }

    signal isAskAgainChangeRequested(bool askAgain)
    signal watchVideoRequested()
    signal access()

    AccessibleItem {
        id: accessibleInfo

        accessibleParent: root.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.Button
        name: headerTitle.text + ". " + headerSubtitle.text + " " + applyBtn.text

        function readInfo() {
            accessibleInfo.ignored = false
            accessibleInfo.focused = true
        }

        function resetFocus() {
            accessibleInfo.ignored = true
            accessibleInfo.focused = false
        }
    }

    StyledTextLabel {
        id: headerTitle

        Layout.fillWidth: true

        font: ui.theme.tabBoldFont

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop

        text: qsTrc("project/migration", "This file was last saved in MuseScore %1").arg(root.appVersion)
    }

    StyledTextLabel {
        id: headerSubtitle

        Layout.fillWidth: true
        Layout.fillHeight: true

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        elide: Text.ElideNone

        text: qsTrc("project/migration", "Please note that the appearance of your score will change due to improvements we have made to default settings for beaming, ties, slurs, system objects and horizontal spacing.")
    }

    CheckBox {
        id: askAgain

        Layout.fillWidth: true

        text: qsTrc("global", "Don’t show this message again")
        checked: !root.isAskAgain

        navigation.panel: navigationPanel
        navigation.row: 3

        onClicked: {
            root.isAskAgainChangeRequested(checked) // not `!checked` because the negation is in `checked: !root.isAskAgain`
        }
    }

    RowLayout {
        FlatButton {
            id: watchVideo

            text: qsTrc("project/migration", "Watch video about changes")

            navigation.panel: root.navigationPanel
            navigation.row: 2

            onClicked: {
                root.watchVideoRequested()
            }
        }

        Item { Layout.fillWidth: true } // spacer

        FlatButton {
            id: applyBtn

            text: qsTrc("global", "OK")

            navigation.panel: root.navigationPanel
            navigation.row: 1
            navigation.accessible.ignored: true
            navigation.onActiveChanged: {
                if (!navigation.active) {
                    accessible.ignored = false
                    accessible.focused = true
                    accessibleInfo.resetFocus()
                }
            }

            onClicked: {
                root.access()
            }
        }
    }
}
