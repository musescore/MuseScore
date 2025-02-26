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
import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui

import MuseScore.CommonScene
import MuseScore.Playback

Item {
    id: root

    property PlaybackToolBarModel playbackModel: null

    property NavigationPanel navPanel: null
    readonly property int navigationOrderEnd: tempoLoader.navigationOrderEnd

    property bool floating: false

    // Not `+ endSeparator.width`: this way, the separator itself is outside the view,
    // which means that it will be exactly at the position of the KDDockWidgets separator
    // between this toolbar and the undo/redo toolbar.
    width: endSeparator.visible ? endSeparator.x
                                : tempoLoader.x + tempoLoader.width
    height: 30

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

        readonly property int navigationOrderEnd: count

        delegate: FlatButton {
            id: btn

            width: 30
            height: width

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
            accessible.name: (item.checkable ? (item.checked ? item.title + "  " + qsTrc("global", "On") :
                                                               item.title + "  " + qsTrc("global", "Off")) : item.title)

            onClicked: {
                if (menuLoader.isMenuOpened || item.subitems.length) {
                    menuLoader.toggleOpened(item.subitems)
                    return
                }

                Qt.callLater(root.playbackModel.handleMenuItem, item.id)
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
        anchors.verticalCenterOffset: 1 // for nicer visual alignment

        //! NOTE: explicit width prevents the content from jumping around
        // when a score is being played
        // See: https://github.com/musescore/MuseScore/issues/9633
        width: 64

        maxTime: root.playbackModel.maxPlayTime
        maxMillisecondsNumber: 9
        time: root.playbackModel.playTime

        navigationPanel: root.navPanel
        navigationOrderStart: buttonsListView.navigationOrderEnd + 1

        onTimeEdited: function(newTime) {
            root.playbackModel.playTime = newTime
        }
    }

    MeasureAndBeatFields {
        id: measureAndBeatFields

        anchors.left: timeField.right
        anchors.leftMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 1 // for nicer visual alignment

        measureNumber: root.playbackModel.measureNumber
        maxMeasureNumber: root.playbackModel.maxMeasureNumber
        beatNumber: root.playbackModel.beatNumber
        maxBeatNumber: root.playbackModel.maxBeatNumber

        font: timeField.font

        navigationPanel: root.navPanel
        navigationOrderStart: timeField.navigationOrderEnd + 1

        onMeasureNumberEdited: function(newValue) {
            root.playbackModel.measureNumber = newValue
        }

        onBeatNumberEdited: function(newValue) {
            root.playbackModel.beatNumber = newValue
        }
    }

    Loader {
        id: tempoLoader

        anchors.left: measureAndBeatFields.right
        anchors.leftMargin: 6

        readonly property int navigationOrderEnd: item?.navigation?.order ?? measureAndBeatFields.navigationOrderEnd

        sourceComponent: root.floating ? tempoViewComponent : tempoButtonComponent

        Component {
            id: tempoViewComponent

            Item {
                // 2 * ui.theme.defaultButtonSize ≈ "60 but scaled to body text size"
                // See https://github.com/musescore/MuseScore/pull/25621#issuecomment-2564382857
                implicitWidth: 2 * ui.theme.defaultButtonSize
                implicitHeight: ui.theme.defaultButtonSize

                TempoView {
                    id: tempoView
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    noteSymbol: root.playbackModel.tempo.noteSymbol
                    tempoValue: root.playbackModel.tempo.value

                    noteSymbolFont.pixelSize: ui.theme.iconsFont.pixelSize
                    tempoValueFont: timeField.font
                }
            }
        }

        Component {
            id: tempoButtonComponent

            FlatButton {
                // 2 * ui.theme.defaultButtonSize ≈ "60 but scaled to body text size"
                // See https://github.com/musescore/MuseScore/pull/25621#issuecomment-2564382857
                implicitWidth: 2 * ui.theme.defaultButtonSize
                implicitHeight: ui.theme.defaultButtonSize

                accentButton: playbackSpeedPopup.isOpened
                transparent: !accentButton

                toolTipTitle: qsTrc("playback", "Speed")

                navigation.panel: root.navPanel
                navigation.order: measureAndBeatFields.navigationOrderEnd + 1

                contentItem: TempoView {
                    anchors.centerIn: parent

                    noteSymbol: root.playbackModel.tempo.noteSymbol
                    tempoValue: root.playbackModel.tempo.value

                    noteSymbolFont.pixelSize: ui.theme.iconsFont.pixelSize
                    tempoValueFont: timeField.font
                }

                onClicked: {
                    playbackSpeedPopup.toggleOpened()
                }

                PlaybackSpeedPopup {
                    id: playbackSpeedPopup

                    playbackModel: root.playbackModel
                }
            }
        }
    }

    SeparatorLine {
        id: endSeparator
        anchors.left: tempoLoader.right
        anchors.leftMargin: 6
        anchors.topMargin: 2
        anchors.bottomMargin: 2

        orientation: Qt.Vertical
        visible: !root.floating
    }
}
