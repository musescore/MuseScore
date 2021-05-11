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
import QtQml.Models 2.3

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
            item.navigation.ensureActive()
        }
    }

    NavigationPanel {
        id: navPanel
        name: "Inspector"
        section: root.navigationSection
        direction: NavigationPanel.Vertical
        order: 2
    }

    InspectorListModel {
        id: inspectorListModel

        onModelChanged: {
            if (navPanel.active) {
                root.focusFirstItem()
            }
        }
    }

    Flickable {
        id: flickableArea

        anchors.fill: parent
        anchors.margins: 12

        function updateContentHeight() {
            var resultContentHeight = 0

            for (var i = 0; i < inspectorRepeater.count; ++i) {
                resultContentHeight += inspectorRepeater.itemAt(i).contentHeight
            }

            flickableArea.contentHeight = resultContentHeight
        }

        function ensureContentVisible(delegateY, delegateContentHeight) {

            var contentBottomY = delegateY + delegateContentHeight

            if (contentBottomY > flickableArea.height) {
                flickableArea.contentY = contentBottomY - flickableArea.height
            } else {
                flickableArea.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 6

            Repeater {
                id: inspectorRepeater

                model: inspectorListModel

                delegate: ExpandableBlank {
                    id: expandableDelegate

                    property var contentHeight: implicitHeight

                    navigation.panel: navPanel
                    navigation.row: (model.index + 1) * 1000 // make unique

                    function viewBySectionType() {
                        flickableArea.contentY = 0

                        switch (inspectorData.sectionType) {
                        case Inspector.SECTION_GENERAL: return generalInspector
                        case Inspector.SECTION_TEXT: return textInspector
                        case Inspector.SECTION_NOTATION: return notationInspector
                        case Inspector.SECTION_SCORE_DISPLAY: return scoreInspector
                        case Inspector.SECTION_SCORE_APPEARANCE: return scoreAppearanceInspector
                        }
                    }

                    contentItemComponent: viewBySectionType()

                    Component.onCompleted: {
                        title = inspectorData.title
                    }

                    function updateContentHeight(newContentHeight) {
                        expandableDelegate.contentHeight = newContentHeight
                        flickableArea.updateContentHeight()
                        flickableArea.ensureContentVisible(y, newContentHeight)
                    }

                    Component {
                        id: generalInspector
                        GeneralInspectorView {
                            model: inspectorData
                            navigationPanel: navPanel
                            navigationRowOffset: expandableDelegate.navigation.row + 1
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: textInspector
                        TextInspectorView {
                            model: inspectorData
                            navigationPanel: navPanel
                            navigationRowOffset: expandableDelegate.navigation.row + 1
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: notationInspector
                        NotationInspectorView {
                            model: inspectorData
                            navigationPanel: navPanel
                            navigationRowOffset: expandableDelegate.navigation.row + 1
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreInspector

                        ScoreDisplayInspectorView {
                            model: inspectorData
                            navigationPanel: navPanel
                            navigationRowOffset: expandableDelegate.navigation.row + 1
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                    Component {
                        id: scoreAppearanceInspector

                        ScoreAppearanceInspectorView {
                            model: inspectorData
                            navigationPanel: navPanel
                            navigationRowOffset: expandableDelegate.navigation.row + 1
                            onContentExtended: expandableDelegate.updateContentHeight(contentHeight)
                        }
                    }
                }
            }
        }
    }
}
