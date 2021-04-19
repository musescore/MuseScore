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
import QtQuick 2.12
import QtQuick.Controls 2.12

import MuseScore.UiComponents 1.0

Item {
    id: root

    property var model

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 36

        spacing: 16

        Row {
            width: parent.width
            height: childrenRect.height

            StyledTextLabel {
                width: parent.width * 2/3

                text: qsTrc("notation", "NAME")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
            }

            StyledTextLabel {
                id: voicesVisibilityHeader

                width: parent.width * 1/3

                text: qsTrc("notation", "VISIBLE VOICES")

                horizontalAlignment: Qt.AlignLeft
                font.capitalization: Font.AllUppercase
            }
        }

        SeparatorLine { anchors.margins: -36 }
    }

    ListView {
        id: view

        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        spacing: 0

        model: root.model

        boundsBehavior: Flickable.StopAtBounds
        interactive: height < contentHeight
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 8
        }

        Connections {
            target: root.model

            function onPartAdded(index) {
                view.positionViewAtIndex(index, ListView.Contain)
                view.currentIndex = index
                view.currentItem.startEditTitle()
            }
        }

        delegate: PartDelegate {
            readonly property int sideMargin: 24

            title: model.title
            maxTitleWidth: voicesVisibilityHeader.x
            currentPartIndex: view.currentIndex
            isSelected: model.isSelected
            isMain: model.isMain
            voicesVisibility: model.voicesVisibility
            voicesTitle: model.voicesTitle
            sidePadding: 24

            onPartClicked: {
                root.model.selectPart(model.index)
                view.currentIndex = model.index
            }

            onTitleChanged: {
                root.model.setPartTitle(model.index, title)
            }

            onVoicesVisibilityChangeRequested: {
                root.model.setVoiceVisible(model.index, voiceIndex, voiceVisible)
            }

            onRemovePartRequested: {
                root.model.removePart(model.index)
            }

            onCopyPartRequested: {
                root.model.copyPart(model.index)
            }
        }
    }
}
