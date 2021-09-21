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

    property string headerTitle: undefined
    property bool headerVisible: true
    property int headerHeight: 22
    property int headerWidth: headerVisible ? 98 : 0
    property int delegateDefaultWidth: 108
    property var model: undefined
    property var rootPanel: undefined

    default property Component delegateComponent

    active: visible

    sourceComponent: ListView {
        id: sectionContentList

        width: contentItem.childrenRect.width
        height: contentHeight
        contentHeight: contentItem.childrenRect.height

        interactive: false
        orientation: Qt.Horizontal
        spacing: 4

        model: root.model

        header: Item {
            height: root.headerHeight
            width: root.headerWidth

            StyledTextLabel {
                anchors {
                    fill: parent
                    rightMargin: 12
                    leftMargin: 12
                }

                horizontalAlignment: Qt.AlignRight
                text: root.headerTitle
                visible: root.headerVisible
            }
        }

        delegate: root.delegateComponent
    }
}
