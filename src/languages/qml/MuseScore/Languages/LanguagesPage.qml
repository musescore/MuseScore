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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    property NavigationSection navigationSection: null

    clip: true

    NavigationPanel {
        id: navPanel
        name: "AddonsLanguages"
        section: root.navigationSection
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Vertical
        accessible.name: qsTrc("languages", "Languages") + navPanel.directionInfo
        order: 8
    }

    QtObject {
        id: prv

        property var selectedLanguage: undefined

        function resetSelectedLanguage() {
            selectedLanguage = undefined
        }
    }

    onSearchChanged: {
        panel.close()
    }

    Component.onCompleted: {
        languageListModel.init()
    }

    LanguageListModel {
        id: languageListModel

        onProgress: function(languageCode, status, indeterminate, current, total) {
            if (prv.selectedLanguage.code !== languageCode) {
                return
            }

            panel.setProgress(status, indeterminate, current, total)
        }
        onFinish: function(item) {
            if (prv.selectedLanguage.code !== item.code) {
                return
            }

            prv.selectedLanguage = item
            panel.resetProgress()
        }
    }

    SortFilterProxyModel {
        id: filterModel

        sourceModel: languageListModel

        filters: [
            FilterValue {
                roleName: "name"
                roleValue: root.search
                compareType: CompareType.Contains
            }
        ]
    }

    Row {
        id: header

        property real itemWidth: parent.width / 2

        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin

        height: 40

        StyledTextLabel {
            width: header.itemWidth

            text: qsTrc("languages", "LANGUAGES")
            horizontalAlignment: Text.AlignLeft
            opacity: 0.5
            font.capitalization: Font.AllUppercase
        }

        StyledTextLabel {
            width: header.itemWidth

            text: qsTrc("languages", "STATUS")
            horizontalAlignment: Text.AlignLeft
            opacity: 0.5
            font.capitalization: Font.AllUppercase
        }
    }

    ListView {
        id: view

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: panel.visible ? panel.top : parent.bottom

        model: filterModel

        clip: true

        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: StyledScrollBar {
            parent: view.parent

            anchors.top: parent.top
            anchors.bottom: panel.visible ? panel.top : parent.bottom
            anchors.right: parent.right

            visible: view.contentHeight > view.height
            z: 1
        }

        delegate: LanguageItem {
            id: item

            width: view.width

            navigation.panel: navPanel
            navigation.row: 1 + model.index
            navigation.onActiveChanged: view.positionViewAtIndex(model.index, ListView.Contain)

            title: model.name
            statusTitle: model.statusTitle

            normalColor: (index % 2 == 0) ? ui.theme.popupBackgroundColor : root.backgroundColor

            headerWidth: header.itemWidth
            sideMargin: root.sideMargin

            onClicked: {
                prv.selectedLanguage = languageListModel.language(model.code)
                panel.open(item.navigation)
            }
        }
    }

    InstallationPanel {
        id: panel

        property alias selectedLanguage: prv.selectedLanguage

        height: 206

        navigation.name: "LanguagesInstallationPanel"

        title: Boolean(selectedLanguage) ? selectedLanguage.name : ""
        installed: Boolean(selectedLanguage) ? (selectedLanguage.status === LanguageStatus.Installed) : false
        hasUpdate: Boolean(selectedLanguage) ? (selectedLanguage.status === LanguageStatus.NeedUpdate) : false
        needRestart: false
        neutralButtonTitle: qsTrc("languages", "Open language preferences")
        background: view

        onSelectedLanguageChanged: {
            panel.resetProgress()
        }

        onInstallRequested: {
            Qt.callLater(languageListModel.install, selectedLanguage.code)
        }

        onUninstallRequested: {
            Qt.callLater(languageListModel.uninstall, selectedLanguage.code)
        }

        onUpdateRequested: {
            Qt.callLater(languageListModel.update, selectedLanguage.code)
        }

        onNeutralButtonClicked: {
            Qt.callLater(languageListModel.openPreferences)
        }

        onClosed: {
            prv.resetSelectedLanguage()
        }
    }
}
