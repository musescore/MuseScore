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
import MuseScore.Shortcuts 1.0

Item {
    id: root

    ShortcutsModel {
        id: shortcutsModel

        selection: view.selection
    }

    function apply() {
        return shortcutsModel.apply()
    }

    Component.onCompleted: {
        shortcutsModel.load()
    }

    QtObject {
        id: privateProperties

        readonly property int buttonWidth: 105
    }

    EditShortcutDialog {
        id: editShortcutDialog

        onApplySequenceRequested: {
            shortcutsModel.applySequenceToCurrentShortcut(newSequence)
        }

        property bool canEditCurrentShortcut: Boolean(shortcutsModel.currentSequence)

        function startEditCurrentShortcut() {
            editShortcutDialog.startEdit(shortcutsModel.currentSequence, shortcutsModel.shortcuts())
        }
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Define...")
                enabled: editShortcutDialog.canEditCurrentShortcut

                onClicked: {
                    editShortcutDialog.startEditCurrentShortcut()
                }
            }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Clear")
                enabled: view.hasSelection

                onClicked: {
                    shortcutsModel.clearSelectedShortcuts()
                }
            }

            Item { Layout.fillWidth: true }

            SearchField {
                id: searchField

                Layout.preferredWidth: 160

                hint: qsTrc("shortcuts", "Search shortcut")
            }
        }

        ValueList {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true

            keyRoleName: "title"
            keyTitle: qsTrc("shortcuts", "action")
            valueRoleName: "sequence"
            valueTitle: qsTrc("shortcuts", "shortcut")
            iconRoleName: "icon"
            readOnly: true

            model: SortFilterProxyModel {
                sourceModel: shortcutsModel

                filters: [
                    FilterValue {
                        roleName: "searchKey"
                        roleValue: searchField.searchText
                        compareType: CompareType.Contains
                    }
                ]
            }

            onDoubleClicked: {
                editShortcutDialog.startEditCurrentShortcut()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: childrenRect.height

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Load...")

                onClicked: {
                    shortcutsModel.loadShortcutsFromFile()
                }
            }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Save")

                onClicked: {
                    shortcutsModel.saveShortcutsToFile()
                }
            }

            Item { Layout.fillWidth: true }

            FlatButton {
                Layout.preferredWidth: privateProperties.buttonWidth

                text: qsTrc("shortcuts", "Reset to default")
                enabled: view.hasSelection

                onClicked: {
                    shortcutsModel.resetToDefaultSelectedShortcuts()
                }
            }
        }
    }
}
