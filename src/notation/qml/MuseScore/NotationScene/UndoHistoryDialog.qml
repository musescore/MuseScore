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
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledDialogView {
    id: root
    
    title: qsTrc("notation", "Undo/redo history")

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight
    margins: 16

    Component.onCompleted: {
        model.load()
        populateList()
    }

    UndoRedoModel {
        id: model
    }

    modal: true

    property int selectedIndex: (() => {
        var val = model.undoRedoActionCount() - model.undoRedoActionCurrentIdx()
        if (val < 0) {
            return 0
        } else if ((val > 0) && (val >= model.undoRedoActionCount())) {
            return model.undoRedoActionCount() - 1
        }
        return val
    })()

    function populateList() {
        listView.model.clear() // Clear the existing model

        for (var i = model.undoRedoActionCount() - 1; i >= 0; i--) {
            listView.model.append({ text: model.undoRedoActionNameAtIdx(i) }) // Add each action name
        }
    }

    function actionIndex() {
        return model.undoRedoActionCount() - selectedIndex - 1
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        spacing: 20


        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 24

            ColumnLayout {
                id: contentColumn

                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.45

                spacing: 20

                property NavigationPanel navigationPanel: NavigationPanel {
                    name: "UndoRedoItemsView"
                    section: root.navigationSection
                    direction: NavigationPanel.Vertical
                    order: 1
                    accessible.name: qsTrc("notation", "Undo/redo items")
                }

                StyledListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300

                    model: ListModel {}

                    delegate: ListItemBlank {
                        navigation.panel: contentColumn.navigationPanel
                        navigation.order: index

                        StyledTextLabel {
                            text: model.text

                            anchors.fill: parent
                            anchors.centerIn: parent
                            font: Qt.font(Object.assign({}, ui.theme.bodyBoldFont, {}))
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            root.selectedIndex = index // Store selected index
                        }

                        onDoubleClicked: {
                            root.ret = { errcode: 0, value: actionIndex() }
                            root.hide()
                        }

                        isSelected: {
                            root.selectedIndex === index // Highlight selected item
                        }

                    }


                    clip: true
                }

                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 24

                    ColumnLayout {
                        id: listColumn

                        Layout.fillHeight: true
                        Layout.preferredWidth: root.width * 0.45

                        spacing: 20
                    }
                }
            }
        }

        ButtonBox {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom

            navigationPanel.section: root.navigationSection
            navigationPanel.order: 2

            // Cancel button
            FlatButton {
                text: qsTrc("global", "Cancel")
                buttonRole: ButtonBoxModel.RejectRole
                buttonId: ButtonBoxModel.Cancel
                enabled: true

                onClicked: {
                    root.reject()
                }
            }

            // Ok button
            FlatButton {
                text: qsTrc("global", "OK")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Ok
                enabled: true
                accentButton: true

                onClicked: {
                    root.ret = { errcode: 0, value: actionIndex() }
                    root.hide()
                }
            }
        }
    }
}
