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

import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../../../common"

ExpandableBlank {
    id: root

    required property NotePlaybackModel model

    property int navigationRowEnd: contentItem.navigationRowEnd

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        property int navigationRowEnd: playbackDurationOffsetSection.navigationRowEnd

        spacing: 12

        height: implicitHeight
        width: root.width

        Item {
            height: childrenRect.height
            width: root.width

            SpinBoxPropertyView {
                id: velocitySection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigationName: "Velocity"
                navigationPanel: root.navigation.panel
                navigationRowStart: root.navigation.row + 1

                titleText: qsTrc("propertiespanel", "Velocity")
                propertyItem: root.model ? root.model.velocity : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: 0
            }

            SpinBoxPropertyView {
                id: tuningsSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationName: "Tuning"
                navigationPanel: root.navigation.panel
                navigationRowStart: velocitySection.navigationRowEnd + 1

                titleText: qsTrc("propertiespanel", "Tuning (cents)")
                propertyItem: root.model ? root.model.tuning : null
            }
        }

        Item {
            height: childrenRect.height
            width: root.width

            SpinBoxPropertyView {
                id: playbackStartOffsetSection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigationName: "PlaybackStartOffset"
                navigationPanel: root.navigation.panel
                navigationRowStart: tuningsSection.navigationRowEnd + 1

                titleText: qsTrc("propertiespanel", "Start offset")
                propertyItem: root.model ? root.model.playbackStartOffset : null

                step: 1
                decimals: 0
                maxValue: 1920
                minValue: -1920
            }

            SpinBoxPropertyView {
                id: playbackDurationOffsetSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationName: "PlaybackDurationOffset"
                navigationPanel: root.navigation.panel
                navigationRowStart: playbackStartOffsetSection.navigationRowEnd + 1

                titleText: qsTrc("propertiespanel", "Duration offset")
                propertyItem: root.model ? root.model.playbackDurationOffset : null

                step: 1
                decimals: 0
                maxValue: 1920
                minValue: -1920
            }
        }
    }
}
