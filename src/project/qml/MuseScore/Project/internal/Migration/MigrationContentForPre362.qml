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
    spacing: 24

    property string appVersion: ""
    property bool isApplyLeland: true
    property bool isApplyEdwin: true
    property bool isRemapPercussion: true

    function activateNavigation()   {
        applyBtn.navigation.requestActive()
        accessibleInfo.readInfo()
    }

    signal isApplyLelandChangeRequested(bool applyLeland)
    signal isApplyEdwinChangeRequested(bool applyEdwin)
    signal isRemapPercussionChangeRequested(bool remapPercussion)
    signal watchVideoRequested()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "MigrationContentPanel"
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

    ColumnLayout {
        id: header
        spacing: 8

        StyledTextLabel {
            id: headerTitle
            font: ui.theme.tabBoldFont
            text: qsTrc("project/migration", "This file was last saved in MuseScore %1").arg(root.appVersion)
        }

        StyledTextLabel {
            id: headerSubtitle
            text: qsTrc("project/migration", "Select the engraving improvements you would like to apply to your score")
        }
    }

    Rectangle {
        id: imageRect
        color: "#F9F9F9"
        height: 178
        radius: 10

        border.width: 1
        border.color: ui.theme.strokeColor

        Layout.fillWidth: true

        Image {
            source: "migration.png"
            height: 158
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: parent
        }
    }

    ColumnLayout {
        id: checkboxes
        spacing: 8

        CheckBox {
            id: lelandOption
            text: qsTrc("project/migration", "Our new notation font, Leland")
            checked: root.isApplyLeland

            navigation.panel: root.navigationPanel
            navigation.row: 1

            onClicked: {
                root.isApplyLelandChangeRequested(!checked)
            }
        }

        CheckBox {
            id: edwinOption
            text: qsTrc("project/migration", "Our new text font, Edwin")
            checked: root.isApplyEdwin

            navigation.panel: root.navigationPanel
            navigation.row: 2

            onClicked: {
                root.isApplyEdwinChangeRequested(!checked)
            }
        }

        CheckBox {
            id: percussionOption
            text: qsTrc("project/migration", "Our new notation and sound mapping for <a href=\"%1\">MDL percussion</a>")
                  .arg("https://musescore.org/node/367337") // non-'/en' URL should redirect to translated version, if available
            checked: root.isRemapPercussion

            navigation.panel: root.navigationPanel
            navigation.row: 3

            onClicked: {
                root.isRemapPercussionChangeRequested(!checked)
            }
        }
    }

    ColumnLayout {
        id: info
        spacing: 12

        StyledTextLabel {
            id: affectedLabel
            text: qsTrc("project/migration", "Please note: score layouts will be affected by improvements to MuseScore Studio")
        }

        FlatButton {
            id: videoButton
            text: qsTrc("project/migration", "Watch video")

            navigation.panel: root.navigationPanel
            navigation.row: 4

            onClicked: {
                root.watchVideoRequested()
            }
        }
    }
}
