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
        if (contentColumn.completed && implicitHeight > 0) {
            root.resizeRequested(width, implicitHeight)
        }
    }

    // buffer channel item; keep in sync with the model state
    property var activeChannelItemObj: null

    // avoid first-frame races
    property bool highlightOn: Boolean(contextMenuModel.highlightSelection) || Boolean(mixerPanelModel.highlightEnabled)

    function recomputeActiveChannelItem() {
        try {
            if (prv.activeChannelIndex >= 0
                    && typeof mixerPanelModel.rowCount === "function"
                    && mixerPanelModel.rowCount() > prv.activeChannelIndex) {
                activeChannelItemObj = mixerPanelModel.get(prv.activeChannelIndex).channelItem
            } else {
                activeChannelItemObj = null
            }
        } catch (e) {
            activeChannelItemObj = null
        }
    }

    onImplicitHeightChanged: {
        root.resizePanelToContentHeight()
    }

    QtObject {
        id: prv

        property var currentNavigateControlIndex: undefined
        property bool isPanelActivated: false
        property int activeChannelIndex: -1

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
        if (typeof focusedIndex !== "number" || focusedIndex < 0) {
            return
        }

        const step = prv.channelItemWidth + 1 // avoid drift on subsequent channels

        const header = contextMenuModel.labelsSectionVisible ? prv.headerWidth : 0
        const x = Math.round(header + focusedIndex * step)

        flickable.scrollToXAnimated(x)
    }

    Connections {
        target: prv
        function onActiveChannelIndexChanged() { Qt.callLater(recomputeActiveChannelItem) }
    }

    Connections {
        target: mixerPanelModel

        // recompute when the model finishes (re)building
        function onRowCountChanged() { Qt.callLater(recomputeActiveChannelItem) }
        function onModelReset()      { Qt.callLater(recomputeActiveChannelItem) }

        function onScrollToIndexRequested(index) {
            Qt.callLater(function() {
                prv.activeChannelIndex = index

                if (mixerPanelModel.autoScrollEnabled) {
                    scrollToFocusedItem(index)
                }
            })
        }

        function onHighlightIndexRequested(index) {
            if (!mixerPanelModel.highlightEnabled) return
            Qt.callLater(function() { prv.activeChannelIndex = index })
        }
    }

    Connections {
        target: contextMenuModel

        function onAutoScrollToSelectionChanged() {
            mixerPanelModel.autoScrollEnabled = contextMenuModel.autoScrollToSelection

            if (contextMenuModel.autoScrollToSelection) {
                if (prv.activeChannelIndex >= 0) {
                    Qt.callLater(function () { scrollToFocusedItem(prv.activeChannelIndex) })
                } else {
                    Qt.callLater(function () { mixerPanelModel.resyncToCurrentSelection() })
                }
            }
        }

        function onHighlightSelectionChanged() {
            mixerPanelModel.highlightEnabled = contextMenuModel.highlightSelection
            if (!contextMenuModel.highlightSelection) {
                Qt.callLater(function () { prv.activeChannelIndex = -1 })
            } else {
                Qt.callLater(function () { mixerPanelModel.resyncToCurrentSelection() })
            }
        }
    }

    MixerPanelModel {
        id: mixerPanelModel

        navigationSection: root.navigationSection
        navigationOrderStart: root.contentNavigationPanelOrderStart + 1 // +1 for toolbar

        Component.onCompleted: {
            mixerPanelModel.highlightEnabled = contextMenuModel.highlightSelection
            mixerPanelModel.autoScrollEnabled = contextMenuModel.autoScrollToSelection

            if (!contextMenuModel.highlightSelection) {
                Qt.callLater(function() { prv.activeChannelIndex = -1 })
            }
        }

        onModelReset: {
            Qt.callLater(setupConnections)
            Qt.callLater(function() { mixerPanelModel.resyncToCurrentSelection() })
        }

        function setupConnections() {
            for (let i = 0; i < mixerPanelModel.rowCount(); i++) {
                let item = mixerPanelModel.get(i)
                item.channelItem.panel.navigationEvent.connect(function (event) {
                    if (event.type === NavigationEvent.AboutActive) {
                        if (Boolean(prv.currentNavigateControlIndex)) {
                            event.setData("controlIndex", [prv.currentNavigateControlIndex.row, prv.currentNavigateControlIndex.column])
                            event.setData("controlOptional", true)
                        }

                        prv.isPanelActivated = true
                        prv.activeChannelIndex = i

                        if (mixerPanelModel.autoScrollEnabled) {
                            scrollToFocusedItem(i)
                        }
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

        interactive: (height < contentHeight || width < contentWidth) && !flickable.resourcePickingActive

        ScrollBar.horizontal: horizontalScrollBar

        ScrollBar.vertical: StyledScrollBar { policy: ScrollBar.AlwaysOn }

        property bool completed: false
        property bool resourcePickingActive: soundSection.resourcePickingActive || fxSection.resourcePickingActive

        PropertyAnimation {
            id: scrollXAnim
            target: flickable
            property: "contentX"
            duration: 500
            easing.type: Easing.InOutCubic
            running: false
        }

        function scrollToXAnimated(targetX, dur) {
            const maxX = Math.max(0, flickable.contentWidth - flickable.width)
            const clamped = Math.max(0, Math.min(targetX, maxX))

            if (flickable.dragging || flickable.flicking) {
                return
            }
            scrollXAnim.stop()
            scrollXAnim.from = flickable.contentX
            scrollXAnim.to = clamped
            if (typeof dur === "number" && dur > 0) {
                scrollXAnim.duration = dur
            }
            scrollXAnim.running = true
        }

        onMovementStarted: scrollXAnim.stop() // stop animation if user moves

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

        Item {
            id: activeOverlays

            parent: flickable.contentItem
            anchors.fill: parent

            readonly property real leftOffset:
                contextMenuModel.labelsSectionVisible ? prv.headerWidth : 0

            // compute x for current active strip
            readonly property real activeX:
                prv.activeChannelIndex < 0 ? -10000
                : leftOffset + (prv.activeChannelIndex * (prv.channelItemWidth + 1)) // +1 for separators

            Rectangle {
                id: activeTopStrip
                visible: highlightOn && prv.activeChannelIndex >= 0
                x: activeOverlays.activeX
                width: prv.channelItemWidth + 1
                height: 2
                color: ui.theme.accentColor
                radius: 0
                z: 1000 // lift above content
            }

            Rectangle {
                id: activeHighlight
                visible: highlightOn && prv.activeChannelIndex >= 0
                x: activeOverlays.activeX
                width: prv.channelItemWidth + 1
                height: parent.height
                color: ui.theme.fontPrimaryColor
                opacity: 0.08
                z: 1000
            }
        }

        Row {
            id: separators

            anchors.fill: parent
            // when labels hidden, grid starts at x=0, not one channel
            anchors.leftMargin: contextMenuModel.labelsSectionVisible ? prv.headerWidth : 0

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

                activeChannelIndex: prv.activeChannelIndex
                activeChannelItem: activeChannelItemObj
                highlightSelection: highlightOn

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
