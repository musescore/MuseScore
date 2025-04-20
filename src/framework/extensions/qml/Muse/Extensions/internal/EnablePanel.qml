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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

InfoPanel {
    id: root

    property bool isEnabled: false

    property var execPointsModel: null
    property int currentExecPointIndex: 0

    signal editShortcutRequested()
    signal execPointSelected(int index)
    signal remove()

    buttonsPanel: RowLayout {
        id: buttons

        spacing: 19

        // StyledDropdown {
        //     id: execPoints

        //     property string text: currentText

        //     Component.onCompleted: {
        //         root.mainButton = execPoints
        //     }

        //     Layout.alignment: Qt.AlignLeft

        //     width: 280

        //     navigation.name: "ExecPointSelector"
        //     navigation.panel: root.contentNavigation
        //     navigation.column: 3

        //     currentIndex: root.currentExecPointIndex
        //     model: root.execPointsModel

        //     onActivated: function(index, value) {
        //         currentIndex = index
        //         root.execPointSelected(index)
        //     }
        // }

        FlatButton {
            id: neutralButton

            Layout.alignment: Qt.AlignLeft

            navigation.name: "EditShortcutButton"
            navigation.panel: root.contentNavigation
            navigation.column: 1

            //: Edit the keyboard shortcut assigned to a plug-in
            text: qsTrc("extensions", "Edit shortcut")

            onClicked: {
                root.editShortcutRequested()
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 22

            FlatButton {
                id: removeButton

                navigation.name: text + "Button"
                navigation.panel: root.contentNavigation
                navigation.column: 2
                accessible.ignored: true
                navigation.onActiveChanged: {
                    if (!navigation.active) {
                        accessible.ignored = false
                    }
                }

                text: qsTrc("workspace", "Remove")

                onClicked: {
                    root.remove()
                }
            }



            FlatButton {
                id: mainButton

                navigation.name: text + "Button"
                navigation.panel: root.contentNavigation
                navigation.column: 3
                accessible.ignored: true
                navigation.onActiveChanged: {
                    if (!navigation.active) {
                        accessible.ignored = false
                    }
                }

                text: !root.isEnabled ? qsTrc("extensions", "Enable") : qsTrc("extensions", "Disable")

                Component.onCompleted: {
                    root.mainButton = mainButton
                }

                onClicked: {
                    //! NOTE temporary
                    // The function with the choice of the call point is not ready yet.
                    // Therefore, here is the previous solution with the button,
                    // but in fact the choice is made from the list
                    // 0 - disabled
                    // 1 - enabled (manual call)
                    // (here we switch to the opposite state)
                    root.execPointSelected(root.isEnabled ? 0 : 1)
                }
            }
        }
    }
}
