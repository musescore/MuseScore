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
import QtQuick 2.9

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property string title: ""
    property string defaultText: ""
    property string info: defaultText
    property bool isMultiLineEdit: false

    property NavigationPanel navigationPanel: null
    property int navigationColumn: 0

    spacing: 10

    StyledTextLabel {
        anchors.left: parent.left
        anchors.right: parent.right

        font: ui.theme.bodyBoldFont
        horizontalAlignment: Text.AlignLeft
        text: title
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right

        sourceComponent: root.isMultiLineEdit ? textAreaComponent : textFieldComponent

        Component {
            id: textAreaComponent

            TextInputArea {
                hint: root.defaultText

                navigation.panel: root.navigationPanel
                navigation.column: root.navigationColumn
                navigation.accessible.name: root.title + " " + currentText

                onTextChanged: function(newTextValue) {
                    root.info = Boolean(newTextValue) ? newTextValue : root.defaultText
                }
            }
        }

        Component {
            id: textFieldComponent

            TextInputField {
                hint: root.defaultText

                navigation.panel: root.navigationPanel
                navigation.column: root.navigationColumn
                navigation.accessible.name: root.title + " " + currentText

                onTextChanged: function(newTextValue) {
                    root.info = Boolean(newTextValue) ? newTextValue : root.defaultText
                }
            }
        }
    }
}

