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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopupView {
    id: root

    property QtObject model: undefined

    contentHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        WidthSection {
            widthProperty: model ? model.frameWidth : null
        }

        SeparatorLine { anchors.margins: -10 }

        HorizontalGapsSection {
            leftGap: model ? model.leftGap : null
            rightGap: model ? model.rightGap: null
        }

        SeparatorLine { anchors.margins: -10 }

        CheckBox {
            isIndeterminate: model ? model.shouldDisplayKeysAndBrackets.isUndefined : false
            checked: model && !isIndeterminate ? model.shouldDisplayKeysAndBrackets.value : false
            text: qsTrc("inspector", "Display key, brackets and braces")

            onClicked: { model.shouldDisplayKeysAndBrackets.value = !checked }
        }
    }
}
