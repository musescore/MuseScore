/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import "../common"
import "../measures"
import "."

PropertiesPanelSection {
    id: root

    required property SystemPageLayoutSettingsModel model

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        PropertiesPanelTabBar {
            id: tabBar

            PropertiesPanelTabButton {
                text: qsTrc("propertiespanel", "System")

                navigation.name: "SystemTab"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 1

                maxWidth: tabBar.truncatedItemWidth
            }

            PropertiesPanelTabButton {
                text: qsTrc("propertiespanel", "Page")

                navigation.name: "PageTab"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationRowStart + 2

                maxWidth: tabBar.truncatedItemWidth
            }
        }

        StackLayout {
            id: lockMenu
            width: parent.width
            currentIndex: tabBar.currentIndex

            height: itemAt(currentIndex).implicitHeight

            MeasuresFlowSection {
                id: systemSection
                visible: root.model ? root.model.scoreIsInPageView : false
                navigationPanel: root.navigationPanel
                navigationRowStart: root.navigationRowStart
                isLocked: root.model ? root.model.allSystemsAreLocked : false
                itemCount: root.model ? root.model.systemCount : 0
                lockLabel: qsTrc("propertiespanel", "Lock selected system(s)", "", root.model ? root.model.systemCount : 0)
                unlockLabel: qsTrc("propertiespanel", "Unlock selected system(s)", "", root.model ? root.model.systemCount : 0)
                lockToolTipTitle: qsTrc("propertiespanel", "Lock/unlock selected system(s)", "", root.model ? root.model.systemCount : 0)
                lockToolTipDescription: qsTrc("propertiespanel", "Keep measures on the selected system(s) together and prevent them from reflowing to the next system", root.model ? root.model.systemCount : 0)
                lockToolTipShortcut: root.model ? root.model.shortcutToggleSystemLock : ""
                lockNavigationName: "SystemLock"

                moveSectionTitle: qsTrc("propertiespanel", "Move measures across systems")
                movePrevText: qsTrc("propertiespanel", "Previous")
                moveNextText: qsTrc("propertiespanel", "Next")
                movePreviousToolTipTitle: qsTrc("propertiespanel", "Move measure(s) to previous system")
                moveNextToolTipTitle: qsTrc("propertiespanel", "Move measure(s) to next system")
                movePreviousShortcut: root.model?.shortcutMoveMeasureUpSystem ?? ""
                moveNextShortcut: root.model ? root.model.shortcutMoveMeasureDownSystem : ""
                previousNavigationName: "SystemUp"
                nextNavigationName: "SystemDown"

                makeIntoText: qsTrc("propertiespanel", "New system from selection")
                makeIntoToolTipTitle: qsTrc("propertiespanel", "New system from selection")
                makeIntoToolTipDescription: qsTrc("propertiespanel", "Create a system containing only the selected measure(s)")
                makeIntoShortcut: root.model ? root.model.shortcutMakeIntoSystem : ""
                makeIntoNavigationName: "MakeSystem"
                isMakeIntoAvailable: root.model ? root.model.isMakeIntoSystemAvailable : false

                controller: ({
                    movePrevious: () => root.model.moveMeasureUpSystem(),
                    moveNext: () => root.model.moveMeasureDownSystem(),
                    toggleLock: () => root.model.toggleSystemLock(),
                    makeInto: () => root.model.makeIntoSystem()
                })
            }

            MeasuresFlowSection {
                id: pageSection
                visible: root.model ? root.model.scoreIsInPageView : false
                navigationPanel: root.navigationPanel
                navigationRowStart: root.navigationRowStart
                isLocked: root.model ? root.model.allPagesAreLocked : false
                itemCount: root.model ? root.model.pageCount : 0
                lockLabel: qsTrc("propertiespanel", "Lock selected page(s)", "", root.model ? root.model.pageCount : 0)
                unlockLabel: qsTrc("propertiespanel", "Unlock selected page(s)", "", root.model ? root.model.pageCount : 0)
                lockToolTipTitle: qsTrc("propertiespanel", "Lock/unlock selected page(s)", root.model ? root.model.pageCount : 0)
                lockToolTipDescription: qsTrc("propertiespanel", "Keep measures on the selected page(s) together and prevent them from reflowing to the next page", root.model ? root.model.pageCount : 0)
                lockToolTipShortcut: root.model ? root.model.shortcutTogglePageLock : ""
                lockNavigationName: "PageLock"

                moveSectionTitle: qsTrc("propertiespanel", "Move systems across pages")
                movePrevText: qsTrc("propertiespanel", "Previous")
                moveNextText: qsTrc("propertiespanel", "Next")
                movePreviousToolTipTitle: qsTrc("propertiespanel", "Move system(s) to previous page")
                moveNextToolTipTitle: qsTrc("propertiespanel", "Move system(s) to next page")
                movePreviousShortcut: root.model?.shortcutMoveSystemUpPage ?? ""
                moveNextShortcut: root.model ? root.model.shortcutMoveSystemDownPage : ""
                previousNavigationName: "PageUp"
                nextNavigationName: "PageDown"

                makeIntoText: qsTrc("propertiespanel", "New page from selection")
                makeIntoToolTipTitle: qsTrc("propertiespanel", "New page from selection")
                makeIntoToolTipDescription: qsTrc("propertiespanel", "Create a page containing only the selected measure(s)")
                makeIntoShortcut: root.model ? root.model.shortcutMakeIntoPage : ""
                makeIntoNavigationName: "MakePage"
                isMakeIntoAvailable: root.model ? root.model.isMakeIntoPageAvailable : false

                controller: ({
                    movePrevious: () => root.model.moveSystemUpPage(),
                    moveNext: () => root.model.moveSystemDownPage(),
                    toggleLock: () => root.model.togglePageLock(),
                    makeInto: () => root.model.makeIntoPage()
                })
            }
        }
    }
}
