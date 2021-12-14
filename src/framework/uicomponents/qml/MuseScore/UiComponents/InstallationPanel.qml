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

PopupPanel {
    id: root

    property string title: ""
    property var additionalInfoModel: undefined
    property string description: ""
    property string neutralButtonTitle: ""

    property bool installed: false
    property bool hasUpdate: false
    property bool needRestart: true

    signal installRequested()
    signal updateRequested()
    signal uninstallRequested()
    signal restartRequested()
    signal neutralButtonClicked()

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
        id: content
        anchors.fill: parent
        anchors.topMargin: 44
        anchors.leftMargin: 68
        anchors.rightMargin: 68
        anchors.bottomMargin: 42

        spacing: 42

        property var mainButton: updateButton.visible ? updateButton : (installButton.visible ? installButton : uninstallButton)

        property bool opened: root.visible
        onOpenedChanged: {
            if (opened) {
                Qt.callLater(focusOnOpened)
            } else {
                accessibleInfo.focused = false
            }
        }

        function focusOnOpened() {
            mainButton.navigation.requestActive()
            accessibleInfo.focused = true
        }

        property NavigationPanel navigationPanel: NavigationPanel {
            name: root.objectName != "" ? root.objectName : "PopupPanel"

            enabled: root.visible
            section: root.navigationSection
            order: 1

            onActiveChanged: function(active) {
                if (active) {
                    root.forceActiveFocus()
                }
            }
        }

        AccessibleItem {
            id: accessibleInfo
            accessibleParent: root.accessible
            visualItem: root
            role: MUAccessible.Information
            name: {
                var text = root.title + "."

                if (Boolean(root.additionalInfoModel)) {
                    for (var i = 0; i < root.additionalInfoModel.length; i++) {
                        text += " " + root.additionalInfoModel[i].title + " " + root.additionalInfoModel[i].value + ". "
                    }
                }

                text += root.description + ". " + content.mainButton.text

                return text
            }
        }

        Column {
            width: 585

            spacing: 8

            StyledTextLabel {
                id: titleLabel

                text: Boolean(root.title) ? root.title : ""
                font: ui.theme.headerBoldFont
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
                id: neutralButton
                Layout.alignment: Qt.AlignLeft

                navigation.name: "NeutralButton"
                navigation.panel: content.navigationPanel
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

                    navigation.name: "UpdateButton"
                    navigation.panel: content.navigationPanel
                    navigation.column: 2

                    text: qsTrc("uicomponents", "Update available")

                    onClicked: {
                        prv.currentOperationButton = updateButton
                        root.updateRequested()
                    }
                }

                FlatButton {
                    visible: (root.installed || root.hasUpdate) && root.needRestart

                    navigation.name: "RestartButton"
                    navigation.panel: content.navigationPanel
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

                    navigation.name: "InstallButton"
                    navigation.panel: content.navigationPanel
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

                    navigation.name: "UninstallButton"
                    navigation.panel: content.navigationPanel
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
