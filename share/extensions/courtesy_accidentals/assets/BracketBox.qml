//==============================================
//  Cautionary Accidentals v4.0
//  https://github.com/XiaoMigros/Cautionary-Accidentals
//  Copyright (C)2023 XiaoMigros
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//==============================================

import QtQuick 2.9
import QtQuick.Layouts 1.2
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0 as MU

Row {
    id: layout
    spacing: 6
    signal activated(int index, var value)
    property var currentValue: control.currentValue
    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
    signal setv(int index)

    MU.StyledTextLabel {
        id: label
        text: qsTr("Brackets:")
        anchors.verticalCenter: control.verticalCenter
    }

    MU.StyledDropdown {
        id: control
        textRole: "text"
        valueRole: "fact"
        currentIndex: 0
        model: [
            {text: qsTr("None"),        fact: 0},
            {text: qsTr("Parentheses"), fact: 1},
            {text: qsTr("Brackets"),    fact: 2}
        ]
        onActivated: function(index, value) {
            currentIndex = index
            layout.activated(index, value)
        }
    }
    onSetv: function(index) {
        control.currentIndex = index
        control.activated(index, Utils.getItemValue(control.model, index, control.valueRole, ""))
    }
}
