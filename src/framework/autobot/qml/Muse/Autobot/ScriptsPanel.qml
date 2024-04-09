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
import Muse.UiComponents 1.0
import Muse.Autobot 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    objectName: "DiagnosticAutobotScriptPanel"

    AutobotScriptsModel {
        id: scriptsModel

        onRequireStartTC: testCaseRun.run(path)
    }

    Component.onCompleted: {
        scriptsModel.load()
    }

    Item {
        id: topPanel
        anchors.left: parent.left
        anchors.right: parent.right
        height: 48

        FlatButton {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: scriptsModel.isRunAllTCMode ? "Stop Run All TC" : "Run All TC"
            onClicked: {
                if (scriptsModel.isRunAllTCMode) {
                    scriptsModel.stopRunAllTC()
                } else {
                    scriptsModel.runAllTC()
                }
            }
        }

        StyledDropdown {
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            model: ["Default", "Fast", "Normal", "Slow"]
            currentIndex: indexOfValue(scriptsModel.speedMode)
            onCurrentValueChanged:scriptsModel.speedMode = currentValue
        }
    }

    ListView {
        anchors.top: topPanel.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true

        model: scriptsModel

        section.property: "typeRole"
        section.delegate: Rectangle {
            width: parent.width
            height: 24
            color: ui.theme.backgroundSecondaryColor

            CheckBox {
                id: allSelectedCheck
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: "All"
                checked: scriptsModel.isAllSelected(section)
                onClicked: scriptsModel.toggleAllSelect(section)

                Connections {
                    target: scriptsModel
                    function onIsAllSelectedChanged() {
                        if (type === section) {
                            allSelectedCheck.checked = arg
                        }
                    }
                }
            }

            StyledTextLabel {
                anchors.left: allSelectedCheck.right
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: section
            }
        }

        delegate: ListItemBlank {
            anchors.left: parent ? parent.left : undefined
            anchors.right: parent ? parent.right : undefined
            height: 48

            CheckBox {
                id: selectedCheck
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                checked: selectedRole
                onClicked: scriptsModel.toggleSelect(indexRole)
            }

            StyledTextLabel {
                id: titleLabel
                anchors.top: parent.top
                anchors.left: selectedCheck.right
                anchors.right: parent.right
                anchors.topMargin: 8
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                horizontalAlignment: Text.AlignLeft
                text: {
                    var status = statusRole
                    if (status !== "") {
                        status = "[" + status + "] "
                    }
                    return status + titleRole
                }
            }

            StyledTextLabel {
                anchors.top: titleLabel.bottom
                anchors.left: selectedCheck.right
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                horizontalAlignment: Text.AlignLeft
                font.pixelSize: titleLabel.font.pixelSize / 1.2
                text: descriptionRole
            }

            onClicked: {
                if (typeRole === "TestCase") {
                    testCaseRun.run(pathRole)
                } else {
                    scriptsModel.stopRunAllTC()
                    scriptsModel.runScript(indexRole)
                }

            }
        }
    }

    TestCaseRunPanel {
        id: testCaseRun
        anchors.fill: parent

        onFinished: scriptsModel.tryRunNextTC()
    }
}
