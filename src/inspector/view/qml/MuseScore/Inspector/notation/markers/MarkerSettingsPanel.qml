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

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "MarkerSettings"

    spacing: 12

    StyledTextLabel {
        text: qsTrc("inspector", "Marker type: ") + markerTypeToString(root.model ? root.model.type : null)

        function markerTypeToString(type) {
            if (!type)
                return ""

            if (type.isUndefined)
                return "--"

            switch (type.value) {
            case MarkerTypes.TYPE_SEGNO: return qsTrc("inspector", "Segno");
            case MarkerTypes.TYPE_VARSEGNO: return qsTrc("inspector", "Segno variation")
            case MarkerTypes.TYPE_CODA: return qsTrc("inspector", "Coda")
            case MarkerTypes.TYPE_VARCODA: return qsTrc("inspector", "Varied coda")
            case MarkerTypes.TYPE_CODETTA: return qsTrc("inspector", "Codetta")
            case MarkerTypes.TYPE_FINE: return qsTrc("inspector", "Fine")
            case MarkerTypes.TYPE_TOCODA: return qsTrc("inspector", "To Coda")
            case MarkerTypes.TYPE_USER: return qsTrc("inspector", "Custom")
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Label")
        propertyItem: root.model ? root.model.label : null

        TextInputField {
            isIndeterminate: root.model ? root.model.label.isUndefined : false
            currentText: root.model ? root.model.label.value : ""
            enabled: root.model ? root.model.label.isEnabled : false

            onCurrentTextEdited: {
                if (!root.model) {
                    return
                }

                root.model.label.value = newTextValue
            }
        }
    }
}
