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
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

InspectorPropertyView {
    id: root

    property QtObject model: null

    objectName: "TremoloSettings"

    titleText: qsTrc("inspector", "Style (between notes)")
    propertyItem: root.model ? root.model.style : null

    RadioButtonGroup {
        id: radioButtonList

        height: 30
        width: parent.width

        model: [
            { iconRole: IconCode.TREMOLO_STYLE_DEFAULT, typeRole: TremoloTypes.STYLE_DEFAULT },
            { iconRole: IconCode.TREMOLO_STYLE_TRADITIONAL, typeRole: TremoloTypes.STYLE_TRADITIONAL },
            { iconRole: IconCode.TREMOLO_STYLE_TRADITIONAL_ALTERNATE, typeRole: TremoloTypes.STYLE_TRADITIONAL_ALTERNATE }
        ]

        delegate: FlatRadioButton {
            ButtonGroup.group: radioButtonList.radioButtonGroup

            checked: root.model && !root.model.style.isUndefined ? root.model.style.value === modelData["typeRole"]
                                                                 : false

            onToggled: {
                root.model.style.value = modelData["typeRole"]
            }

            StyledIconLabel {
                iconCode: modelData["iconRole"]
            }
        }
    }
}
