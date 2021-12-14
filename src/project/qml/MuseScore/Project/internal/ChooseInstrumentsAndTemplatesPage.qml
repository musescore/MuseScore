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

    property string description: pagesStack.currentIndex === 0
                                 ? instrumentsPage.description
                                 : ""

    property NavigationSection navigationSection: null

    readonly property bool hasSelection: {
        if (pagesStack.currentIndex === 0) {
            return instrumentsPage.hasSelectedInstruments
        } else if (pagesStack.currentIndex === 1) {
            return templatePage.hasSelectedTemplate
        }

        return false
    }

    signal done

    function result() {
        var result = {}

        if (pagesStack.currentIndex === 0) {
            result["scoreOrder"] = instrumentsPage.currentOrder()
            result["instruments"] = instrumentsPage.instruments()
        } else if (pagesStack.currentIndex === 1) {
            result["templatePath"] = templatePage.selectedTemplatePath
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
            onNavigationTriggered: {
                bar.currentItemNavigationIndex = [navigation.row, navigation.column]
                bar.currentIndex = 0
            }
        }

        StyledTabButton {
            id: chooseFromTemplateBtn
            text: qsTrc("project", "Choose from template")

            navigation.name: "Choose from template"
            navigation.panel: topNavPanel
            navigation.column: 1
            onNavigationTriggered: {
                bar.currentItemNavigationIndex = [navigation.row, navigation.column]
                bar.currentIndex = 1
            }
        }

        Component.onCompleted: {
            if (root.preferredScoreCreationMode === "FromInstruments") {
                currentIndex = 0
                chooseInstrumentsBtn.navigation.requestActive()
            } else if (root.preferredScoreCreationMode === "FromTemplate") {
                currentIndex = 1
                chooseFromTemplateBtn.navigation.requestActive()
            }
        }
    }

    StackLayout {
        id: pagesStack

        anchors.top: bar.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        ChooseInstrumentsPage {
            id: instrumentsPage
            navigationSection: root.navigationSection
        }

        ChooseTemplatePage {
            id: templatePage
            navigationSection: root.navigationSection

            onDone: {
                root.done()
            }
        }
    }
}
