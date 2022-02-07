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

import "internal"

InfoPanel {
    id: root

    property string neutralButtonTitle: ""

    property bool isEnabled: false

    signal neutralButtonClicked()
    signal enabledChanged(bool enabled)

    buttonsPanel: RowLayout {
        id: buttons

        spacing: 19

        FlatButton {
            id: neutralButton
            Layout.alignment: Qt.AlignLeft

            navigation.name: "NeutralButton"
            navigation.panel: root.contentNavigation
            navigation.column: 1

            text: Boolean(root.neutralButtonTitle) ? root.neutralButtonTitle : ""

            onClicked: {
                root.neutralButtonClicked()
            }
        }

        FlatButton {
            id: mainButton
            Layout.alignment: Qt.AlignRight

            navigation.name: text + "Button"
            navigation.panel: root.contentNavigation
            navigation.column: 3

            text: !root.isEnabled ? qsTrc("uicomponents", "Enable") : qsTrc("uicomponents", "Disable")

            Component.onCompleted: {
                root.mainButton = mainButton
            }

            onClicked: {
                root.enabledChanged(!root.isEnabled)
            }
        }
    }
}
