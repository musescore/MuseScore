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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import "internal"

StyledDialogView {
    id: root

    property alias notes: view.notes
    property alias previousReleasesNotes: view.previousReleasesNotes

    contentWidth: 644
    contentHeight: 474

    margins: 22

    onNavigationActivateRequested: {
        buttons.focusOnFirst()
    }

    onAccessibilityActivateRequested: {
        accessibleInfo.readInfo()
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 24

        AccessibleItem {
             id: accessibleInfo

             visualItem: content
             role: MUAccessible.Button
             name: releaseTitleLabel.text + " " + view.notes + " " + buttons.defaultButtonName

             function readInfo() {
                 accessibleInfo.ignored = false
                 accessibleInfo.focused = true
             }

             function resetFocus() {
                 accessibleInfo.ignored = true
                 accessibleInfo.focused = false
             }
         }

        Column {
            Layout.alignment: Qt.AlignTop

            spacing: 8

            StyledTextLabel {
                id: releaseTitleLabel

                text: qsTrc("update", "A new version of MuseScore Studio is available!")
                font: ui.theme.headerBoldFont
            }

            StyledTextLabel {
                id: releaseNotesLabel

                text: qsTrc("update", "Release notes")
                font: ui.theme.largeBodyBoldFont
                horizontalAlignment: Qt.AlignLeft
            }
        }

        SeparatorLine {
            Layout.leftMargin: -root.margins
            Layout.rightMargin: -root.margins
        }

        ReleaseNotesView {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        SeparatorLine {
            Layout.leftMargin: -root.margins
            Layout.rightMargin: -root.margins
        }

        AppReleaseInfoBottomPanel {
            id: buttons

            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height
            Layout.alignment: Qt.AlignBottom

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 1

            onRemindLaterRequested: {
                root.ret = { errcode: 0, value: "remindLater" }
                root.hide()
            }

            onInstallRequested: {
                root.ret = { errcode: 0, value: "install" }
                root.hide()
            }

            onSkipRequested: {
                root.ret = { errcode: 0, value: "skip" }
                root.hide()
            }
        }
    }
}
