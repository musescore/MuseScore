/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Preferences

import "internal"

StyledDialogView {
    id: root

    title: qsTrc("preferences", "Preferences")

    contentWidth: 880
    contentHeight: 640
    resizable: true

    property string currentPageId: ""
    property var params: null

    property QtObject prv: QtObject {
        property var pages: []

        function pageIndex(pageId) {
            return root.prv.pages.findIndex(page => page.id === pageId)
        }

        function loadedPages() {
            const result = []
            for (let i = 0; i < pagesRepeater.count; ++i) {
                const loader = pagesRepeater.itemAt(i)
                if (loader && loader.item) {
                    result.push(loader.item)
                }
            }

            return result
        }
    }

    Component.onCompleted: {
        preferencesModel.load(root.currentPageId)
        root.prv.pages = preferencesModel.availablePages()
    }

    onNavigationActivateRequested: {
        menu.focusOnCurrentPage()
    }

    PreferencesModel {
        id: preferencesModel

        onUserAgreesToResetPreferences: {
            for (const page of root.prv.loadedPages()) {
                page.reset()
            }

            Qt.callLater(preferencesModel.resetFactorySettings)
        }
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

                navigation.section: root.navigationSection
                navigation.order: 1

                model: preferencesModel
            }

            SeparatorLine { orientation: Qt.Vertical }

            StackLayout {
                id: stack

                currentIndex: root.prv.pageIndex(preferencesModel.currentPageId)

                Repeater {
                    id: pagesRepeater

                    model: root.prv.pages

                    Loader {
                        id: pageLoader

                        required property var modelData
                        required property int index

                        readonly property bool isCurrent: pageLoader.modelData.id === preferencesModel.currentPageId

                        function loadIfCurrent() {
                            if (!pageLoader.isCurrent || pageLoader.item || !pageLoader.modelData.path) {
                                return
                            }

                            const properties = {
                                navigationSection: root.navigationSection,
                                navigationOrderStart: (pageLoader.index + 1) * 100
                            }

                            if (root.currentPageId === pageLoader.modelData.id) {
                                for (const key in root.params) {
                                    properties[key] = root.params[key]
                                }
                            }

                            pageLoader.setSource("../" + pageLoader.modelData.path, properties)
                        }

                        onIsCurrentChanged: {
                            pageLoader.loadIfCurrent()
                        }

                        Component.onCompleted: {
                            pageLoader.loadIfCurrent()
                        }

                        Connections {
                            target: pageLoader.item
                            ignoreUnknownSignals: true

                            function onHideRequested() {
                                root.hide()
                            }
                        }
                    }
                }
            }
        }

        SeparatorLine { }

        PreferencesButtonsPanel {
            id: buttonsPanel

            Layout.fillWidth: true
            Layout.preferredHeight: 70

            navigation.section: root.navigationSection
            navigation.order: 100000

            onRevertFactorySettingsRequested: {
                preferencesModel.askForConfirmationOfPreferencesReset()
            }

            onRejectRequested: {
                preferencesModel.cancel()
                root.reject()
            }

            onApplyRequested: {
                preferencesModel.apply()

                let ok = true
                for (const page of root.prv.loadedPages()) {
                    ok &= page.apply()
                }

                if (ok) {
                    root.hide()
                }
            }
        }
    }
}
