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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/NewScore"

StyledDialogView {
    id: root

    title: qsTrc("project/newscore", "New Score")

    contentHeight: 600
    contentWidth: 1024
    resizable: true

    objectName: "NewScoreDialog"

    function onDone() {
        var result = {}

        var instrumentsAndTemplatePageResult = chooseInstrumentsAndTemplatePage.result()
        for (var key in instrumentsAndTemplatePageResult) {
            result[key] = instrumentsAndTemplatePageResult[key]
        }

        var scoreInfoPageResult = scoreInfoPage.result()
        for (key in scoreInfoPageResult) {
            result[key] = scoreInfoPageResult[key]
        }

        if (newScoreModel.createScore(result)) {
            root.activateParentOnClose = false
            root.accept()
        }
    }

    onOpened: {
        chooseInstrumentsAndTemplatePage.focusOnSelected()
    }

    NewScoreModel {
        id: newScoreModel
    }

    Item {
        id: popupsAnchorItem
        anchors.fill: parent
        anchors.margins: 8
    }

    StackLayout {
        id: pagesStack

        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: footer.top
        anchors.bottomMargin: 20

        ChooseInstrumentsAndTemplatesPage {
            id: chooseInstrumentsAndTemplatePage

            navigationSection: root.navigationSection

            Component.onCompleted: {
                preferredScoreCreationMode = newScoreModel.preferredScoreCreationMode()
            }

            onDone: {
                root.onDone()
            }
        }

        ScoreInfoPage {
            id: scoreInfoPage

            navigationSection: root.navigationSection
            popupsAnchorItem: popupsAnchorItem
        }
    }

    RowLayout {
        id: footer

        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

        spacing: 12

        NavigationPanel {
            id: navBottomPanel

            name: "BottomPanel"
            section: root.navigationSection
            order: 100
            direction: NavigationPanel.Horizontal
        }

        StyledTextLabel {
            id: descriptionLabel
            text: pagesStack.currentIndex === 0
                  ? chooseInstrumentsAndTemplatePage.description
                  : ""

            Layout.fillWidth: true
            Layout.maximumHeight: doneButton.height

            font: ui.theme.bodyFont
            opacity: 0.7
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }

        FlatButton {
            navigation.name: "Cancel"
            navigation.panel: navBottomPanel
            navigation.column: 1

            text: qsTrc("global", "Cancel")

            onClicked: {
                root.reject()
            }
        }

        FlatButton {
            navigation.name: "Back"
            navigation.panel: navBottomPanel
            navigation.column: 2

            visible: pagesStack.currentIndex > 0

            text: qsTrc("global", "Back")

            onClicked: {
                pagesStack.currentIndex--
            }
        }

        FlatButton {
            navigation.name: "Next"
            navigation.panel: navBottomPanel
            navigation.column: 3

            visible: pagesStack.currentIndex < pagesStack.count - 1
            enabled: chooseInstrumentsAndTemplatePage.hasSelection

            text: qsTrc("global", "Next")

            onClicked: {
                pagesStack.currentIndex++
            }
        }

        FlatButton {
            id: doneButton

            navigation.name: "Done"
            navigation.panel: navBottomPanel
            navigation.column: 4

            enabled: chooseInstrumentsAndTemplatePage.hasSelection

            text: qsTrc("global", "Done")

            onClicked: {
                root.onDone()
            }
        }
    }
}
