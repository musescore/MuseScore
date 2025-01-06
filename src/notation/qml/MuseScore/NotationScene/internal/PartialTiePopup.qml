/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

    property alias notationViewNavigationSection: partialTieNavPanel.section
    property alias navigationOrderStart: partialTieNavPanel.order
    readonly property alias navigationOrderEnd: partialTieNavPanel.order

    property QtObject model: partialTiePopupModel

    showArrow: false

    onOpened: {
        tieMenuLoader.show(Qt.point(0, 0))
    }

    onClosed: {
        tieMenuLoader.close()
        partialTiePopupModel.onClosed()
    }

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        const opensUp = partialTiePopupModel.tieDirection
        const popupHeight = tieMenuLoader.item.height + root.margins * 2 + root.padding * 2
        root.x = partialTiePopupModel.dialogPosition.x - root.parent.x
        root.y = partialTiePopupModel.dialogPosition.y - root.parent.y - (opensUp ? popupHeight : 0)
        root.setOpensUpward(opensUp)
    }

    contentWidth: tieMenuLoader.width
    contentHeight: tieMenuLoader.childrenRect.height

    ContextMenuLoader {
        id: tieMenuLoader
        closeMenuOnSelection: false
        focusOnOpened: false
        opensUpward: root.opensUpward

        items: partialTiePopupModel.items

        onHandleMenuItem: function(itemId) {
            partialTiePopupModel.toggleItemChecked(itemId)
        }

        PartialTiePopupModel {
            id: partialTiePopupModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }

            onItemsChanged: function() {
                tieMenuLoader.show(Qt.point(0, 0))
            }
        }

        Component.onCompleted: {
            partialTiePopupModel.init()
        }

        NavigationPanel {
            id: partialTieNavPanel
            name: "PartialTieMenu"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("notation", "Partial tie menu items")

            onSectionChanged: function() {
                tieMenuLoader.notationViewNavigationSection = section
            }

            onOrderChanged: function() {
                tieMenuLoader.navigationOrderStart = order
            }
        }
    }
}
