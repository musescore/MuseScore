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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0
import MuseScore.Playback 1.0

Loader {
    id: root

    property string headerTitle: ""
    property bool headerVisible: true
    property int headerWidth: 98
    property int headerHeight: implicitHeight - spacingAbove - spacingBelow

    property int channelItemWidth: 108

    property real spacingAbove: 4
    property real spacingBelow: 4

    property var model: undefined

    property int navigationRowStart: 0

    property bool needReadChannelName: false

    default property Component delegateComponent

    signal navigateControlIndexChanged(var index)

    active: visible

    sourceComponent: Row {
        width: implicitWidth
        height: root.spacingAbove + sectionContentList.contentHeight + root.spacingBelow
        spacing: 1 // for separator (will be rendered in MixerPanel.qml)

        StyledTextLabel {
            visible: root.headerVisible

            anchors.top: parent.top
            anchors.topMargin: root.spacingAbove

            width: root.headerWidth
            height: root.headerHeight

            leftPadding: 12
            rightPadding: 12

            horizontalAlignment: Qt.AlignRight
            text: root.headerTitle
        }

        ListView {
            id: sectionContentList

            anchors.top: parent.top
            anchors.topMargin: root.spacingAbove
            width: contentItem.childrenRect.width
            height: Math.max(1, contentHeight) // HACK: if the height is 0, the listview won't create any delegates
            contentHeight: contentItem.childrenRect.height

            interactive: false
            orientation: Qt.Horizontal
            spacing: 1 // for separators (will be rendered in MixerPanel.qml)

            model: root.model
            delegate: delegateComponent
        }
    }
}
