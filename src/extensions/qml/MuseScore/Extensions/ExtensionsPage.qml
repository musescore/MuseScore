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
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Extensions 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    property NavigationSection navigationSection: null

    clip: true

    Component.onCompleted: {
        extensionListModel.load()
    }

    QtObject {
        id: prv

        property var selectedExtension: undefined
        property var lastNavigatedExtension: undefined

        function resetSelectedExtension() {
            selectedExtension = undefined
        }
    }

    ExtensionListModel {
        id: extensionListModel

        onProgress: function(extensionCode, status, indeterminate, current, total) {
            if (!Boolean(prv.selectedExtension) || prv.selectedExtension.code !== extensionCode) {
                return
            }

            extensionPanel.setProgress(status, indeterminate, current, total)
        }
        onFinish: function(item) {
            if (!Boolean(prv.selectedExtension) || prv.selectedExtension.code !== item.code) {
                return
            }

            prv.lastNavigatedExtension = null
            prv.selectedExtension = item
            extensionPanel.resetProgress()
            extensionPanel.close()
        }
    }

    Rectangle {
        id: topGradient

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

    StyledFlickable {
        id: flickable

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin
        anchors.bottom: extensionPanel.visible ? extensionPanel.top : parent.bottom

        contentWidth: width
        contentHeight: extensionsColumn.implicitHeight

        topMargin: topGradient.height
        bottomMargin: 24

        ScrollBar.vertical: StyledScrollBar {
            parent: flickable.parent

            anchors.top: parent.top
            anchors.bottom: extensionPanel.visible ? extensionPanel.top : parent.bottom
            anchors.right: parent.right

            visible: flickable.contentHeight > flickable.height
            z: 1
        }

        Column {
            id: extensionsColumn
            anchors.fill: parent

            spacing: 24

            ExtensionsListView {
                id: installedExtensionsView

                width: parent.width
                title: qsTrc("extensions", "Installed")
                visible: count > 0

                model: extensionListModel

                flickableItem: extensionsColumn

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

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "InstalledExtensions"
                navigationPanel.order: 6

                onClicked: function(index, extension, navigationControl) {
                    prv.selectedExtension = extensionListModel.extension(extension.code)

                    extensionPanel.open()
                    prv.lastNavigatedExtension = navigationControl
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, installedExtensionsView.headerHeight + 16)
                }
            }

            ExtensionsListView {
                id: notInstalledExtensionsView

                width: parent.width
                title: qsTrc("extensions", "Not installed")
                visible: count > 0

                model: extensionListModel

                flickableItem: extensionsColumn

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

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "InstalledExtensions"
                navigationPanel.order: 7

                onClicked: function(index, extension, navigationControl) {
                    prv.selectedExtension = extensionListModel.extension(extension.code)

                    extensionPanel.open()
                    prv.lastNavigatedExtension = navigationControl
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, notInstalledExtensionsView.headerHeight + 16)
                }
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
            Qt.callLater(resetNavigationFocus)
        }

        function resetNavigationFocus() {
            if (prv.lastNavigatedExtension) {
                prv.lastNavigatedExtension.requestActive()
                return
            }

            if (installedExtensionsView.count > 0) {
                installedExtensionsView.focusOnFirst()
            } else {
                notInstalledExtensionsView.focusOnFirst()
            }
        }
    }
}
