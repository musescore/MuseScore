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
import QtQuick

import Muse.Ui
import Muse.UiComponents

Column {
    id: root

    property alias currentText: input.currentText
    property alias errorText: errorLabel.text

    property NavigationPanel navigationPanel: null
    property int navigationOrder: 0

    spacing: 8

    function focusOnInput() {
        input.navigation.requestActive()
    }

    StyledTextLabel {
        id: label

        text: qsTrc("project/convert", "Save as")
        horizontalAlignment: Text.AlignLeft
    }

    TextInputField {
        id: input

        width: parent.width

        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrder
        navigation.accessible.name: label.text + " " + input.currentText
        navigation.accessible.description: root.errorText

        onTextChanged: function(newTextValue) {
            input.currentText = newTextValue
        }
    }

    StyledTextLabel {
        id: errorLabel

        width: parent.width

        visible: Boolean(errorLabel.text)
        horizontalAlignment: Text.AlignLeft
    }
}
