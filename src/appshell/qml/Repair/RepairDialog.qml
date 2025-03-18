/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Preferences 1.0
import MuseScore.Repair 1.0

import "../Preferences/internal"
import "internal"

StyledDialogView {
    id: root

    title: qsTrc("appshell/repair", "Repair")

    contentWidth: 880
    contentHeight: 640
    resizable: true

    property string currentPageId: ""
    property var params: null

    property QtObject prv: QtObject {
        property var pagesObjects: (new Map())

        function resolveStackCurrentIndex() {
            var keys = Object.keys(root.prv.pagesObjects)
            return keys.indexOf(repairModel.currentPageId)
        }

        function updateStackCurrentIndex() {
            stack.currentIndex = resolveStackCurrentIndex()
        }
    }

    Component.onCompleted: {
        repairModel.load(root.currentPageId)

        initPagesObjects()

        prv.updateStackCurrentIndex()
    }

    function initPagesObjects() {
        var pages = repairModel.availablePages()
        for (var i in pages) {
            var pageInfo = pages[i]

            if (!Boolean(pageInfo.path)) {
                continue
            }

            var pageComponent = Qt.createComponent("../" + pageInfo.path)

            var properties = {
                navigationSection: root.navigationSection,
                navigationOrderStart: (i + 1) * 100
            }

            if (root.currentPageId === pageInfo.id) {
                var params = root.params
                for (var key in params) {
                    var value = params[key]
                    properties[key] = value
                }
            }

            var obj = pageComponent.createObject(stack, properties)

            if (!Boolean(obj)) {
                continue
            }

            obj.hideRequested.connect(function() {
                root.hide()
            })

            root.prv.pagesObjects[pageInfo.id] = obj
        }
    }

// This is the "repairModel" - and it holds together some of the other components of this dialog.
// Note that the term repairModel is also used elsewhere in this document.
// This one has been commented out in order to make room for the RepairModel - which was created
// as a copy of PreferencesModel and then changed.
/*
    PreferencesModel {
        id: repairModel

        onCurrentPageIdChanged: function(currentPageId) {
            prv.updateStackCurrentIndex()
        }
    }
*/
    RepairModel {
        id: repairModel

        onCurrentPageIdChanged: function(currentPageId) {
            prv.updateStackCurrentIndex()
        }
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 0

            // RepairMenu
            // This is the piece that stacks all of the different selectable menu items. Seemingly this
            // Is what needs to be modified to change the number of selectable menu tabs.
            // If removed, the languages tab shows up and there is no way to switch to a different tab.
            RepairMenu {
                id: menu

                Layout.fillHeight: true
                Layout.preferredWidth: 220

                navigation.section: root.navigationSection
                navigation.order: 1

                model: repairModel
            } // END RepairMenu

            SeparatorLine { orientation: Qt.Vertical }

            StackLayout {
                id: stack
            }
        }

        SeparatorLine { }

        // OK/Cancel Button Panel
        // This is a section at the bottom of the dialog that houses the
        // OK and Cancel buttons.
        PreferencesButtonsPanel {
            id: buttonsPanel

            Layout.fillWidth: true
            Layout.preferredHeight: 70

            navigation.section: root.navigationSection
            navigation.order: 100000

            onRevertFactorySettingsRequested: {
                if (!repairModel.askForConfirmationOfPreferencesReset()) {
                    return;
                }

                var pages = repairModel.availablePages()

                for (var i in pages) {
                    var page = pages[i]
                    var obj = root.prv.pagesObjects[page.id]
                    obj.reset()
                }

                repairModel.resetFactorySettings()
            }

            onRejectRequested: {
                repairModel.cancel()
                root.reject()
            }

            onApplyRequested: {
                repairModel.apply()

                var ok = true
                var pages = repairModel.availablePages()

                for (var i in pages) {
                    var page = pages[i]
                    var obj = root.prv.pagesObjects[page.id]
                    ok &= obj.apply()
                }

                if (ok) {
                    root.hide()
                }
            }
        }
        // END OK/Cancel Button Panel
    }
}
