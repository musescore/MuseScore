/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.InstrumentsScene

Item {
    id: root

    required property AbstractLayoutPanelTreeItem item

    readonly property int type: root.item ? root.item.type : LayoutPanelItemType.UNDEFINED
    readonly property bool isOpened: Boolean(popupLoader.openedPopup) && popupLoader.openedPopup.isOpened

    // System markings rows keep their cog enabled when there's nothing to configure yet: it then explains them
    readonly property bool showsSystemObjectsHelp: root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER
                                                   && Boolean(root.item) && !root.item.settingsEnabled

    function toggle(button: FlatButton) {
        if (root.isOpened) {
            popupLoader.openedPopup.close()
            return
        }

        let comp = null
        let params = {}

        if (root.type === LayoutPanelItemType.PART) {
            comp = instrumentSettingsComp

            params["partId"] = root.item.id
            params["instrumentId"] = (root.item as PartTreeItem).instrumentId()
        } else if (root.type === LayoutPanelItemType.STAFF) {
            comp = staffSettingsComp

            params["id"] = root.item.id
        } else if (root.showsSystemObjectsHelp) {
            comp = systemObjectsLayerHelpComp
        } else if (root.type === LayoutPanelItemType.SYSTEM_OBJECTS_LAYER) {
            comp = systemObjectsLayerSettingsComp

            params["staffId"] = (root.item as SystemObjectsLayerTreeItem).staffId()
        }

        popupLoader.open(comp, button, params)
    }

    Loader {
        id: popupLoader

        readonly property StyledPopupView openedPopup: popupLoader.item as StyledPopupView

        function open(comp: Component, button: FlatButton, params) {
            popupLoader.sourceComponent = comp
            if (!popupLoader.openedPopup) {
                return
            }

            popupLoader.openedPopup.parent = button
            popupLoader.openedPopup.needActiveFirstItem = button.navigation.highlight

            popupLoader.openedPopup.load(params)

            popupLoader.openedPopup.closed.connect(function() {
                popupLoader.sourceComponent = null
            })

            popupLoader.openedPopup.open()
        }
    }

    Component {
        id: instrumentSettingsComp

        InstrumentSettingsPopup {
            onReplaceInstrumentRequested: {
                // The popup would close when the dialog to select the new
                // instrument is shown; when it closes, it is unloaded, i.e.
                // deleted, which means that it is deleted while a signal
                // handler inside it is being executed. This causes a crash.
                // To prevent that, let the popup close itself, and perform the
                // actual operation "later", i.e. not (directly or indirectly)
                // inside the signal handler in the popup.
                Qt.callLater((root.item as PartTreeItem).replaceInstrument)
            }

            onResetAllFormattingRequested: {
                // Same as above
                Qt.callLater((root.item as PartTreeItem).resetAllFormatting)
            }
        }
    }

    Component {
        id: staffSettingsComp

        StaffSettingsPopup {}
    }

    Component {
        id: systemObjectsLayerSettingsComp

        SystemObjectsLayerSettingsPopup {}
    }

    // Shown instead of the system markings settings while the score has no system markings to configure
    Component {
        id: systemObjectsLayerHelpComp

        SystemObjectsLayerHelpPopup {}
    }
}
