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
import MuseScore.Preferences 1.0

import "internal"

StyledDialogView {
    id: root

    title: qsTrc("appshell", "Preferences")

    contentWidth: 880
    contentHeight: 600
    resizable: true

    property string currentPageId: ""

    property QtObject privatesProperties: QtObject {
        property var pagesObjects: (new Map())
        property bool inited: false
    }

    Component.onCompleted: {
        preferencesModel.load(root.currentPageId)

        initPagesObjects()

        root.privatesProperties.inited = true
    }

    function initPagesObjects() {
        var pages = preferencesModel.availablePages()
        for (var i in pages) {
            var pageInfo = pages[i]

            var pagePath = Boolean(pageInfo.path) ? pageInfo.path : "Preferences/StubPreferencesPage.qml"
            var pageComponent = Qt.createComponent("../" + pagePath)

            var obj = pageComponent.createObject(stack)

            if (!Boolean(obj)) {
                continue
            }

            obj.hideRequested.connect(function() {
                root.hide()
            })

            root.privatesProperties.pagesObjects[pageInfo.id] = obj
        }
    }

    PreferencesModel {
        id: preferencesModel
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 0

            PreferencesMenu {
                id: menu

                Layout.fillHeight: true
                Layout.preferredWidth: 220

                model: preferencesModel
            }

            SeparatorLine { orientation: Qt.Vertical }

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true

                color: ui.theme.backgroundSecondaryColor

                StackLayout {
                    id: stack

                    anchors.fill: parent
                    anchors.margins: 30

                    currentIndex: {
                        var keys = Object.keys(root.privatesProperties.pagesObjects)
                        return keys.indexOf(preferencesModel.currentPageId)
                    }
                }
            }
        }

        SeparatorLine { }

        PreferencesButtonsPanel {
            id: buttonsPanel

            Layout.fillWidth: true
            Layout.preferredHeight: 70

            onRevertFactorySettingsRequested: {
                preferencesModel.resetFactorySettings()
            }

            onRejectRequested: {
                preferencesModel.cancel()
                root.reject()
            }

            onApplyRequested: {
                preferencesModel.apply()

                var ok = true
                var pages = preferencesModel.availablePages()

                for (var i in pages) {
                    var page = pages[i]
                    var obj = root.privatesProperties.pagesObjects[page.id]
                    ok &= obj.apply()
                }

                if (ok) {
                    root.hide()
                }
            }
        }
    }
}
