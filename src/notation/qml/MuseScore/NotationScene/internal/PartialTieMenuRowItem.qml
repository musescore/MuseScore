/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

ListItemBlank {
    function calculateWidth() {
        let result = 0

        result += rowLayout.anchors.leftMargin

        result += Math.ceil(checkIcon.Layout.preferredWidth)
        result += rowLayout.spacing

        result += Math.ceil(titleLabel.implicitWidth)
        result += rowLayout.spacing
        result += rowLayout.anchors.rightMargin

        return result
    }

    id: rowDelegate

    required property var modelData
    required property var model

    RowLayout {
        id: rowLayout

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12

        spacing: 12

        StyledIconLabel {
            id: checkIcon
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: 16
            iconCode: rowDelegate.modelData.checked ? IconCode.TICK_RIGHT_ANGLE : IconCode.NONE
        }

        StyledTextLabel {
            id: titleLabel
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignLeft

            text: rowDelegate.modelData.title

            textFormat: Text.RichText
        }
    }
}
