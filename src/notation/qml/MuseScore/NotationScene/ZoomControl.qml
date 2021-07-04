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
import QtQuick.Layouts 1.15

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

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
    signal changeZoomRequested(var newZoomIndex)
    signal zoomInRequested()
    signal zoomOutRequested()

    spacing: 0

    FlatButton {
        id: zoomInButton
        icon: IconCode.ZOOM_IN
        iconFont: ui.theme.toolbarIconsFont

        normalStateColor: "transparent"

        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrderMin

        onClicked: {
            root.zoomInRequested()
        }
    }

    FlatButton {
        id: zoomOutButton
        Layout.leftMargin: 4

        icon: IconCode.ZOOM_OUT
        iconFont: ui.theme.toolbarIconsFont

        normalStateColor: "transparent"

        navigation.panel: root.navigationPanel
        navigation.order: zoomInButton.navigation.order + 1

        onClicked: {
            root.zoomOutRequested()
        }
    }

    Row {
        Layout.leftMargin: 12
        Layout.fillHeight: true

        spacing: 1

        NumberInputField {
            id: zoomInputField

            anchors.verticalCenter: parent.verticalCenter

            addLeadingZeros: false
            font: ui.theme.bodyFont

            navigation.panel: root.navigationPanel
            navigation.order: zoomOutButton.navigation.order + 1

            onValueEdited: {
                root.changeZoomPercentageRequested(newValue)
            }
        }

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            text: "%"
        }
    }

    FlatButton {
        id: menuButton
        Layout.leftMargin: 4
        Layout.preferredWidth: 20

        icon: IconCode.SMALL_ARROW_DOWN

        normalStateColor: menuLoader.isMenuOpened ? ui.theme.accentColor : "transparent"

        navigation.panel: root.navigationPanel
        navigation.order: zoomInputField.navigation.order + 1

        StyledMenuLoader {
            id: menuLoader

            menuAnchorItem: ui.rootItem

            onHandleAction: {
                root.changeZoomRequested(actionIndex)
            }
        }

        onClicked: {
            menuLoader.toggleOpened(root.availableZoomList, parent.navigation)
        }
    }
}
