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

    FontMetrics {
        id: bodyMetrics
        font: ui.theme.bodyFont
    }

    FontMetrics {
        id: largeBodyMetrics
        font: ui.theme.largeBodyBoldFont
    }

    //! SMuFL symbols in the music font, sized to the text and centered on its line. The music font's own line
    //! is about four times as tall as its symbols, so it can't simply sit in a line of text.
    component SymbolGlyph: Item {
        id: glyph

        property string text: ""
        property real lineHeight: bodyMetrics.height
        property real basePixelSize: 22 // a staff space of 5.5px
        property real maxInkHeight: lineHeight * 1.3 // tall symbols (e.g. a metronome mark's note) are scaled down

        //! The model names another music font for symbols this one lacks (see addSymbolFallbacks)
        property string fontFamily: ui.theme.musicalFont.family

        readonly property rect baseInk: baseMetrics.tightBoundingRect(text)
        readonly property real inkScale: (baseInk.height > maxInkHeight) ? (maxInkHeight / baseInk.height) : 1

        implicitWidth: Math.max(baseMetrics.advanceWidth(text), baseInk.x + baseInk.width) * inkScale
        implicitHeight: lineHeight

        FontMetrics {
            id: baseMetrics
            font.family: glyph.fontFamily
            font.pixelSize: glyph.basePixelSize
        }

        Text {
            // Puts the middle of the symbol's ink in the middle of the line
            y: (glyph.height - glyph.baseInk.height * glyph.inkScale) / 2 - (baselineOffset + glyph.baseInk.y * glyph.inkScale)

            text: glyph.text
            color: ui.theme.fontPrimaryColor
            font.family: glyph.fontFamily
            font.pixelSize: glyph.basePixelSize * glyph.inkScale
        }
    }

    //! A row's label: { text, isSymbol, emphasis } parts, e.g. "Moderato ", ♩, " = 114". emphasis (text only):
    //! 1 = bold and bright, -1 = dimmed, e.g. the sung syllable of a lyric's word
    component LabelParts: Flow {
        id: flow

        property var parts: []

        Repeater {
            model: flow.parts

            delegate: Item {
                id: part

                required property var modelData

                width: modelData.isSymbol ? symbol.implicitWidth : Math.min(label.implicitWidth, flow.width)
                height: modelData.isSymbol ? symbol.implicitHeight : label.height

                SymbolGlyph {
                    id: symbol
                    visible: part.modelData.isSymbol
                    text: part.modelData.isSymbol ? part.modelData.text : ""
                    fontFamily: part.modelData.fontFamily || ui.theme.musicalFont.family
                }

                StyledTextLabel {
                    id: label
                    visible: !part.modelData.isSymbol
                    width: parent.width
                    readonly property int emphasis: part.modelData.emphasis || 0

                    text: part.modelData.isSymbol ? "" : part.modelData.text
                    font: (emphasis > 0) ? ui.theme.bodyBoldFont : ui.theme.bodyFont
                    opacity: (emphasis > 0) ? 1 : ((emphasis < 0) ? 0.5 : 0.85)
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    //! A listed score item: [prefix] label .... location, with what it does (or more lines of label parts) below. Hover: highlight it, scrolling
    //! to it only if it's out of view. Click: keep it that way. Double-click: select it. itemIndex < 0: plain text.
    component ItemRow: Item {
        id: row

        required property var modelData

        readonly property int itemIndex: modelData.itemIndex
        readonly property bool isInteractive: itemIndex >= 0
        readonly property bool isPinned: Boolean(root.model) && isInteractive && root.model.pinnedIndex === itemIndex

        width: parent ? parent.width : 0
        implicitHeight: rowLayout.implicitHeight + 4

        Rectangle {
            anchors.fill: parent
            radius: 3

            color: row.isPinned ? ui.theme.accentColor : ui.theme.buttonColor

            opacity: {
                if (row.isPinned) {
                    return ui.theme.accentOpacityNormal
                }

                return (row.isInteractive && rowMouseArea.containsMouse) ? ui.theme.buttonOpacityHover : 0
            }
        }

        RowLayout {
            id: rowLayout

            x: 4
            y: 2
            width: parent.width - 8
            spacing: 6

            StyledTextLabel {
                Layout.alignment: Qt.AlignTop
                visible: !isEmpty
                text: row.modelData.prefix
                opacity: 0.6
            }

            Column {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop

                LabelParts {
                    width: parent.width
                    parts: row.modelData.parts
                }

                StyledTextLabel {
                    width: parent.width
                    visible: !isEmpty
                    text: row.modelData.detail
                    opacity: 0.6
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.Wrap
                }

                // e.g. the staccato and tenuto that make up a tenuto-staccato, or "(Strings only)"
                Repeater {
                    model: row.modelData.lines || []

                    delegate: LabelParts {
                        required property var modelData

                        width: parent ? parent.width : 0
                        parts: modelData
                        opacity: 0.7
                    }
                }
            }

            StyledTextLabel {
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                visible: !isEmpty
                text: row.modelData.location
                opacity: 0.6
                horizontalAlignment: Text.AlignRight
            }
        }

        MouseArea {
            id: rowMouseArea

            anchors.fill: parent
            enabled: row.isInteractive
            hoverEnabled: true

            onEntered: root.model.hoverItem(row.itemIndex)
            onExited: root.model.unhoverItem(row.itemIndex)
            onClicked: root.model.clickItem(row.itemIndex)
            onDoubleClicked: root.model.doubleClickItem(row.itemIndex)
        }
    }

    //! The rows that set a value, then (after a line) the markings that adjust it
    component SectionRows: Column {
        id: section

        property var rows: []
        property var markings: []
        property real topGap: 3 // keeps the first row clear of the value above it

        spacing: 2
        topPadding: (rows.length + markings.length) > 0 ? topGap : 0

        Repeater {
            model: section.rows

            delegate: ItemRow {}
        }

        Rectangle {
            visible: section.rows.length > 0 && section.markings.length > 0
            width: parent.width
            height: 1
            color: ui.theme.strokeColor
        }

        Repeater {
            model: section.markings

            delegate: ItemRow {}
        }
    }

    contentItemComponent: Column {
        id: content

        //! One label column for all the sections, so their values line up
        readonly property real labelColumnWidth: Math.max(
            pitchLabelMetrics.advanceWidth,
            tempoLabelMetrics.advanceWidth,
            dynamicLabelMetrics.advanceWidth,
            soundLabelMetrics.advanceWidth,
            otherLabelMetrics.advanceWidth
        )

        spacing: 12
        width: root.width

        TextMetrics {
            id: pitchLabelMetrics
            font: ui.theme.bodyFont
            text: qsTrc("propertiespanel", "Pitch")
        }

        TextMetrics {
            id: tempoLabelMetrics
            font: ui.theme.bodyFont
            text: qsTrc("propertiespanel", "Tempo")
        }

        TextMetrics {
            id: dynamicLabelMetrics
            font: ui.theme.bodyFont
            text: qsTrc("propertiespanel", "Dynamic")
        }

        TextMetrics {
            id: soundLabelMetrics
            font: ui.theme.bodyFont
            text: qsTrc("propertiespanel", "Sound")
        }

        TextMetrics {
            id: otherLabelMetrics
            font: ui.theme.bodyFont
            text: qsTrc("propertiespanel", "Other")
        }

        StyledTextLabel {
            width: parent.width
            visible: root.model ? !root.model.isPlayed : false
            text: qsTrc("propertiespanel", "Not played: this note’s “Play” setting is off")
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }

        // --- Pitch: what playback sounds, and the ottava / capo that move it
        GridLayout {
            width: parent.width
            columns: 2
            columnSpacing: 8
            rowSpacing: 2

            StyledTextLabel {
                Layout.preferredWidth: content.labelColumnWidth
                Layout.alignment: Qt.AlignLeft | Qt.AlignBaseline
                text: pitchLabelMetrics.text
                horizontalAlignment: Text.AlignLeft
            }

            // A button that brings the note back into view, e.g. after a listed item took the view elsewhere
            Item {
                id: pitchButton

                readonly property real padding: 6
                readonly property string tooltipText: qsTrc("propertiespanel", "Show the selected note")

                Layout.fillWidth: true // without a filling item, the grid gives this column's spare width to the labels
                Layout.alignment: Qt.AlignBaseline

                // The text starts at the left, lined up with the values below it; the button's frame and the area that
                // takes clicks reach padding past it on both sides
                implicitWidth: pitchText.implicitWidth
                implicitHeight: pitchText.implicitHeight + 4
                baselineOffset: pitchText.y + pitchText.baselineOffset

                Rectangle {
                    id: pitchFrame

                    x: -pitchButton.padding
                    width: pitchText.implicitWidth + 2 * pitchButton.padding
                    height: parent.height
                    radius: 3
                    color: ui.theme.buttonColor
                    opacity: pitchMouseArea.containsPress ? ui.theme.buttonOpacityHit
                                                          : (pitchMouseArea.containsMouse ? ui.theme.buttonOpacityHover : 0)
                }

                Rectangle {
                    anchors.fill: pitchFrame
                    radius: 3
                    color: "transparent"
                    border.width: 1
                    border.color: ui.theme.strokeColor
                }

                StyledTextLabel {
                    id: pitchText

                    y: 2
                    text: root.model ? root.model.pitch : ""
                    font: ui.theme.largeBodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                MouseArea {
                    id: pitchMouseArea

                    anchors.fill: pitchFrame
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onClicked: root.model.showSelectedNote()

                    onContainsMouseChanged: {
                        if (containsMouse) {
                            ui.tooltip.show(pitchButton, pitchButton.tooltipText)
                        } else {
                            ui.tooltip.hide(pitchButton)
                        }
                    }
                }
            }

            Item { width: 1; height: 1; visible: pitchShiftRows.visible }
            SectionRows {
                id: pitchShiftRows
                Layout.fillWidth: true
                visible: rows.length > 0
                rows: root.model ? root.model.pitchRows : []
            }
        }

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
                        Layout.preferredWidth: content.labelColumnWidth
                        Layout.alignment: Qt.AlignLeft | Qt.AlignBaseline
                        text: tempoLabelMetrics.text
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
                    SectionRows {
                        Layout.fillWidth: true
                        rows: passColumn.modelData.tempoRows
                        markings: root.model ? root.model.tempoMarkings : []
                    }

                    // --- Dynamic
                    StyledTextLabel {
                        Layout.topMargin: 6
                        Layout.preferredWidth: content.labelColumnWidth
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: dynamicLabelMetrics.text
                        horizontalAlignment: Text.AlignLeft
                    }

                    RowLayout {
                        Layout.topMargin: 6
                        Layout.fillWidth: true
                        spacing: 6

                        StyledTextLabel {
                            visible: passColumn.modelData.dynamicApprox
                            text: "≈"
                            font: ui.theme.largeBodyBoldFont
                        }

                        SymbolGlyph {
                            visible: text !== ""
                            text: passColumn.modelData.dynamicSymbol
                            lineHeight: largeBodyMetrics.height
                            basePixelSize: 28
                            maxInkHeight: lineHeight * 1.5
                        }

                        StyledTextLabel {
                            visible: !isEmpty
                            text: passColumn.modelData.dynamicText
                            font.family: ui.theme.largeBodyBoldFont.family
                            font.pixelSize: ui.theme.largeBodyBoldFont.pixelSize
                            font.bold: true
                            font.italic: true
                        }

                        Rectangle {
                            id: muteBadge

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

                        Item { Layout.fillWidth: true }

                        StyledTextLabel {
                            Layout.rightMargin: 4 // lines up with the rows' locations
                            text: passColumn.modelData.dynamicPercent
                            opacity: 0.75
                        }
                    }

                    Item { width: 1; height: 1 }
                    SectionRows {
                        Layout.fillWidth: true
                        rows: passColumn.modelData.dynamicRows
                        markings: root.model ? root.model.dynamicMarkings : []
                    }
                }
            }
        }

        // --- Sound: what chooses the sound (technique text, Muse Sounds flags, the sung syllable)
        GridLayout {
            width: parent.width
            columns: 2
            columnSpacing: 8
            visible: root.model ? root.model.soundRows.length > 0 : false

            StyledTextLabel {
                Layout.preferredWidth: content.labelColumnWidth
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.topMargin: 2 // level with the first row's text
                text: soundLabelMetrics.text
                horizontalAlignment: Text.AlignLeft
            }

            SectionRows {
                Layout.fillWidth: true
                topGap: 0 // nothing above it
                rows: root.model ? root.model.soundRows : []
            }
        }

        // --- Other: everything else that shapes how the note plays
        GridLayout {
            width: parent.width
            columns: 2
            columnSpacing: 8
            visible: root.model ? root.model.otherMarkings.length > 0 : false

            StyledTextLabel {
                Layout.preferredWidth: content.labelColumnWidth
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.topMargin: 2 // level with the first row's text
                text: otherLabelMetrics.text
                horizontalAlignment: Text.AlignLeft
            }

            SectionRows {
                Layout.fillWidth: true
                topGap: 0 // nothing above it
                rows: root.model ? root.model.otherMarkings : []
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
    }
}

