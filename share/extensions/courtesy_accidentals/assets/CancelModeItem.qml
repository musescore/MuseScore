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
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0 as MU

Column {
    id: layout
    property int value: radioButton1.checked ? 1 : 2
    spacing: style.regSpace
    opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
    width: parent.width

    signal clicked
    signal setv(int value)

    MU.RoundedRadioButton {
        id: radioButton1
        implicitWidth: parent.width
        text: qsTr("Stop after note is cancelled in original octave")
        onClicked: layout.clicked()
        checked: true
    }
    MU.RoundedRadioButton {
        id: radioButton2
        implicitWidth: parent.width
        text: qsTr("Always cancel in all octaves")
        onClicked: layout.clicked()
    }
    onSetv: function (nvalue) {
        radioButton1.checked = nvalue == 1
        radioButton2.checked = nvalue == 2
        clicked()
    }
}
