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
	spacing: style.regSpace
	anchors.topMargin: style.minSpace
	opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled
	
	property int value: radioButton0.checked ? 0 : (radioButton1.checked ? 1 : 2)
	
	signal clicked
	signal setv(int nvalue)
	
	label: StyledLabel {text: qsTr("Add cautionary accidentals to:")}
	
	Column {
		spacing: style.regSpace
		//leftPadding: style.regSpace
		width: parent.width
		
		MU.RoundedRadioButton {
			id: radioButton0
			implicitWidth: parent.width
			text: qsTr("All notes after the note with accidental")
			onClicked: root.clicked()
			checked: true
		}
		MU.RoundedRadioButton {
			id: radioButton1
			implicitWidth: parent.width
			text: qsTr("Notes on the same beat as the note with accidental")
			onClicked: root.clicked()
		}
		MU.RoundedRadioButton {
			id: radioButton2
			implicitWidth: parent.width
			text: qsTr("Notes played any time throughout the note with accidental")
			onClicked: root.clicked()
		}
	}
	onSetv: function (nvalue) {
		radioButton0.checked = nvalue == 0
		radioButton1.checked = nvalue == 1
		radioButton2.checked = nvalue == 2
		clicked()
	}
}
