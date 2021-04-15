/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

QmlDialog {
    id: root

    height: 600
    width: 1024

    title: qsTrc("userscores", "New Score")

    Rectangle {

        NewScoreModel {
            id: newScoreModel
        }

        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor

        StackLayout {
            id: pagesStack
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.margins: 20

            ChooseInstrumentsAndTemplatesPage {
                id: chooseInstrumentsAndTemplatePage

                Component.onCompleted: {
                    preferredScoreCreationMode = newScoreModel.preferredScoreCreationMode()
                }
            }

            ScoreInfoPage {
                id: scoreInfoPage
            }
        }

        Row {
            id: buttons
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            anchors.right: parent.right
            anchors.rightMargin: 16

            spacing: 12

            readonly property int buttonHeight: 30
            readonly property int buttonWidth: 132

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                visible: pagesStack.currentIndex > 0

                text: qsTrc("userscores", "Back")

                onClicked: {
                    pagesStack.currentIndex--
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                visible: pagesStack.currentIndex < pagesStack.count - 1
                enabled: chooseInstrumentsAndTemplatePage.hasSelection

                text: qsTrc("userscores", "Next")

                onClicked: {
                    pagesStack.currentIndex++
                }
            }

            FlatButton {
                height: buttons.buttonHeight
                width: buttons.buttonWidth

                enabled: chooseInstrumentsAndTemplatePage.hasSelection

                text: qsTrc("userscores", "Done")

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
                        root.hide()
                    }
                }
            }
        }
    }
}
