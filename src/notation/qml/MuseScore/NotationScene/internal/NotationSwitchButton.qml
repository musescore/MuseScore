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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatRadioButton {
    id: root

    property bool needSave: false

    signal closeRequested()

    width: Math.min(200, implicitContentWidth)
    radius: 0

    checkedColor: ui.theme.backgroundPrimaryColor

    RowLayout {
        id: contentRow
        anchors.fill: parent
        spacing: 4

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: root.implicitContentWidth > 200
            Layout.preferredWidth: implicitWidth
            Layout.leftMargin: 12

            horizontalAlignment: Text.AlignLeft

            text: root.text + (root.needSave ? "*" : "")
        }

        FlatButton {
            Layout.preferredHeight: 20
            Layout.preferredWidth: height
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            transparent: true
            icon: IconCode.CLOSE_X_ROUNDED
            iconFont {
                family: ui.theme.iconsFont.family
                pixelSize: 12
            }

            onClicked: {
                root.closeRequested()
            }
        }

        SeparatorLine { orientation: Qt.Vertical }
    }
}
