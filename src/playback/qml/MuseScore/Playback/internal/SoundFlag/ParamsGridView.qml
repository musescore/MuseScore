/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    property alias title: titleLabel.text

    property alias model: repeaterPreset.model
    property var selectionModel: null

    property bool isTruncated: flickable.contentHeight > prv.flickableMaxHeight

    property bool needAddPaddingForScrollbar: false

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "SoundFlagParams" + title
        direction: NavigationPanel.Vertical
        accessible.name: title
    }

    signal toggleParamRequested(string paramCode)

    spacing: 8

    QtObject {
        id: prv

        property int flickableMaxHeight: 132
    }

    StyledTextLabel {
        id: titleLabel

        width: parent.width

        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap
    }

    StyledFlickable {
        id: flickable

        width: parent.width
        height: Math.min(contentHeight, prv.flickableMaxHeight)

        contentHeight: gridView.implicitHeight

        ScrollBar.vertical: StyledScrollBar {
            id: scrollBar

            padding: 0
            policy: root.isTruncated ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        }

        GridLayout {
            id: gridView

            width: parent.width - (root.needAddPaddingForScrollbar ? scrollBar.width + 8 : 0)

            columns: 2
            rows: Math.ceil(root.model.length / 2)
            columnSpacing: 4
            rowSpacing: 4

            Repeater {
                id: repeaterPreset

                width: parent.width
                height: parent.height

                FlatButton {
                    id: button

                    Layout.preferredWidth: (gridView.width - gridView.rowSpacing) / 2
                    Layout.preferredHeight: implicitHeight

                    text: modelData["name"]
                    accentButton: root.selectionModel.indexOf(modelData["code"]) !== -1

                    drawFocusBorderInsideRect: true

                    navigation.name: "Param" + index
                    navigation.panel: root.navigationPanel
                    navigation.row: index
                    navigation.column: 1
                    navigation.onHighlightChanged: {
                        if (navigation.highlight) {
                            var pos = button.mapToItem(gridView, 0, 0)
                            var rect = Qt.rect(pos.x, pos.y, button.width, button.height)
                            Utils.ensureContentVisible(flickable, rect, 0)
                        }
                    }

                    accessible.name: text + "; " + (accentButton ? qsTrc("global", "On") : qsTrc("global", "Off"))

                    onClicked: {
                        root.toggleParamRequested(modelData["code"])
                    }

                    contentItem: StyledTextLabel {
                        width: button.width - 24 // 12px padding on each side

                        text: button.text
                        font: ui.theme.bodyFont
                    }
                }
            }
        }
    }
}
