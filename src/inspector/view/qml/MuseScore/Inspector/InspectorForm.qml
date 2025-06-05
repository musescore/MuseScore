/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "."

Rectangle {
    id: root

    property alias model: sectionList.model
    property alias notationView: popupController.notationView

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    color: ui.theme.backgroundPrimaryColor

    onVisibleChanged: {
        inspectorListModel.setInspectorVisible(root.visible)
    }

    function focusFirstItem() {
        var item = sectionList.itemAtIndex(0)
        if (item) {
            item.navigation.requestActive()
        }
    }

    QtObject {
        id: prv

        function closePreviousOpenedPopup(newOpenedPopup, visualControl) {
            if (Boolean(popupController.popup) && popupController.popup !== newOpenedPopup) {
                popupController.popup.close()
            }

            popupController.visualControl = visualControl
            popupController.popup = newOpenedPopup
        }
    }

    InspectorPopupController {
        id: popupController
    }

    StyledListView {
        id: sectionList
        anchors.fill: parent

        topMargin: 12
        bottomMargin: 12

        spacing: 12

        cacheBuffer: Math.max(0, contentHeight)

        function ensureContentVisible(invisibleContentHeight) {
            if (sectionList.contentY + invisibleContentHeight > 0) {
                sectionList.contentY += invisibleContentHeight
            } else {
                sectionList.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        model: InspectorListModel {
            id: inspectorListModel
        }

        onContentHeightChanged: {
            returnToBounds()
        }

        delegate: Column {
            width: ListView.view.width
            spacing: sectionList.spacing

            property var navigationPanel: _item.navigationPanel

            SeparatorLine {
                visible: model.index !== 0
            }

            InspectorSectionDelegate {
                id: _item

                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 12

                sectionModel: model.inspectorSectionModel
                anchorItem: root
                navigationPanel.section: root.navigationSection
                navigationPanel.order: root.navigationOrderStart + model.index

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    sectionList.ensureContentVisible(invisibleContentHeight)
                }

                onPopupOpened: function(openedPopup, visualControl) {
                    prv.closePreviousOpenedPopup(openedPopup, visualControl)
                }
            }
        }
    }
}
