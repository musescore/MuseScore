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
import QtQuick.Controls 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0 as MU

GroupBox {
	id: root
	opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
	
	property alias checked: checkBox.checked   // both values are currently needed separately, but may be combinable in future
	property alias value: cancelModeItem.value
	
	signal clicked
	signal setv(bool checked, int value)
	
	label: MU.CheckBox {
		id: checkBox
		enabled: root.enabled
		opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
		text: qsTr("Add cautionary accidentals to notes in any octave")
		checked: false
		onClicked: {checked = !checked; root.clicked()}
		signal setv(bool checked)
		onSetv: function(value) {checked = value}
	}
	
	CancelModeItem {
		id: cancelModeItem
		enabled: root.enabled && checkBox.checked
        onClicked: root.clicked()
	}
	
    onSetv: function(checked, value) {
        checkBox.setv(checked)
        cancelModeItem.setv(value)
    }
}
