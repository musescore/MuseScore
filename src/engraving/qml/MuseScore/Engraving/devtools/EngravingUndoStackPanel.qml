/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Engraving

ColumnLayout {
    id: root
    objectName: "DiagnosticEngravingUndoStackPanel"

    spacing: 12

    FlatButton {
        Layout.fillWidth: true

        text: "Reload"
        onClicked: undoStackModel.reload()
    }

    StyledTextLabel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: !treeView.visible
        text: "Undo stack is empty"
    }

    StyledTreeView {
        id: treeView
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: rows > 0

        model: EngravingUndoStackModel {
            id: undoStackModel
        }

        Component.onCompleted: {
            undoStackModel.init();
        }

        columnWidthProvider: function (column) {
            return width;
        }

        delegate: TreeViewDelegate {
            required property var itemData

            contentItem: StyledTextLabel {
                text: itemData.text
                horizontalAlignment: Text.AlignLeft
            }

            background: Rectangle {
                color: itemData.color
                border.color: ui.theme.fontPrimaryColor
                border.width: itemData.isCurrent ? 2 : 0
            }
        }
    }
}
