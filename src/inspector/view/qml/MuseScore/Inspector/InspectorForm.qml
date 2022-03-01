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

import "."

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

    QtObject {
        id: prv

        property var openedPopup: null

        function closePreviousOpenedPopup(newOpenedPopup) {
            if (Boolean(openedPopup) && openedPopup !== newOpenedPopup) {
                openedPopup.close()
            }

            openedPopup = newOpenedPopup
        }
    }

    StyledFlickable {
        id: flickableArea
        anchors.fill: parent

        function ensureContentVisible(invisibleContentHeight) {
            if (flickableArea.contentY + invisibleContentHeight > 0) {
                flickableArea.contentY += invisibleContentHeight
            } else {
                flickableArea.contentY = 0
            }
        }

        flickableDirection: Flickable.VerticalFlick

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
            spacing: 12

            Repeater {
                id: inspectorRepeater

                model: InspectorListModel {
                    id: inspectorListModel
                }

                delegate: Column {
                    width: parent.width

                    spacing: contentColumn.spacing

                    SeparatorLine {
                        anchors.margins: -12

                        visible: model.index !== 0
                    }

                    InspectorSectionDelegate {
                        sectionModel: model.inspectorSectionModel
                        index: model.index
                        anchorItem: root
                        navigationSection: root.navigationSection

                        onReturnToBoundsRequested: {
                            flickableArea.returnToBounds()
                        }

                        onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                            flickableArea.ensureContentVisible(invisibleContentHeight)
                        }

                        onPopupOpened: {
                            prv.closePreviousOpenedPopup(openedPopup)
                        }
                    }
                }
            }
        }
    }
}
