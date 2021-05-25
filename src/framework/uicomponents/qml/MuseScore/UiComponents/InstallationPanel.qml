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
import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0

PopupPanel {
    id: root

    property string title
    property var additionalInfoModel
    property string description
    property string neutralButtonTitle

    property bool installed: false
    property bool hasUpdate: false

    signal installRequested()
    signal updateRequested()
    signal uninstallRequested()
    signal restartRequested()
    signal neutralButtonClicked()

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    height: 360

    visible: false

    accessible.name: root.title

    QtObject {
        id: prv

        property var currentOperationButton: undefined
    }

    function setProgress(status, indeterminate, current, total) {
        if (!Boolean(prv.currentOperationButton)) {
            return
        }

        prv.currentOperationButton.setProgress(status, indeterminate, current, total)
    }

    function resetProgress() {
        if (!Boolean(prv.currentOperationButton)) {
            return
        }

        prv.currentOperationButton.resetProgress()
    }

    content: Column {
        anchors.fill: parent
        anchors.topMargin: 44
        anchors.leftMargin: 68
        anchors.rightMargin: 68
        anchors.bottomMargin: 42

        spacing: 42

        property bool opened: root.visible
        onOpenedChanged: {
            if (opened) {
                focusOnOpened()
            }
        }

        function focusOnOpened() {
            if (updateButton.visible) {
                updateButton.navigation.requestActive()
            } else {
                installButton.navigation.requestActive()
            }
        }

        Column {
            width: 585

            spacing: 8

            StyledTextLabel {
                font: ui.theme.headerBoldFont

                text: Boolean(root.title) ? root.title : ""
            }

            Row {
                anchors.left: parent.left
                anchors.right: parent.right

                spacing: 4

                visible: Boolean(additionalInfoModel)

                Repeater {
                    model: additionalInfoModel
                    Row {
                        spacing: 4
                        Rectangle {
                            width: 2
                            height: parent.height - 4
                            anchors.verticalCenter: parent.verticalCenter
                            color: ui.theme.fontPrimaryColor

                            visible: index !== 0
                        }

                        StyledTextLabel {
                            font: ui.theme.largeBodyFont
                            text: modelData.title
                        }

                        StyledTextLabel {
                            font: ui.theme.largeBodyBoldFont
                            text: modelData.value
                        }
                    }
                }
            }
        }

        StyledTextLabel {
            width: 585
            height: 88

            opacity: 0.75
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft

            text: Boolean(root.description) ? root.description : ""
            visible: Boolean(root.description)
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 19

            FlatButton {
                id: openFullDescriptionButton
                Layout.alignment: Qt.AlignLeft

                navigation.name: "openFullDescription"
                navigation.panel: root.navigation
                navigation.column: 1

                text: Boolean(root.neutralButtonTitle) ? root.neutralButtonTitle : ""

                onClicked: {
                    prv.currentOperationButton = undefined
                    root.neutralButtonClicked()
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 19

                ProgressButton {
                    id: updateButton

                    visible: root.hasUpdate

                    navigation.name: "Update"
                    navigation.panel: root.navigation
                    navigation.column: 2

                    text: qsTrc("uicomponents", "Update available")

                    onClicked: {
                        prv.currentOperationButton = updateButton
                        root.updateRequested()
                    }
                }

                FlatButton {
                    visible: root.installed || root.hasUpdate

                    navigation.name: "Restart"
                    navigation.panel: root.navigation
                    navigation.column: 3

                    text: qsTrc("uicomponents", "Restart")

                    onClicked: {
                        prv.currentOperationButton = undefined
                        root.restartRequested()
                    }
                }

                ProgressButton {
                    id: installButton

                    visible: !root.installed && !root.hasUpdate

                    navigation.name: "Install"
                    navigation.panel: root.navigation
                    navigation.column: 4

                    text: qsTrc("uicomponents", "Install")

                    onClicked: {
                        prv.currentOperationButton = installButton
                        root.installRequested()
                    }
                }

                FlatButton {
                    id: uninstallButton

                    visible: root.installed || root.hasUpdate

                    navigation.name: "Uninstall"
                    navigation.panel: root.navigation
                    navigation.column: 5

                    text: qsTrc("uicomponents", "Uninstall")

                    onClicked: {
                        prv.currentOperationButton = undefined
                        root.uninstallRequested()
                    }
                }
            }
        }
    }
}
