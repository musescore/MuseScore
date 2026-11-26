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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Name")

    Rectangle {
        id: content

        required property MixerChannelItem channelItem
        required property int index

        property bool isRenaming: false

        width: root.channelItemWidth
        height: 22

        function openContextMenu() {
            let items = root.model.channelContextMenuItems(content.index)
            if (items.length === 0) {
                return
            }

            menuLoader.parent = content
            menuLoader.toggleOpened(items)
        }

        function startRenaming() {
            content.isRenaming = true
            renameField.currentText = content.channelItem.title
            renameField.ensureActiveFocus()
            renameField.selectAll()
        }

        function finishRenaming(newName) {
            if (!content.isRenaming) {
                return
            }

            content.isRenaming = false
            root.model.renameVcaGroup(content.index, newName)
        }

        function resolveLabelColor() {
            switch(channelItem.type) {
            case MixerChannelItem.PrimaryInstrument:
            case MixerChannelItem.SecondaryInstrument:
                return ui.theme.accentColor
            case MixerChannelItem.Aux:
                return "#63D47B"
            case MixerChannelItem.Master:
                return "#F87BDC"
            case MixerChannelItem.Vca:
                return "#F5A742"
            }

            return ui.theme.accentColor
        }

        function resolveLabelColorOpacity() {
            if (channelItem.type === MixerChannelItem.SecondaryInstrument) {
                return 0.25
            }

            return 0.5
        }

        readonly property color labelColor: resolveLabelColor()

        color: Utils.colorWithAlpha(labelColor, resolveLabelColorOpacity())
        border.color: labelColor
        border.width: 1

        StyledTextLabel {
            id: textLabel
            anchors.centerIn: parent

            font: ui.theme.bodyBoldFont

            readonly property int margin: -8
            width: margin + parent.width + margin

            visible: !content.isRenaming
            text: content.channelItem.title
        }

        TextInputField {
            id: renameField

            anchors.fill: parent

            visible: content.isRenaming
            textHorizontalAlignment: Qt.AlignHCenter
            textSidePadding: 4
            background.radius: 2

            onTextEditingFinished: function(newTextValue) {
                content.finishRenaming(newTextValue)
            }

            onEscaped: {
                content.isRenaming = false
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            enabled: parent.enabled && !content.isRenaming
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    content.openContextMenu()
                }
            }

            onDoubleClicked: {
                if (content.channelItem.type === MixerChannelItem.Vca) {
                    content.startRenaming()
                }
            }

            onContainsMouseChanged: {
                if (mouseArea.containsMouse && textLabel.truncated) {
                    ui.tooltip.show(mouseArea, content.channelItem.title)
                } else {
                    ui.tooltip.hide(mouseArea)
                }
            }
        }

        StyledMenuLoader {
            id: menuLoader

            // Lets the menu and its submenus extend beyond the mixer panel
            menuAnchorItem: content.Window.window ? content.Window.window.contentItem : null

            onHandleMenuItem: function(itemId) {
                if (itemId === "vca-rename") {
                    content.startRenaming()
                    return
                }

                // Deferred, because the action can remove this delegate
                Qt.callLater(root.model.handleChannelContextMenuItem, content.index, itemId)
            }
        }
    }
}
