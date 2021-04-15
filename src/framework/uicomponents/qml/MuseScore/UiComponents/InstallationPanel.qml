/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
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

    QtObject {
        id: privateProperties

        property var currentOperationButton: undefined
    }

    function setProgress(status, indeterminate, current, total) {
        if (!Boolean(privateProperties.currentOperationButton)) {
            return
        }

        privateProperties.currentOperationButton.setProgress(status, indeterminate, current, total)
    }

    function resetProgress() {
        if (!Boolean(privateProperties.currentOperationButton)) {
            return
        }

        privateProperties.currentOperationButton.resetProgress()
    }

    content: Column {
        anchors.fill: parent
        anchors.topMargin: 44
        anchors.leftMargin: 68
        anchors.rightMargin: 68
        anchors.bottomMargin: 42

        spacing: 42

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
                text: Boolean(root.neutralButtonTitle) ? root.neutralButtonTitle : ""

                onClicked: {
                    privateProperties.currentOperationButton = undefined
                    root.neutralButtonClicked()
                }
            }

            Row {
                Layout.alignment: Qt.AlignRight

                spacing: 19

                ProgressButton {
                    id: updateButton

                    visible: root.hasUpdate

                    text: qsTrc("uicomponents", "Update available")

                    onClicked: {
                        privateProperties.currentOperationButton = updateButton
                        root.updateRequested()
                    }
                }

                FlatButton {
                    visible: root.installed || root.hasUpdate

                    text: qsTrc("uicomponents", "Restart")

                    onClicked: {
                        privateProperties.currentOperationButton = undefined
                        root.restartRequested()
                    }
                }

                ProgressButton {
                    id: installButton

                    visible: !root.installed && !root.hasUpdate

                    text: qsTrc("uicomponents", "Install")

                    onClicked: {
                        privateProperties.currentOperationButton = installButton
                        root.installRequested()
                    }
                }

                FlatButton {
                    id: uninstallButton

                    visible: root.installed || root.hasUpdate

                    text: qsTrc("uicomponents", "Uninstall")

                    onClicked: {
                        privateProperties.currentOperationButton = undefined
                        root.uninstallRequested()
                    }
                }
            }
        }
    }
}
