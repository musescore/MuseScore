/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root
    margins: 0

    contentWidth: content.width
    contentHeight: content.height

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: tieMenuList.navigation.order

    showArrow: false

    onClosed: {
        partialTiePopupModel.onClosed()
    }

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        const opensUp = partialTiePopupModel.tieDirection
        const popupHeight = content.height + root.margins * 2 + root.padding * 2
        root.x = partialTiePopupModel.dialogPosition.x - root.parent.x
        root.y = partialTiePopupModel.dialogPosition.y - root.parent.y - (opensUp ? popupHeight : 0)
        root.setOpensUpward(opensUp)

        tieMenuList.calculateWidth()
    }

    Component.onCompleted: {
        partialTiePopupModel.init()
    }

    Column {
        id: content
        width: tieMenuList.width
        height: tieMenuList.height
        spacing: 0

        PartialTiePopupModel {
            id: partialTiePopupModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }

            onItemsChanged: function() {
                tieMenuList.model = partialTiePopupModel.items
            }
        }

        StyledListView {
            id: tieMenuList

            property int itemHeight: 32

            implicitWidth: contentItem.childrenRect.width
            height: itemHeight * count

            spacing: 0
            arrowControlsAvailable: false

            model: partialTiePopupModel.items

            navigation.section: notationViewNavigationSection
            navigation.order: navigationOrderStart

            visible: true

            function calculateWidth() {
                var result = 0
                for (var item in tieMenuList.contentItem.children) {
                    var row = tieMenuList.contentItem.children[item];
                    if (!(row instanceof ListItemBlank)) {
                        continue
                    }
                    result = Math.max(result, tieMenuList.contentItem.children[item].calculateWidth())
                }

                tieMenuList.width = result
            }

            delegate: PartialTieMenuRowItem {
                implicitHeight: tieMenuList.itemHeight
                hoverHitColor: ui.theme.accentColor
                anchors.left: parent ? parent.left : undefined
                anchors.right: parent ? parent.right : undefined

                navigation.panel: tieMenuList.navigation
                navigation.row: model.index

                onClicked: {
                    partialTiePopupModel.toggleItemChecked(modelData.id)
                }
            }
        }
    }
}
