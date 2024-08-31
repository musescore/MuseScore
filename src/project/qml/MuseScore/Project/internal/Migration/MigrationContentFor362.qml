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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

ColumnLayout {
    id: root
    spacing: 16

    property string appVersion: ""
    property bool isRemapPercussion: true

    function activateNavigation() {
        applyBtn.navigation.requestActive()
        accessibleInfo.readInfo()
    }

    signal isRemapPercussionChangeRequested(bool remapPercussion)
    signal watchVideoRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "MigrationPanel"
        accessible.role: MUAccessible.Dialog
    }

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
        text: qsTrc("project/migration", "This file was last saved in MuseScore %1").arg(root.appVersion)
        font: ui.theme.tabBoldFont
    }

    StyledTextLabel {
        id: headerSubtitle
        text: qsTrc("project/migration", "Please note that the appearance of your score will change due to improvements we have made to default settings for beaming, ties, slurs, system objects and horizontal spacing.")
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignLeft

        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    CheckBox {
        id: percussionOption
        text: qsTrc("project/migration", "Use our new notation and sound mapping for <a href=\"%1\">MDL percussion</a>")
              .arg("https://musescore.org/node/367337") // non-'/en' URL should redirect to translated version, if available
        checked: root.isRemapPercussion

        navigation.panel: root.navigationPanel
        navigation.row: 1

        onClicked: {
            root.isRemapPercussionChangeRequested(!checked)
        }
    }

    FlatButton {
        id: watchVideo

        text: qsTrc("project/migration", "Watch video about changes")

        navigation.panel: root.navigationPanel
        navigation.row: 2

        onClicked: {
            root.watchVideoRequested()
        }
    }
}
