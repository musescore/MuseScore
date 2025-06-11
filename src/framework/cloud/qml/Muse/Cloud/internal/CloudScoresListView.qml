/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
import MuseScore.Project 1.0
import Muse.Cloud 1.0

ScoresListView {
    id: root

    navigation.name: "OnlineScoresList"
    navigation.accessible.name: qsTrc("project", "Online scores list")

    Component.onCompleted: {
        prv.updateDesiredRowCount()
    }

    Connections {
        target: root.model
        onStateChanged: {
            if (root.model.state === CloudScoresModel.Fine) {
                // After the model has loaded more, check if even more is needed
                prv.updateDesiredRowCount();
            }
        }
    }

    QtObject {
        id: prv

        readonly property int remainingScoresBelowViewport:
            root.view.count - Math.ceil((root.view.contentY + root.view.height) / root.view.rowHeight)

        readonly property bool isSatisfied: remainingScoresBelowViewport >= 20

        onIsSatisfiedChanged: {
            if (!isSatisfied) {
                updateDesiredRowCount();
            }
        }

        property bool updateDesiredRowCountScheduled: false

        function updateDesiredRowCount() {
            if (updateDesiredRowCountScheduled) {
                return
            }

            if (isSatisfied || !root.model.hasMore) {
                return
            }

            updateDesiredRowCountScheduled = true

            Qt.callLater(function() {
                let newDesiredRowCount = root.model.rowCount + (20 - remainingScoresBelowViewport)

                if (root.model.desiredRowCount < newDesiredRowCount) {
                    root.model.desiredRowCount = newDesiredRowCount
                }

                updateDesiredRowCountScheduled = false
            })
        }
    }

    columns: [
        //! Note: sometimes, in the `delegate` item, some properties are used that don't seem to be defined:
        //! `score`, `navigationPanel`, `navigationRow`, `navigationColumnStart`, `listItem`
        //! These properties are provided by the `Loader` inside `columnsRepeater` in `ScoreListItem.qml`.

        ScoresListView.ColumnItem {
            id: visibilityColumn
            header: qsTrc("project/cloud", "Visibility")

            width: function (parentWidth) {
                let parentWidthExclusingSpacing = parentWidth - root.columns.length * root.view.columnSpacing;
                return 0.16 * parentWidthExclusingSpacing
            }

            delegate: Item {
                id: visibilityContainer

                implicitWidth: visibilityRow.implicitWidth
                implicitHeight: visibilityRow.implicitHeight

                visible: !visibilityLabel.isEmpty

                readonly property var iconAndText: {
                    switch (score.cloudVisibility ?? 0) {
                    case CloudVisibility.Private:
                        return { "iconCode": IconCode.LOCK_CLOSED, "text": qsTrc("project/cloud", "Private") }
                    case CloudVisibility.Unlisted:
                        return { "iconCode": IconCode.LOCK_OPEN, "text": qsTrc("project/cloud", "Unlisted") }
                    case CloudVisibility.Public:
                        return { "iconCode": IconCode.GLOBE, "text": qsTrc("project/cloud", "Public") }
                    }
                    return { "iconCode": IconCode.NONE, "text": "" }
                }

                NavigationFocusBorder {
                    navigationCtrl: NavigationControl {
                        name: "VisibilityLabel"
                        panel: navigationPanel
                        row: navigationRow
                        column: navigationColumnStart
                        enabled: visibilityContainer.visible && visibilityContainer.enabled && !visibilityLabel.isEmpty
                        accessible.name: visibilityColumn.header + ": " + visibilityContainer.iconAndText.text
                        accessible.role: MUAccessible.StaticText

                        onActiveChanged: {
                            if (active) {
                                listItem.scrollIntoView()
                            }
                        }
                    }

                    anchors.margins: -radius
                    radius: 2 + border.width
                }

                RowLayout {
                    id: visibilityRow
                    spacing: 8

                    StyledIconLabel {
                        iconCode: visibilityContainer.iconAndText.iconCode
                    }

                    StyledTextLabel {
                        id: visibilityLabel
                        Layout.fillWidth: true

                        text: visibilityContainer.iconAndText.text

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        },

        ScoresListView.ColumnItem {
            id: modifiedColumn

            //: Stands for "Last time that this score was modified".
            //: Used as the header of this column in the scores list.
            header: qsTrc("project", "Modified")

            width: function (parentWidth) {
                let parentWidthExclusingSpacing = parentWidth - root.columns.length * root.view.columnSpacing;
                return 0.16 * parentWidthExclusingSpacing
            }

            delegate: StyledTextLabel {
                id: modifiedLabel
                text: score.timeSinceModified ?? ""

                font.capitalization: Font.AllUppercase
                horizontalAlignment: Text.AlignLeft

                NavigationFocusBorder {
                    navigationCtrl: NavigationControl {
                        name: "ModifiedLabel"
                        panel: navigationPanel
                        row: navigationRow
                        column: navigationColumnStart
                        enabled: modifiedLabel.visible && modifiedLabel.enabled && !modifiedLabel.isEmpty
                        accessible.name: modifiedColumn.header + ": " + modifiedLabel.text
                        accessible.role: MUAccessible.StaticText

                        onActiveChanged: {
                            if (active) {
                                listItem.scrollIntoView()
                            }
                        }
                    }

                    anchors.margins: -radius
                    radius: 2 + border.width
                }
            }
        },

        ScoresListView.ColumnItem {
            id: sizeColumn
            header: qsTrc("global", "Size", "file size")

            width: function (parentWidth) {
                let parentWidthExclusingSpacing = parentWidth - root.columns.length * root.view.columnSpacing;
                return 0.13 * parentWidthExclusingSpacing
            }

            delegate: StyledTextLabel {
                id: sizeLabel
                text: Boolean(score.fileSize) ? score.fileSize : "-"

                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft

                NavigationFocusBorder {
                    navigationCtrl: NavigationControl {
                        name: "SizeLabel"
                        panel: navigationPanel
                        row: navigationRow
                        column: navigationColumnStart
                        enabled: sizeLabel.visible && sizeLabel.enabled && !sizeLabel.isEmpty
                        accessible.name: sizeColumn.header + ": " + (Boolean(score.fileSize) ? score.fileSize : qsTrc("global", "Unknown"))
                        accessible.role: MUAccessible.StaticText

                        onActiveChanged: {
                            if (active) {
                                listItem.scrollIntoView()
                            }
                        }
                    }

                    anchors.margins: -radius
                    radius: 2 + border.width
                }
            }
        },

        ScoresListView.ColumnItem {
            id: viewsColumn

            //: Stands for "The number of times this score was viewed on MuseScore.com".
            //: Used as the header of this column in the scores list.
            header: qsTrc("project", "Views", "number of views")

            width: function (parentWidth) {
                let parentWidthExclusingSpacing = parentWidth - root.columns.length * root.view.columnSpacing;
                return Math.max(0.08 * parentWidthExclusingSpacing, 76)
            }

            delegate: Item {
                id: viewsContainer

                implicitWidth: viewsRow.implicitWidth
                implicitHeight: viewsRow.implicitHeight

                visible: !viewsLabel.isEmpty

                NavigationFocusBorder {
                    navigationCtrl: NavigationControl {
                        name: "ViewsLabel"
                        panel: navigationPanel
                        row: navigationRow
                        column: navigationColumnStart
                        enabled: viewsContainer.visible && viewsContainer.enabled
                        accessible.name: viewsColumn.header + ": " + viewsLabel.text
                        accessible.role: MUAccessible.StaticText

                        onActiveChanged: {
                            if (active) {
                                listItem.scrollIntoView()
                            }
                        }
                    }

                    anchors.margins: -radius
                    radius: 2 + border.width
                }

                RowLayout {
                    id: viewsRow
                    spacing: 8

                    StyledIconLabel {
                        iconCode: IconCode.EYE_OPEN
                    }

                    StyledTextLabel {
                        id: viewsLabel
                        Layout.fillWidth: true

                        text: score.cloudViewCount ?? ""

                        font: ui.theme.largeBodyFont
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }
    ]

    view.footer: root.model.state === CloudScoresModel.Loading
                 ? busyIndicatorComp : null

    Component {
        id: busyIndicatorComp

        Item {
            width: ListView.view ? ListView.view.width : 0
            height: root.view.rowHeight

            StyledBusyIndicator {
                id: indicator

                anchors.centerIn: parent
            }
        }
    }
}

