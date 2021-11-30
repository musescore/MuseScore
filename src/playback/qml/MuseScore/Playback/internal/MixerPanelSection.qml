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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0
import MuseScore.Playback 1.0

Loader {
    id: root

    property string headerTitle: ""
    property bool headerVisible: true
    property int headerWidth: 98

    property int delegateDefaultWidth: 108

    property real spacingAbove: 4
    property real spacingBelow: 4

    property var model: undefined
    property var rootPanel: undefined

    property int navigationRowStart: 0

    property bool needReadChannelName: false

    default property Component delegateComponent

    signal navigateControlIndexChanged(var index)

    active: visible

    sourceComponent: ListView {
        id: sectionContentList

        width: contentItem.childrenRect.width
        height: contentHeight
        contentHeight: contentItem.childrenRect.height

        interactive: false
        orientation: Qt.Horizontal
        spacing: 0

        model: root.model

        header: Item {
            width: visible ? root.headerWidth + 1 : 0 // +1 for separator
            height: parent.height
            visible: root.headerVisible

            StyledTextLabel {
                anchors.top: parent.top
                anchors.topMargin: root.spacingAbove
                anchors.bottom: parent.bottom
                anchors.bottomMargin: root.spacingBelow
                anchors.right: separator.left
                anchors.rightMargin: 12
                anchors.left: parent.left
                anchors.leftMargin: 12

                horizontalAlignment: Qt.AlignRight
                text: root.headerTitle
            }

            SeparatorLine {
                id: separator
                anchors.right: parent.right
                orientation: Qt.Vertical
            }
        }

        delegate: Row {
            id: delegateRow

            width: root.delegateDefaultWidth + 1 // for separator
            height: root.spacingAbove + delegateLoader.implicitHeight + root.spacingBelow

            topPadding: root.spacingAbove
            bottomPadding: root.spacingBelow
            spacing: 0

            Loader {
                id: delegateLoader

                width: root.delegateDefaultWidth

                property var mixerItem: model.item
                sourceComponent: root.delegateComponent
            }

            SeparatorLine {
                orientation: Qt.Vertical
                height: root.height
            }
        }
    }
}
