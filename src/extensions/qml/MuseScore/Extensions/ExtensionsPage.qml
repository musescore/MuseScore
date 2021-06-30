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
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    Component.onCompleted: {
        extensionListModel.load()
    }

    QtObject {
        id: prv

        property var selectedExtension: undefined

        function resetSelectedExtension() {
            selectedExtension = undefined
        }
    }

    ExtensionListModel {
        id: extensionListModel

        onProgress: {
            if (prv.selectedExtension.code !== extensionCode) {
                return
            }

            extensionPanel.setProgress(status, indeterminate, current, total)
        }
        onFinish: {
            if (prv.selectedExtension.code !== item.code) {
                return
            }

            prv.selectedExtension = item
            extensionPanel.resetProgress()
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: root.backgroundColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    Flickable {
        id: flickable

        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin
        anchors.bottom: extensionPanel.visible ? extensionPanel.top : parent.bottom

        clip: true

        contentWidth: width
        contentHeight: extensionsColumn.height
        interactive: height < contentHeight

        ScrollBar.vertical: StyledScrollBar {
            parent: flickable.parent

            anchors.top: parent.top
            anchors.bottom: extensionPanel.visible ? extensionPanel.top : parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16

            visible: flickable.contentHeight > flickable.height
            z: 1
        }

        Column {
            id: extensionsColumn

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 20

            ExtensionsListView {
                id: installedView

                anchors.left: parent.left
                anchors.right: parent.right

                title: qsTrc("extensions", "Installed")

                model: extensionListModel
                visible: count > 0

                selectedExtensionCode: Boolean(prv.selectedExtension) ? prv.selectedExtension.code : ""

                filters: [
                    FilterValue {
                        roleName: "name"
                        roleValue: root.search
                        compareType: CompareType.Contains
                    },
                    FilterValue {
                        roleName: "status"
                        roleValue: ExtensionStatus.Installed
                        compareType: CompareType.Equal
                    }
                ]

                onClicked: {
                    prv.selectedExtension = extensionListModel.extension(extension.code)

                    extensionPanel.open()
                }
            }

            ExtensionsListView {
                id: notInstalledView

                anchors.left: parent.left
                anchors.right: parent.right

                title: qsTrc("extensions", "Not Installed")

                model: extensionListModel
                visible: count > 0

                selectedExtensionCode: Boolean(prv.selectedExtension) ? prv.selectedExtension.code : ""

                filters: [
                    FilterValue {
                        roleName: "name"
                        roleValue: root.search
                        compareType: CompareType.Contains
                    },
                    FilterValue {
                        roleName: "status"
                        roleValue: ExtensionStatus.NoInstalled
                        compareType: CompareType.Equal
                    }
                ]

                onClicked: {
                    prv.selectedExtension = extensionListModel.extension(extension.code)

                    extensionPanel.open()
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: flickable.bottom

        visible: !extensionPanel.visible
        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: root.backgroundColor
            }
        }
    }

    onSearchChanged: {
        extensionPanel.close()
    }

    InstallationPanel {
        id: extensionPanel

        property alias selectedExtension: prv.selectedExtension

        title: Boolean(selectedExtension) ? selectedExtension.name : ""
        description: Boolean(selectedExtension) ? selectedExtension.description : ""
        installed: Boolean(selectedExtension) ? (selectedExtension.status === ExtensionStatus.Installed ||
                                                 selectedExtension.status === ExtensionStatus.NeedUpdate) : false
        hasUpdate: Boolean(selectedExtension) ? (selectedExtension.status === ExtensionStatus.NeedUpdate) : false
        neutralButtonTitle: qsTrc("languages", "View full description")
        background: flickable

        onSelectedExtensionChanged: {
            extensionPanel.resetProgress()
        }

        additionalInfoModel: [
            {"title": qsTrc("languages", "Author:"), "value": qsTrc("languages", "MuseScore")},
            {"title": qsTrc("languages", "Maintained by:"), "value": qsTrc("languages", "MuseScore")}
        ]

        onInstallRequested: {
            Qt.callLater(extensionListModel.install, selectedExtension.code)
        }

        onUpdateRequested: {
            Qt.callLater(extensionListModel.update, selectedExtension.code)
        }

        onUninstallRequested: {
            Qt.callLater(extensionListModel.uninstall, selectedExtension.code)
        }

        onNeutralButtonClicked: {
            Qt.callLater(extensionListModel.openFullDescription, selectedExtension.code)
        }

        onClosed: {
            prv.resetSelectedExtension()
        }
    }
}
