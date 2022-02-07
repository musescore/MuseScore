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

import "internal"

InfoPanel {
    id: root

    property string neutralButtonTitle: ""

    property bool installed: false
    property bool hasUpdate: false
    property bool needRestart: true

    signal installRequested()
    signal updateRequested()
    signal uninstallRequested()
    signal restartRequested()
    signal neutralButtonClicked()

    buttonsPanel: RowLayout {
        id: buttons

        spacing: 19

        function updateMainButton() {
            var button = null
            if (updateButton.visible) {
                button = updateButton
            } else if (installButton.visible) {
                button = installButton
            } else {
                button = uninstallButton
            }

            root.mainButton = button
        }

        FlatButton {
            id: neutralButton
            Layout.alignment: Qt.AlignLeft

            navigation.name: "NeutralButton"
            navigation.panel: root.contentNavigation
            navigation.column: 1

            text: Boolean(root.neutralButtonTitle) ? root.neutralButtonTitle : ""

            onClicked: {
                root.currentOperationButton = undefined
                root.neutralButtonClicked()
            }
        }

        Row {
            Layout.alignment: Qt.AlignRight

            spacing: 19

            ProgressButton {
                id: updateButton

                visible: root.hasUpdate
                onVisibleChanged: {
                    buttons.updateMainButton()
                }

                navigationName: "UpdateButton"
                navigationPanel: root.contentNavigation
                navigationColumn: 2

                text: qsTrc("uicomponents", "Update available")

                onClicked: {
                    root.currentOperationButton = updateButton
                    root.updateRequested()
                }
            }

            FlatButton {
                visible: (root.installed || root.hasUpdate) && root.needRestart
                onVisibleChanged: {
                    buttons.updateMainButton()
                }

                navigation.name: "RestartButton"
                navigation.panel: root.contentNavigation
                navigation.column: 3

                text: qsTrc("uicomponents", "Restart")

                onClicked: {
                    root.currentOperationButton = undefined
                    root.restartRequested()
                }
            }

            ProgressButton {
                id: installButton


                visible: root.installed && !root.hasUpdate
                onVisibleChanged: {
                    buttons.updateMainButton()
                }

                navigationName: "InstallButton"
                navigationPanel: root.contentNavigation
                navigationColumn: 4

                text: qsTrc("uicomponents", "Install")

                onClicked: {
                    root.currentOperationButton = installButton
                    root.installRequested()
                }
            }

            FlatButton {
                id: uninstallButton

                visible: root.installed || root.hasUpdate
                onVisibleChanged: {
                    buttons.updateMainButton()
                }

                navigation.name: "UninstallButton"
                navigation.panel: root.contentNavigation
                navigation.column: 5

                text: qsTrc("uicomponents", "Uninstall")

                onClicked: {
                    root.currentOperationButton = undefined
                    root.uninstallRequested()
                }
            }
        }
    }
}
