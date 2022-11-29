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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.CommonScene 1.0
import MuseScore.Shortcuts 1.0

Item {
    id: root

    property var playbackModel: null
    property var navPanel: null
    property bool floating: false

    width: childrenRect.width
    height: 30

    signal shortcutChanged()

    ListView {
        id: buttonsListView

        anchors.left: parent.left

        width: contentWidth
        height: contentHeight

        contentHeight: root.height
        spacing: 4

        model: root.playbackModel

        orientation: Qt.Horizontal
        interactive: false

        delegate: FlatButton {
            id: btn

            width: 30
            height: width

            mouseArea.acceptedButtons: Qt.LeftButton | Qt.RightButton

            property var item: Boolean(model) ? model.itemRole : null

            icon: Boolean(item) ? item.icon : IconCode.NONE

            toolTipTitle: Boolean(item) ? item.title : ""
            toolTipDescription: Boolean(item) ? item.description : ""
            toolTipShortcut: Boolean(item) ? item.shortcuts : ""

            iconFont: ui.theme.toolbarIconsFont

            accentButton: (Boolean(item) && item.checked) || menuLoader.isMenuOpened
            transparent: !accentButton

            navigation.panel: root.navPanel
            navigation.name: toolTipTitle
            navigation.order: model.index

            onClicked: function(mouse) {
                if (menuLoader.isMenuOpened || item.subitems.length) {
                    menuLoader.toggleOpened(item.subitems)
                    return
                }

                if (mouse.button === Qt.RightButton) {
                    contextMenu.show(Qt.point(mouseArea.mouseX, mouseArea.mouseY))
                    return;
                }

                Qt.callLater(root.playbackModel.handleMenuItem, item.id)
            }

            ToolbarShortcutsContextMenu {
                id: contextMenu
                actionCode: btn.item ? btn.item.action : ""

                onItemHandled: {
                    console.log(btn.item.action + " handled!")
                    root.shortcutChanged()
                }
            }

            StyledMenuLoader {
                id: menuLoader

                onHandleMenuItem: function(itemId) {
                    root.playbackModel.handleMenuItem(itemId)
                }
            }
        }
    }

    SeparatorLine {
        id: buttonsSeparator

        anchors.left: buttonsListView.right
        anchors.leftMargin: 6
        anchors.topMargin: 2
        anchors.bottomMargin: 2

        orientation: Qt.Vertical
        visible: !root.floating
    }

    TimeInputField {
        id: timeField

        anchors.left: buttonsSeparator.right
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter

        //! NOTE: explicit width prevents the content from jumping around
        // when a score is being played
        // See: https://github.com/musescore/MuseScore/issues/9633
        width: 58

        maxTime: root.playbackModel.maxPlayTime
        maxMillisecondsNumber: 9
        time: root.playbackModel.playTime

        onTimeEdited: function(newTime) {
            root.playbackModel.playTime = newTime
        }
    }

    MeasureAndBeatFields {
        id: measureAndBeatFields

        anchors.left: timeField.right
        anchors.verticalCenter: parent.verticalCenter

        measureNumber: root.playbackModel.measureNumber
        maxMeasureNumber: root.playbackModel.maxMeasureNumber
        beatNumber: root.playbackModel.beatNumber
        maxBeatNumber: root.playbackModel.maxBeatNumber

        font: timeField.font

        onMeasureNumberEdited: function(newValue) {
            root.playbackModel.measureNumber = newValue
        }

        onBeatNumberEdited: function(newValue) {
            root.playbackModel.beatNumber = newValue
        }
    }

    TempoView {
        id: tempoView

        anchors.left: measureAndBeatFields.right
        anchors.verticalCenter: parent.verticalCenter

        //! NOTE: explicit width prevents the content from jumping around
        // when a score is being played
        // See: https://github.com/musescore/MuseScore/issues/9633
        width: 56

        noteSymbol: root.playbackModel.tempo.noteSymbol
        tempoValue: root.playbackModel.tempo.value

        noteSymbolFont.pixelSize: ui.theme.iconsFont.pixelSize
        tempoValueFont: timeField.font
    }

    SeparatorLine {
        anchors.left: tempoView.right
        anchors.topMargin: 2
        anchors.bottomMargin: 2

        orientation: Qt.Vertical
        visible: !root.floating
    }
}
