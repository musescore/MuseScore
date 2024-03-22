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

import QtQuick 2.15
import MuseScore.UiComponents 1.0 as MU


MU.CheckBox {
    property bool key: false
    text: key ? qsTr("Add courtesy if note before key change is a grace note")
              : qsTr("Add courtesy if note with accidental is a grace note")
    signal changed
    onClicked: {checked = !checked; changed()}
    signal setv(bool checked)
    onSetv: function(value) {checked = value; changed()}
}
