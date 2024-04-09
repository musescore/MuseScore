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

StyledDialogView {
    id: root

    property string filePath: ""

    title: "Autobot Select File"

    contentHeight: 400
    contentWidth: 400

    Component.onCompleted: {
        autoClose.start()
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    StyledTextLabel {
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        text: "The file will be saved here:\n" + root.filePath
    }

    Timer {
        id: autoClose
        interval: 2000
        onTriggered: root.close()
    }
}
