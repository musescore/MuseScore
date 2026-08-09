/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

AbstractElementPopup {
    id: root

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: organPedalMarksNavPanel.order

    property int currentPage: 0

    margins: 4
    padding: 4

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    focusPolicies: PopupView.DefaultFocus & ~PopupView.ClickFocus
    placementPolicies: organPedalMarkModel.placeAbove ? PopupView.PreferAbove : PopupView.PreferBelow

    model: OrganPedalMarkPopupModel {
        id: organPedalMarkModel
    }

    RowLayout {
        id: content

        spacing: padding

        NavigationPanel {
            id: organPedalMarksNavPanel
            name: "OrganPedalMarksPopup"
            direction: NavigationPanel.Horizontal
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Organ pedal marks popup")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
        }

        function goToPreviousPage() {
            if (root.currentPage > 0) {
                root.currentPage--
            } else {
                root.currentPage = organPedalMarkModel.pages.length - 1
            }

            Qt.callLater(requestNavigationActive, pedalMarksRepeater.count - 1)
        }

        function goToNextPage() {
            if (root.currentPage < organPedalMarkModel.pages.length - 1) {
                root.currentPage++
            } else {
                root.currentPage = 0
            }

            Qt.callLater(requestNavigationActive, 0)
        }

        function requestNavigationActive(index) {
            (pedalMarksRepeater.itemAt(index) as FlatButton).navigation.requestActive()
        }

        FlatButton {
            id: leftButton

            implicitWidth: 16
            transparent: true

            contentItem: StyledIconLabel {
                id: leftArrowLabel
                iconCode: IconCode.CHEVRON_LEFT
                font.pixelSize: 16
            }

            onClicked: {
                content.goToPreviousPage()
            }
        }

        Repeater {
            id: pedalMarksRepeater

            model: organPedalMarkModel.pages[root.currentPage]

            delegate: FlatButton {
                id: pedalMark

                required property var modelData
                required property int index

                minWidth: 2
                transparent: true

                navigation.panel: organPedalMarksNavPanel
                navigation.order: index
                accessible.name: modelData.accessibleName
                navigation.onNavigationEvent: function(event) {
                    switch (event.type) {
                    case NavigationEvent.Up:
                    case NavigationEvent.Left: {
                        if (index == 0) {
                            content.goToPreviousPage()

                            event.accepted = true
                        }

                        break
                    }

                    case NavigationEvent.Right:
                    case NavigationEvent.Down: {
                        if (index == pedalMarksRepeater.count - 1) {
                            content.goToNextPage()

                            event.accepted = true
                        }

                        break
                    }
                    }
                }

                StyledTextLabel {
                    id: pedalMarkLabel

                    text: modelData.pedalMark
                    font.pixelSize: 30

                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: modelData.offset
                }

                onClicked: {
                    organPedalMarkModel.changePedalMark(root.currentPage, index)
                }
            }
        }

        FlatButton {
            id: rightButton

            implicitWidth: 16
            transparent: true

            contentItem: StyledIconLabel {
                id: rightArrowLabel
                iconCode: IconCode.CHEVRON_RIGHT
                font.pixelSize: 16
            }

            onClicked: {
                content.goToNextPage()
            }
        }
    }
}