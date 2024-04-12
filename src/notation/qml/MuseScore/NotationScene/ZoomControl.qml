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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

RowLayout {
    id: root

    property alias currentZoomPercentage: zoomInputField.value
    property alias minZoomPercentage: zoomInputField.minValue
    property alias maxZoomPercentage: zoomInputField.maxValue
    property var availableZoomList: []

    property NavigationPanel navigationPanel: null
    property int navigationOrderMin: 0
    readonly property int navigationOrderMax: menuButton.navigation.order

    signal changeZoomPercentageRequested(var newZoomPercentage)
    signal changeZoomRequested(var zoomId)
    signal zoomInRequested()
    signal zoomOutRequested()

    spacing: 0

    FlatButton {
        id: zoomOutButton
        icon: IconCode.ZOOM_OUT
        iconFont: ui.theme.toolbarIconsFont

        width: height
        height: 28
        transparent: true

        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrderMin
        accessible.name: qsTrc("notation", "Zoom out")

        onClicked: {
            root.zoomOutRequested()
        }
    }

    FlatButton {
        id: zoomInButton
        Layout.leftMargin: 4

        icon: IconCode.ZOOM_IN
        iconFont: ui.theme.toolbarIconsFont

        width: height
        height: 28
        transparent: true

        navigation.panel: root.navigationPanel
        navigation.order: zoomOutButton.navigation.order + 1
        accessible.name: qsTrc("notation", "Zoom in")

        onClicked: {
            root.zoomInRequested()
        }
    }

    Row {
        Layout.leftMargin: 12
        Layout.fillHeight: true

        spacing: 1

        NumberInputField {
            id: zoomInputField

            anchors.verticalCenter: parent.verticalCenter

            live: false

            addLeadingZeros: false
            font: ui.theme.bodyFont

            navigation.panel: root.navigationPanel
            navigation.order: zoomOutButton.navigation.order + 1

            onValueEdited: function(newValue) {
                root.changeZoomPercentageRequested(newValue)
            }
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: "%"
        }
    }

    MenuButton {
        id: menuButton
        Layout.leftMargin: 4
        Layout.preferredWidth: 20
        height: 28

        icon: IconCode.SMALL_ARROW_DOWN

        navigation.panel: root.navigationPanel
        navigation.order: zoomInputField.navigation.order + 1
        accessible.name: qsTrc("notation", "Zoom menu")

        menuModel: root.availableZoomList
        menuAnchorItem: ui.rootItem
        onHandleMenuItem: function(itemId) {
            root.changeZoomRequested(itemId)
        }
    }
}
