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

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

import "../../../common"
import "../../../"

FocusableItem {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        TextSection {
            id: beginningTextSection
            titleText: qsTrc("inspector", "Beginning text")
            propertyItem: root.model ? root.model.beginingText : null

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1
        }

        OffsetSection {
            id: beginningTextOffsetSection
            titleText: qsTrc("inspector", "Vertical offset")
            verticalOffset: root.model ? root.model.beginingTextVerticalOffset : null

            navigationPanel: root.navigationPanel
            navigationRowStart: beginningTextSection.navigationRowEnd + 1
        }

        SeparatorLine { anchors.margins: -12 }

        TextSection {
            id: continiousTextSection
            titleText:  qsTrc("inspector", "Text when continuing to a new system")
            propertyItem: root.model ? root.model.continiousText : null

            navigationPanel: root.navigationPanel
            navigationRowStart: beginningTextOffsetSection.navigationRowEnd + 1
        }

        OffsetSection {
            id: continiousTextOffsetSection
            titleText: qsTrc("inspector", "Vertical offset")
            verticalOffset: root.model ? root.model.continiousTextVerticalOffset : null

            navigationPanel: root.navigationPanel
            navigationRowStart: continiousTextSection.navigationRowEnd + 1
        }

        SeparatorLine { anchors.margins: -12; visible: endTextSection.visible }

        TextSection {
            id: endTextSection
            titleText:  qsTrc("inspector", "End text")
            propertyItem: root.model ? root.model.endText : null

            navigationPanel: root.navigationPanel
            navigationRowStart: continiousTextOffsetSection.navigationRowEnd + 1
        }

        OffsetSection {
            id: endTextOffsetSection
            titleText: qsTrc("inspector", "Vertical offset")
            verticalOffset: root.model ? root.model.endTextVerticalOffset : null

            navigationPanel: root.navigationPanel
            navigationRowStart: endTextSection.navigationRowEnd + 1
        }
    }
}
