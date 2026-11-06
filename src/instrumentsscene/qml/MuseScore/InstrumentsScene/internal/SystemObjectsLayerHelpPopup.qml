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

StyledPopupView {
    id: root

    property bool needActiveFirstItem: false

    contentWidth: 240
    contentHeight: helpLabel.implicitHeight

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "SystemObjectsLayerHelpPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    function load(obj) {}

    onOpened: {
        if (root.needActiveFirstItem) {
            helpNavCtrl.requestActive()
        }
    }

    StyledTextLabel {
        id: helpLabel

        width: parent.width

        text: qsTrc("layoutpanel", "System markings let you choose which staves show elements like tempo markings, "
                                   + "rehearsal marks, system text, voltas, and jumps. Add one or more of these "
                                   + "elements in the score to configure their visibility here.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap

        NavigationControl {
            id: helpNavCtrl

            name: "SystemObjectsLayerHelp"
            panel: root.navigationPanel
            order: 1
            enabled: helpLabel.visible

            accessible.role: MUAccessible.StaticText
            accessible.visualItem: helpLabel
            accessible.name: helpLabel.text
        }

        NavigationFocusBorder {
            navigationCtrl: helpNavCtrl
        }
    }
}
