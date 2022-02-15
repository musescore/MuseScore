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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0
import MuseScore.InstrumentsScene 1.0

Item {
    id: root

    property string preferredScoreCreationMode: ""

    property string description: bar.currentIndex === 0
                                 ? currentPage.description
                                 : ""

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

    readonly property var currentPage: pageLoader.item

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
        switch (bar.currentIndex) {
        case 0:
            chooseInstrumentsBtn.navigation.requestActive()
            break
        case 1:
            chooseFromTemplateBtn.navigation.requestActive()
            break
        }
    }

    StyledTabBar {
        id: bar

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        onCurrentItemChanged: {
            if (currentItem && currentItem.navigation) {
                currentItemNavigationIndex = [currentItem.navigation.row, currentItem.navigation.column]
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

            text: qsTrc("project", "Choose instruments")

            navigation.name: "Choose instruments"
            navigation.panel: topNavPanel
            navigation.column: 0
        }

        StyledTabButton {
            id: chooseFromTemplateBtn

            text: qsTrc("project", "Choose from template")

            navigation.name: "Choose from template"
            navigation.panel: topNavPanel
            navigation.column: 1
        }

        Component.onCompleted: {
            switch(root.preferredScoreCreationMode) {
            case "FromInstruments":
                currentIndex = 0
                chooseInstrumentsBtn.navigation.requestActive()
                break
            case "FromTemplate":
                currentIndex = 1
                chooseFromTemplateBtn.navigation.requestActive()
                break
            }
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
            switch(bar.currentIndex) {
            case 0: return instrumentsPageComp
            case 1: return templatePageComp
            }

            return undefined
        }
    }

    Component {
        id: instrumentsPageComp

        ChooseInstrumentsPage {
            navigationSection: root.navigationSection
        }
    }

    Component {
        id: templatePageComp

        ChooseTemplatePage {
            navigationSection: root.navigationSection

            onDone: {
                root.done()
            }
        }
    }
}
