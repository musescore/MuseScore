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
import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Autobot 1.0

Rectangle {

    id: root

    property string path: ""

    signal finished()

    color: ui.theme.backgroundPrimaryColor

    visible: false

    Component.onCompleted: {
        runModel.init()
    }

    function run(path) {
        root.visible = true

        if (root.path === path) {
            //! NOTE Just show panel with current TC
            return
        }

        root.path = path
        runModel.loadInfo(path)
        runModel.perform()
    }

    function close() {
        root.visible = false
    }

    MouseArea {
        anchors.fill: parent
    }

    TestCaseRunModel {
        id: runModel

        onCurrentStepChanged: function(stepIndex) {
            stepsView.positionViewAtIndex(stepIndex, ListView.Center)
        }

        onStatusChanged: {
            if (runModel.status === "Finished") {
                Qt.callLater(root.finished)
            }
        }
    }

    Item {
        id: toolBar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: 40

        FlatButton {
            id: back
            anchors.verticalCenter: parent.verticalCenter
            icon: IconCode.ARROW_LEFT
            onClicked: root.close()
        }

        FlatButton {
            id: runBtn
            anchors.right: abortBtn.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: (runModel.status === "Running") ? "Pause" : "Run"
            onClicked: runModel.perform()
        }

        FlatButton {
            id: abortBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: "Abort"
            onClicked: runModel.abort()
        }
    }

    Item {
        id: infoPanel
        anchors.top: toolBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8

        height: 48

        StyledTextLabel {
            id: titleLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignLeft
            text: runModel.testCase.name ?? ""
        }

        StyledTextLabel {
            anchors.top: titleLabel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 2
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: titleLabel.font.pixelSize / 1.2
            text: runModel.testCase.description ?? ""
        }
    }

    StyledTextLabel {
        id: stepsInfos
        anchors.top: infoPanel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: 32
        horizontalAlignment: Text.AlignLeft
        text: "Steps (" + runModel.testCase.stepsCount + "):"
    }

    ListView {
        id: stepsView
        anchors.top: stepsInfos.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        clip: true

        model: runModel.steps

        delegate: ListItemBlank {
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 48

            StyledTextLabel {
                id: stepTitle
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignLeft
                text: (model.index+1) + ": " + modelData.name
            }

            StyledTextLabel {
                anchors.top: stepTitle.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                horizontalAlignment: Text.AlignLeft
                text: {
                    var duration = modelData.duration
                    if (duration !== "") {
                        return modelData.status + " [" + duration + "]"
                    }

                    modelData.status
                }
            }
        }
    }
}
