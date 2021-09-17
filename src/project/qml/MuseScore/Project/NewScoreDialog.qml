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

import "internal"

StyledDialogView {
    id: root

    title: qsTrc("project", "New Score")

    contentHeight: 600
    contentWidth: 1024
    resizable: true

    onOpened: {
        chooseInstrumentsAndTemplatePage.focusOnFirst()
    }

    NewScoreModel {
        id: newScoreModel
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 16
        anchors.bottomMargin: 16

        spacing: 40

        StackLayout {
            id: pagesStack

            Layout.fillHeight: true
            Layout.fillWidth: true

            ChooseInstrumentsAndTemplatesPage {
                id: chooseInstrumentsAndTemplatePage

                navigationSection: root.navigation

                Component.onCompleted: {
                    preferredScoreCreationMode = newScoreModel.preferredScoreCreationMode()
                }
            }

            ScoreInfoPage {
                id: scoreInfoPage

                navigation: root.navigation
                popupsAnchorItem: content
            }

            onCurrentIndexChanged: {
                switch(currentIndex) {
                case 0:
                    chooseInstrumentsAndTemplatePage.focusOnFirst()
                    break
                case 1:
                    scoreInfoPage.focusOnFirst()
                    break
                }
            }
        }

        Row {
            id: buttons

            Layout.alignment: Qt.AlignRight

            spacing: 12

            readonly property int buttonHeight: 30
            readonly property int buttonWidth: 132

            NavigationPanel {
                id: navBottomPanel

                name: "BottomPanel"
                section: root.navigationSection
                order: 100
                direction: NavigationPanel.Horizontal
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                navigation.name: "Cancel"
                navigation.panel: navBottomPanel
                navigation.column: 1

                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                navigation.name: "Back"
                navigation.panel: navBottomPanel
                navigation.column: 2

                visible: pagesStack.currentIndex > 0

                text: qsTrc("project", "Back")

                onClicked: {
                    pagesStack.currentIndex--
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                navigation.name: "Next"
                navigation.panel: navBottomPanel
                navigation.column: 3

                visible: pagesStack.currentIndex < pagesStack.count - 1
                enabled: chooseInstrumentsAndTemplatePage.hasSelection

                text: qsTrc("project", "Next")

                onClicked: {
                    pagesStack.currentIndex++
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                navigation.name: "Done"
                navigation.panel: navBottomPanel
                navigation.column: 4

                enabled: chooseInstrumentsAndTemplatePage.hasSelection

                text: qsTrc("project", "Done")

                onClicked: {
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
                        root.isDoActiveParentOnClose = false
                        root.accept()
                    }
                }
            }
        }
    }
}
