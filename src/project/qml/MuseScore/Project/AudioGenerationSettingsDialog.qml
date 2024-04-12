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
import MuseScore.Project 1.0

StyledDialogView {
    id: root

    contentHeight: 300
    contentWidth: 500
    margins: 20

    objectName: "AudioGenerationSettingsDialog"

    onNavigationActivateRequested: {
        settingsOptions.focusOnDefaultSettingControl()
    }

    onAccessibilityActivateRequested: {
        accessibleInfo.readInfo()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent

        spacing: 0

        AccessibleItem {
            id: accessibleInfo

            accessibleParent: buttons.navigationPanel.accessible
            visualItem: content
            role: MUAccessible.Button
            name: "%1; %2; %3".arg(titleLabel.text)
                              .arg(subtitleLabel.text)
                              .arg(settingsOptions.defaultSettingControlText)

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
            id: titleLabel

            text: qsTrc("project/save", "Generate MP3 audio for web playback?")
            font: ui.theme.largeBodyBoldFont
        }

        StyledTextLabel {
            id: subtitleLabel

            Layout.fillWidth: true
            Layout.topMargin: 15

            text: qsTrc("project/save", "This could take a few minutes each time you save, depending on the size of your score. These settings can always be changed in Preferences.")
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignLeft
            horizontalAlignment: Text.AlignLeft
        }

        AudioGenerationSettings {
            id: settingsOptions

            Layout.fillWidth: true
            Layout.topMargin: 15

            navigationPanel.order: 1
            navigationPanel.section: root.navigationSection

            onAccessibleInfoResetRequested: {
                accessibleInfo.resetFocus()
            }
        }

        SeparatorLine {
            Layout.topMargin: 30
            Layout.leftMargin: -root.margins
            Layout.rightMargin: -root.margins
        }

        RowLayout {
            id: buttons

            Layout.fillWidth: true
            Layout.topMargin: 20

            property NavigationPanel navigationPanel: NavigationPanel {
                name: "AudioGenerationSettingsButtons"
                direction: NavigationPanel.Horizontal
                section: root.navigationSection
                order: 2
            }

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                accentButton: true
                text: qsTrc("global", "OK")

                navigation.panel: buttons.navigationPanel
                navigation.order: 1

                onClicked: {
                    root.ret = { "errcode": 0 }
                    root.hide()
                }
            }
        }
    }
}
