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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.InstrumentsScene 1.0

Item {
    id: root

    property string preferredScoreCreationMode: ""

    property string description: Boolean(currentPage) ? currentPage.description : ""

    property NavigationSection navigationSection: null

    readonly property bool hasSelection: {
        if (!currentPage) {
            return false
        }

        if (bar.currentIndex === 0) {
            return currentPage.hasSelectedInstruments
        } else if (bar.currentIndex === 1) {
            return currentPage.hasSelectedTemplate
        }

        return false
    }

    readonly property Item currentPage: pageLoader.item

    signal done

    function result() {
        var result = {}

        switch(bar.currentIndex) {
        case 0:
            result["scoreOrder"] = currentPage.currentOrder()
            result["instruments"] = currentPage.instruments()
            break
        case 1:
            result["templatePath"] = currentPage.selectedTemplatePath
            break
        }

        return result
    }

    function focusOnSelected() {
        if (pageLoader.item) {
            pageLoader.item.navigation.requestActive()
        }
    }

    Component.onCompleted: {
        theInstrumentsOnScoreModel.load()
    }

    InstrumentsOnScoreListModel {
        id: theInstrumentsOnScoreModel
    }

    StyledTabBar {
        id: bar

        property bool completed: false

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        onCurrentItemChanged: {
            if (currentItem && currentItem.navigation) {
                currentItemNavigationIndex = [currentItem.navigation.row, currentItem.navigation.column]
                root.focusOnSelected()
            }
        }

        property var currentItemNavigationIndex: []

        NavigationPanel {
            id: topNavPanel

            name: "ChooseTabPanel"
            section: root.navigationSection
            order: 1

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlIndex", bar.currentItemNavigationIndex)
                }
            }
        }

        StyledTabButton {
            id: chooseInstrumentsBtn

            text: qsTrc("project/newscore", "Choose instruments")

            navigation.name: "Choose instruments"
            navigation.panel: topNavPanel
            navigation.column: 0
        }

        StyledTabButton {
            id: createFromTemplateBtn

            text: qsTrc("project/newscore", "Create from template")

            navigation.name: "Create from template"
            navigation.panel: topNavPanel
            navigation.column: 1
        }

        Component.onCompleted: {
            switch(root.preferredScoreCreationMode) {
            case "FromInstruments":
                currentIndex = 0
                break
            case "FromTemplate":
                currentIndex = 1
                break
            }

            bar.completed = true
        }
    }

    Loader {
        id: pageLoader

        anchors.top: bar.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        sourceComponent: {
            if (!bar.completed) {
                return null
            }

            switch(bar.currentIndex) {
            case 0: return instrumentsPageComp
            case 1: return templatePageComp
            }

            return null
        }
    }

    Component {
        id: instrumentsPageComp

        ChooseInstrumentsPage {
            navigationSection: root.navigationSection
            instrumentsOnScoreModel: theInstrumentsOnScoreModel
        }
    }

    Component {
        id: templatePageComp

        CreateFromTemplatePage {
            property string description: ""

            navigationSection: root.navigationSection

            onDone: {
                root.done()
            }
        }
    }
}
