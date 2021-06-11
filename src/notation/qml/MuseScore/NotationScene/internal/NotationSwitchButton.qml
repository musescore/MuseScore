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
import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatRadioButton {
    id: root

    property string title: ""
    property bool needSave: false

    signal closeRequested()

    normalStateColor: ui.theme.backgroundSecondaryColor
    hoverStateColor: selectedStateColor
    pressedStateColor: selectedStateColor
    selectedStateColor: ui.theme.backgroundPrimaryColor

    width: 200
    radius: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft

            text: root.title + (root.needSave ? "*" : "")
        }

        FlatButton {
            Layout.preferredHeight: 14
            Layout.preferredWidth: height
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            normalStateColor: "transparent"
            icon: IconCode.CLOSE_X_ROUNDED
            onClicked: root.closeRequested()
        }

        SeparatorLine { orientation: Qt.Vertical }
    }
}
