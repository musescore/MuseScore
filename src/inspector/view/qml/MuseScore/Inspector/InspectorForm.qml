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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "common"
import "general"
import "notation"
import "text"
import "score"

Rectangle {
    id: root

    property alias model: inspectorRepeater.model

    property NavigationSection navigationSection: null

    color: ui.theme.backgroundPrimaryColor

    function focusFirstItem() {
        var item = inspectorRepeater.itemAt(0)
        if (item) {
            item.navigation.requestActive()
        }
    }

    Flickable {
        id: flickableArea
        anchors.fill: parent

        function ensureContentVisible(invisibleContentHeight) {
            if (flickableArea.contentY + invisibleContentHeight > 0) {
                flickableArea.contentY += invisibleContentHeight
            } else {
                flickableArea.contentY = 0
            }
        }

        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        maximumFlickVelocity: 1000

        contentHeight: contentColumn.childrenRect.height + 2 * contentColumn.anchors.margins

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        ScrollBar.vertical: StyledScrollBar {}

        Column {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 12

            height: childrenRect.height
            spacing: 6

            Repeater {
                id: inspectorRepeater
                model: InspectorListModel {}

                delegate: ExpandableBlank {
                    id: expandableDelegate

                    required property int index
                    required property var inspectorSectionModel // Comes from inspectorListModel

                    NavigationPanel {
                        id: navPanel
                        name: expandableDelegate.title
                        section: root.navigationSection
                        direction: NavigationPanel.Vertical
                        accessible.name: expandableDelegate.title
                        enabled: root.visible
                        order: expandableDelegate.index + 2
                    }

                    navigation.panel: navPanel
                    navigation.row: 0

                    title: inspectorSectionModel.title

                    contentItemComponent: {
                        switch (inspectorSectionModel.sectionType) {
                        case Inspector.SECTION_GENERAL: return generalInspector
                        case Inspector.SECTION_TEXT: return textInspector
                        case Inspector.SECTION_NOTATION:
                            if (inspectorSectionModel.isMultiModel()) {
                                return notationInspectorMultiElements
                            } else {
                                return notationInspectorSingleElement
                            }
                        case Inspector.SECTION_SCORE_DISPLAY: return scoreInspector
                        case Inspector.SECTION_SCORE_APPEARANCE: return scoreAppearanceInspector
                        }
                    }

                    Component {
                        id: generalInspector

                        GeneralInspectorView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                            anchorItem: root

                            onEnsureContentVisibleRequested: {
                                flickableArea.ensureContentVisible(-invisibleContentHeight)
                            }
                        }
                    }

                    Component {
                        id: textInspector

                        TextInspectorView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                            anchorItem: root

                            onEnsureContentVisibleRequested: {
                                flickableArea.ensureContentVisible(-invisibleContentHeight)
                            }
                        }
                    }

                    Component {
                        id: notationInspectorMultiElements

                        NotationMultiElementView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                            anchorItem: root

                            onEnsureContentVisibleRequested: {
                                flickableArea.ensureContentVisible(-invisibleContentHeight)
                            }
                        }
                    }

                    Component {
                        id: notationInspectorSingleElement

                        NotationSingleElementView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                        }
                    }

                    Component {
                        id: scoreInspector

                        ScoreDisplayInspectorView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                        }
                    }

                    Component {
                        id: scoreAppearanceInspector

                        ScoreAppearanceInspectorView {
                            model: expandableDelegate.inspectorSectionModel
                            navigationPanel: navPanel
                            navigationRowStart: expandableDelegate.navigation.row + 1
                        }
                    }
                }
            }
        }
    }
}
