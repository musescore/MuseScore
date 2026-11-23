/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

ExpandableBlank {
    id: root

    required property NotePlaybackInfoModel model

    property int navigationRowEnd: navigation.row

    enabled: model ? !model.isEmpty : false
    title: model ? model.title : ""
    isExpanded: true

    width: parent.width

    contentItemComponent: Column {
        id: content

        spacing: 12
        width: root.width

        Repeater {
            model: root.model ? root.model.passes : []

            delegate: Column {
                id: passColumn

                required property var modelData

                width: content.width
                spacing: 6

                StyledTextLabel {
                    visible: !isEmpty
                    text: passColumn.modelData.label
                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 8
                    rowSpacing: 2

                    // --- Tempo
                    StyledTextLabel {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignBaseline
                        text: qsTrc("propertiespanel", "Tempo")
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledTextLabel {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignBaseline
                        text: passColumn.modelData.tempo
                        font: ui.theme.largeBodyBoldFont
                        horizontalAlignment: Text.AlignLeft
                    }

                    Item { width: 1; height: 1; visible: tempoDetail.visible }
                    StyledTextLabel {
                        id: tempoDetail
                        Layout.fillWidth: true
                        visible: !isEmpty
                        text: passColumn.modelData.tempoDetail
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.Wrap
                    }

                    Item { width: 1; height: 1 }
                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: passColumn.modelData.tempoSource
                        opacity: 0.75
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.Wrap
                    }

                    // --- Dynamic
                    StyledTextLabel {
                        Layout.topMargin: 6
                        Layout.alignment: Qt.AlignLeft | Qt.AlignBaseline
                        text: qsTrc("propertiespanel", "Dynamic")
                        horizontalAlignment: Text.AlignLeft
                    }

                    Row {
                        Layout.topMargin: 6
                        Layout.fillWidth: true
                        spacing: 8

                        StyledTextLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            text: passColumn.modelData.dynamic
                            font.family: ui.theme.largeBodyBoldFont.family
                            font.pixelSize: ui.theme.largeBodyBoldFont.pixelSize
                            font.bold: true
                            font.italic: true
                        }

                        Rectangle {
                            id: muteBadge

                            anchors.verticalCenter: parent.verticalCenter
                            visible: root.model ? root.model.isMuted : false
                            width: 18
                            height: 18
                            radius: 3
                            color: ui.theme.accentColor

                            StyledTextLabel {
                                anchors.centerIn: parent
                                text: "M"
                                font: ui.theme.bodyBoldFont
                                color: "white"
                            }
                        }

                        StyledTextLabel {
                            anchors.verticalCenter: parent.verticalCenter
                            text: passColumn.modelData.dynamicDetail
                            opacity: 0.75
                        }
                    }

                    Item { width: 1; height: 1 }
                    StyledTextLabel {
                        Layout.fillWidth: true
                        text: passColumn.modelData.dynamicSource
                        opacity: 0.75
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.Wrap
                    }
                }
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: root.model ? root.model.isMuted : false
            text: root.model ? "M — " + root.model.muteReason : ""
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }

        StyledTextLabel {
            width: parent.width
            visible: root.model ? root.model.velocityOverride > 0 : false
            text: root.model ? qsTrc("propertiespanel", "Velocity override: %1 (set below)").arg(root.model.velocityOverride) : ""
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }

        StyledTextLabel {
            width: parent.width
            visible: root.model ? root.model.isTiedContinuation : false
            text: qsTrc("propertiespanel", "Tied from an earlier note: the dynamic is the one where the tie starts.")
            opacity: 0.75
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }
    }
}

