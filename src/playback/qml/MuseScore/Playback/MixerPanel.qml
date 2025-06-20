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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0
import MuseScore.Playback 1.0

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
        if (contentColumn.completed && implicitHeight > 0) {
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

        Component.onCompleted: {
            mixerPanelModel.load()
        }

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

        contentWidth: contentColumn.width + 1 // for trailing separator
        contentHeight: Math.max(contentColumn.height, height)

        implicitHeight: contentColumn.height

        interactive: height < contentHeight || width < contentWidth

        ScrollBar.horizontal: horizontalScrollBar

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }

        property bool completed: false

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
                model: contextMenuModel.labelsSectionVisible ? mixerPanelModel.count + 1 : mixerPanelModel.count

                SeparatorLine { orientation: Qt.Vertical }
            }
        }

        Column {
            id: contentColumn

            anchors.bottom: parent.bottom
            width: childrenRect.width
            spacing: 0

            property bool completed: false

            Component.onCompleted: {
                contentColumn.completed = true
            }

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
