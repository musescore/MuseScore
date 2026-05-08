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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

import "internal"

ColumnLayout {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 1

    property alias contextMenuModel: contextMenuModel
    property Component toolbarComponent: MixerPanelToolbar {
        navigation.section: root.navigationSection
        navigation.order: root.contentNavigationPanelOrderStart
    }

    signal resizeRequested(var newWidth, var newHeight)

    spacing: 0

    function resizePanelToContentHeight() {
        if (contentRow.completed && implicitHeight > 0) {
            root.resizeRequested(width, implicitHeight)
        }
    }

    onImplicitHeightChanged: {
        root.resizePanelToContentHeight()
    }

    QtObject {
        id: prv

        property var currentNavigateControlIndex: undefined
        property bool isPanelActivated: false

        readonly property real headerWidth: 98
        readonly property real channelItemWidth: 108

        function setNavigateControlIndex(index) {
            if (!Boolean(prv.currentNavigateControlIndex) ||
                    index.row !== prv.currentNavigateControlIndex.row ||
                    index.column !== prv.currentNavigateControlIndex.column) {
                prv.isPanelActivated = false
            }

            prv.currentNavigateControlIndex = index
        }
    }

    function scrollToFocusedItem(focusedIndex) {
        let targetScrollPosition = (focusedIndex) * (prv.channelItemWidth + 1) // + 1 for separators
        let maxContentX = flickable.contentWidth - flickable.width

        if (targetScrollPosition + prv.channelItemWidth > flickable.contentX + flickable.width) {
            flickable.contentX = Math.min(targetScrollPosition + prv.channelItemWidth - flickable.width, maxContentX)
        } else if (targetScrollPosition < flickable.contentX) {
            flickable.contentX = Math.max(targetScrollPosition - prv.channelItemWidth, 0)
        }
    }

    MixerPanelModel {
        id: mixerPanelModel

        navigationSection: root.navigationSection
        navigationOrderStart: root.contentNavigationPanelOrderStart + 1 // +1 for toolbar

        onModelReset: {
            Qt.callLater(setupConnections)
        }

        function setupConnections() {
            for (let i = 0; i < mixerPanelModel.rowCount(); i++) {
                let item = mixerPanelModel.get(i)
                item.channelItem.panel.navigationEvent.connect(function(event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        if (Boolean(prv.currentNavigateControlIndex)) {
                            event.setData("controlIndex", [prv.currentNavigateControlIndex.row, prv.currentNavigateControlIndex.column])
                            event.setData("controlOptional", true)
                        }

                        prv.isPanelActivated = true
                        scrollToFocusedItem(i)
                    }
                })
            }
        }
    }

    VideoPanelModel {
        id: videoMixerModel
    }

    Component.onCompleted: {
        videoMixerModel.load()
    }

    MixerPanelContextMenuModel {
        id: contextMenuModel

        Component.onCompleted: {
            contextMenuModel.load()
        }
    }

    StyledFlickable {
        id: flickable

        Layout.fillWidth: true
        Layout.fillHeight: true

        contentWidth: contentRow.width + 1 // for trailing separator
        contentHeight: Math.max(contentRow.height, height)

        implicitHeight: contentRow.height

        interactive: (height < contentHeight || width < contentWidth) && !flickable.resourcePickingActive

        ScrollBar.horizontal: horizontalScrollBar

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }

        property bool completed: false
        property bool resourcePickingActive: soundSection.resourcePickingActive || fxSection.resourcePickingActive

        function positionViewAtEnd() {
            if (!flickable.completed) {
                return
            }

            if (flickable.contentY == flickable.contentHeight) {
                return
            }

            flickable.contentY = flickable.contentHeight - flickable.height
        }

        onContentHeightChanged: {
            flickable.positionViewAtEnd()
        }

        Component.onCompleted: {
            flickable.completed = true
        }

        Row {
            id: separators

            anchors.fill: parent
            anchors.leftMargin: contextMenuModel.labelsSectionVisible ? prv.headerWidth : prv.channelItemWidth

            spacing: prv.channelItemWidth

            Repeater {
                model: contextMenuModel.labelsSectionVisible ? mixerPanelModel.count + (videoMixerModel.hasVideo ? 2 : 1)
                                                             : mixerPanelModel.count + (videoMixerModel.hasVideo ? 1 : 0)

                SeparatorLine { orientation: Qt.Vertical }
            }
        }

        Row {
            id: contentRow

            anchors.bottom: parent.bottom
            width: childrenRect.width
            spacing: 0

            property bool completed: false

            Component.onCompleted: {
                contentRow.completed = true
            }

            Column {
                id: contentColumn

                width: childrenRect.width
                spacing: 0

                MixerSoundSection {
                    id: soundSection

                    visible: contextMenuModel.soundSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth
                    spacingAbove: 8

                    model: mixerPanelModel

                    navigationRowStart: 1
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerFxSection {
                    id: fxSection

                    visible: contextMenuModel.audioFxSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth

                    model: mixerPanelModel

                    navigationRowStart: 100
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerAuxSendsSection {
                    id: auxSendsSection

                    visible: contextMenuModel.auxSendsSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth

                    channelItemWidth: prv.channelItemWidth

                    model: mixerPanelModel

                    navigationRowStart: 200
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerBalanceSection {
                    id: balanceSection

                    visible: contextMenuModel.balanceSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth

                    model: mixerPanelModel

                    navigationRowStart: 300
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerVolumeSection {
                    id: volumeSection

                    visible: contextMenuModel.volumeSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth

                    model: mixerPanelModel

                    navigationRowStart: 400
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerFaderSection {
                    id: faderSection

                    visible: contextMenuModel.faderSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth
                    spacingAbove: -3
                    spacingBelow: -2

                    model: mixerPanelModel

                    navigationRowStart: 500
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerMuteAndSoloSection {
                    id: muteAndSoloSection

                    visible: contextMenuModel.muteAndSoloSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth

                    model: mixerPanelModel

                    navigationRowStart: 600
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }

                MixerTitleSection {
                    id: titleSection

                    visible: contextMenuModel.titleSectionVisible
                    headerVisible: contextMenuModel.labelsSectionVisible
                    headerWidth: prv.headerWidth
                    channelItemWidth: prv.channelItemWidth
                    spacingAbove: 2
                    spacingBelow: 0

                    model: mixerPanelModel

                    navigationRowStart: 700
                    needReadChannelName: prv.isPanelActivated

                    onNavigateControlIndexChanged: function(index) {
                        prv.setNavigateControlIndex(index)
                    }
                }
            }

            Column {
                id: videoMixerChannel

                visible: videoMixerModel.hasVideo
                width: visible ? prv.channelItemWidth : 0
                spacing: 0

                Item {
                    width: parent.width
                    height: soundSection.visible ? soundSection.height : 0
                }

                Item {
                    width: parent.width
                    height: fxSection.visible ? fxSection.height : 0
                }

                Item {
                    width: parent.width
                    height: auxSendsSection.visible ? auxSendsSection.height : 0
                }

                Item {
                    width: parent.width
                    height: balanceSection.visible ? balanceSection.height : 0
                }

                Item {
                    width: parent.width
                    height: volumeSection.visible ? volumeSection.height : 0

                    RowLayout {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        spacing: 4

                        StyledSlider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            stepSize: 1
                            value: videoMixerModel.volumePercent
                            enabled: videoMixerModel.hasVideo && !videoMixerModel.muted
                            navigation.panel: videoNavigationPanel
                            navigation.row: 1
                            navigation.accessible.name: qsTrc("playback", "Video volume") + " " + videoMixerModel.volumePercent + "%"

                            onMoved: {
                                videoMixerModel.volumePercent = value
                            }
                        }

                        StyledTextLabel {
                            Layout.preferredWidth: 32
                            horizontalAlignment: Text.AlignRight
                            text: videoMixerModel.volumePercent + "%"
                            font: ui.theme.captionFont
                        }
                    }
                }

                Item {
                    width: parent.width
                    height: faderSection.visible ? faderSection.height : 0
                }

                Item {
                    width: parent.width
                    height: muteAndSoloSection.visible ? muteAndSoloSection.height : 0

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        FlatToggleButton {
                            height: 20
                            width: 20
                            icon: IconCode.MUTE
                            checked: videoMixerModel.muted
                            enabled: videoMixerModel.hasVideo
                            navigation.panel: videoNavigationPanel
                            navigation.row: 2
                            navigation.accessible.name: qsTrc("playback", "Video mute")

                            onToggled: {
                                videoMixerModel.muted = !checked
                            }
                        }

                        FlatToggleButton {
                            height: 20
                            width: 20
                            icon: IconCode.SOLO
                            checked: videoMixerModel.solo
                            enabled: videoMixerModel.hasVideo
                            navigation.panel: videoNavigationPanel
                            navigation.row: 3
                            navigation.accessible.name: qsTrc("playback", "Video solo")

                            onToggled: {
                                videoMixerModel.solo = !videoMixerModel.solo
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: titleSection.visible ? titleSection.height : 0
                    color: Utils.colorWithAlpha(ui.theme.accentColor, 0.5)
                    border.color: ui.theme.accentColor
                    border.width: 1

                    StyledTextLabel {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        text: qsTrc("playback", "Video")
                        font: ui.theme.bodyBoldFont
                    }
                }

                NavigationPanel {
                    id: videoNavigationPanel
                    name: "VideoMixerPanel"
                    section: root.navigationSection
                    order: root.contentNavigationPanelOrderStart + mixerPanelModel.count + 1
                    direction: NavigationPanel.Horizontal
                }
            }
        }
    }

    Column {
        spacing: 0
        Layout.fillWidth: true
        visible: horizontalScrollBar.isScrollbarNeeded

        SeparatorLine {}

        StyledScrollBar {
            id: horizontalScrollBar
            width: parent.width
            policy: ScrollBar.AlwaysOn
        }
    }

    onHeightChanged: {
        flickable.positionViewAtEnd()
    }
}
