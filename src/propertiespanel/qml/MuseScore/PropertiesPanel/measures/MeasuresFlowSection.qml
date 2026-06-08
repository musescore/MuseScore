/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.PropertiesPanel

Column {
    id: root
    width: parent.width
    spacing: 12

    property var navigationPanel
    property int navigationRowStart: 0
    property bool isLocked: false
    property int itemCount: 0

    property string lockLabelSingular: ""
    property string lockLabelPlural: ""
    property string unlockLabelSingular: ""
    property string unlockLabelPlural: ""

    property string lockToolTipTitle: ""
    property string lockToolTipDescription: ""
    property string lockToolTipShortcut: ""
    property string lockNavigationName: ""

    property string moveSectionTitle: ""
    property string movePrevText: ""
    property string moveNextText: ""

    property string movePreviousToolTipTitle: ""
    property string moveNextToolTipTitle: ""
    property string movePreviousShortcut: ""
    property string moveNextShortcut: ""

    property string previousNavigationName: ""
    property string nextNavigationName: ""

    property string makeIntoText: ""
    property string makeIntoToolTipTitle: ""
    property string makeIntoToolTipDescription: ""
    property string makeIntoShortcut: ""
    property string makeIntoNavigationName: ""

    property bool isMakeIntoAvailable: false

    property var controller

    FlatButton {
        id: toggleLockButton
        visible: root.visible
        width: parent.width

        navigation.panel: root.navigationPanel
        navigation.name: root.lockNavigationName
        navigation.row: root.navigationRowStart + 1

        orientation: Qt.Horizontal
        icon: root.isLocked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
        text: root.isLocked
            ? (root.itemCount > 1 ? root.unlockLabelPlural : root.unlockLabelSingular)
            : (root.itemCount > 1 ? root.lockLabelPlural : root.lockLabelSingular)

        toolTipTitle: root.lockToolTipTitle
        toolTipDescription: root.lockToolTipDescription
        toolTipShortcut: root.lockToolTipShortcut

        accentButton: root.isLocked

        onClicked: root.controller.toggleLock()
    }

    Column {
        width: parent.width
        spacing: 8

        StyledTextLabel {
            width: parent.width
            visible: root.visible
            horizontalAlignment: Qt.AlignLeft
            text: root.moveSectionTitle
        }

        Row {
            id: moveButtons
            visible: root.visible
            width: parent.width
            spacing: 4

            FlatButton {
                width: (moveButtons.width - moveButtons.spacing) / 2

                navigation.panel: root.navigationPanel
                navigation.name: root.previousNavigationName
                navigation.row: root.navigationRowStart + 2

                orientation: Qt.Horizontal
                icon: IconCode.ARROW_UP
                text: root.movePrevText

                toolTipTitle: root.movePreviousToolTipTitle
                toolTipShortcut: root.movePreviousShortcut

                onClicked: root.controller.movePrevious()
            }

            FlatButton {
                id: moveNextButton
                width: (moveButtons.width - moveButtons.spacing) / 2

                navigation.panel: root.navigationPanel
                navigation.name: root.nextNavigationName
                navigation.row: root.navigationRowStart + 3

                orientation: Qt.Horizontal
                icon: IconCode.ARROW_DOWN
                text: root.moveNextText

                toolTipTitle: root.moveNextToolTipTitle
                toolTipShortcut: root.moveNextShortcut

                onClicked: root.controller.moveNext()
            }
        }
    }

    FlatButton {
        id: makeIntoButton
        visible: root.visible
        enabled: root.isMakeIntoAvailable
        width: parent.width

        navigation.panel: root.navigationPanel
        navigation.name: root.makeIntoNavigationName
        navigation.row: root.navigationRowStart + 4

        orientation: Qt.Horizontal
        text: root.makeIntoText

        toolTipTitle: root.makeIntoToolTipTitle
        toolTipDescription: root.makeIntoToolTipDescription
        toolTipShortcut: root.makeIntoShortcut

        onClicked: root.controller.makeInto()
    }
}
