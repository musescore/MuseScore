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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Item {
    id: container

    width: 0

    height: 0

    anchors.fill: parent
    property var openedPopup: null
    property bool isPopupOpened: Boolean(openedPopup) && openedPopup.isOpened

    signal opened()
    signal closed()

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: Boolean(loader.item)
                                        ? loader.item.navigationOrderEnd
                                        : navigationOrderStart

    QtObject {
        id: prv

        function componentByType(type) {
            switch (type) {
            case Notation.TYPE_HARP_DIAGRAM: return harpPedalComp
            case Notation.TYPE_CAPO: return capoComp
            }

            return null
        }

        function openPopup(popup) {
            if (Boolean(popup)) {
                openedPopup = popup
                container.opened()
                popup.open()
            }
        }

        function closeOpenedPopup() {
            if (isPopupOpened) {
                openedPopup.close()
                resetOpenedPopup()
            }
        }

        function resetOpenedPopup() {
            container.closed(false)
            openedPopup = null
        }
    }

    function show(elementType, viewPos, size) {
        if (isPopupOpened) {
            prv.closeOpenedPopup()
        }
        opened()
        var popup = loader.createPopup(prv.componentByType(elementType), viewPos, size)
        prv.openPopup(popup)
    }

    function close() {
        closed()
        prv.closeOpenedPopup()
    }

    Loader {
        id: loader

        function createPopup(comp, pos, size) {
            loader.sourceComponent = comp
            loader.item.parent = container
            loader.item.updatePosition(pos, size)

            //! NOTE: All navigation panels in popups must be in the notation view section.
            //        This is necessary so that popups do not activate navigation in the new section,
            //        but at the same time, when clicking on the component (text input), the focus in popup's window should be activated
            loader.item.navigationSection = null

            loader.item.notationViewNavigationSection = container.notationViewNavigationSection
            loader.item.navigationOrderStart = container.navigationOrderStart

            return loader.item
        }
    }

    Component {
        id: harpPedalComp
        HarpPedalPopup {
            onClosed: {
                prv.resetOpenedPopup()
                loader.sourceComponent = null
            }
        }
    }

    Component {
        id: capoComp
        CapoPopup {
            onClosed: {
                prv.resetOpenedPopup()
                loader.sourceComponent = null
            }
        }
    }
}
