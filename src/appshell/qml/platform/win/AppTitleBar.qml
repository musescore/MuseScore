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
import MuseScore.AppShell 1.0

import "../"

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    property alias title: titleTextmetrics.text
    property rect titleMoveAreaRect: Qt.rect(titleMoveArea.x, titleMoveArea.y, titleMoveArea.width, titleMoveArea.height)

    property alias windowIsMiximized: systemButtons.windowIsMiximized

    signal showWindowMinimizedRequested()
    signal toggleWindowMaximizedRequested()
    signal closeWindowRequested()

    RowLayout {
        anchors.fill: parent

        spacing: 8

        AppMenuBar {
            id: menu

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.preferredWidth: width
            Layout.preferredHeight: height
        }

        StyledTextLabel {
            id: titleLabel

            Layout.fillWidth: true
            Layout.fillHeight: true

            horizontalAlignment: Text.AlignLeft
            leftPadding: {
                var parentCenterX = parent.width / 2
                var expectedTextCenterX = parentCenterX - titleTextmetrics.width / 2
                if (expectedTextCenterX > x) {
                    return expectedTextCenterX - x
                }

                return 0
            }

            text: titleTextmetrics.elidedText
            textFormat: Text.RichText

            TextMetrics {
                id: titleTextmetrics

                text: qsTrc("appshell", "MuseScore 4")
                font: titleLabel.font
                elide: Qt.ElideRight
                elideWidth: titleLabel.width
            }

            Item {
                id: titleMoveArea

                anchors.fill: parent
            }
        }

        AppSystemButtons {
            id: systemButtons

            Layout.alignment: Qt.RightLeft | Qt.AlignVCenter
            Layout.preferredWidth: width
            Layout.preferredHeight: height

            onShowWindowMinimizedRequested: {
                root.showWindowMinimizedRequested()
            }

            onToggleWindowMaximizedRequested: {
                root.toggleWindowMaximizedRequested()
            }

            onCloseWindowRequested: {
                root.closeWindowRequested()
            }
        }
    }
}
