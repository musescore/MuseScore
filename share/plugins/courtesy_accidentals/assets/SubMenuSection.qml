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
import MuseScore.UiComponents 1.0 as MU
import MuseScore.Ui 1.0

Column {
	id: root
	property alias title: menuButton.title
	property alias isExpanded: menuButton.isExpanded
	default property alias content: column.children
	
	property alias source: dynamicImage.source
	
	width: parent.width
	spacing: 0
	anchors.margins: 0
	
	MenuButton {
		id: menuButton
	}
	StyledFrame {
		visible: root.isExpanded
		width: parent.width - 2 * root.parent.padding
		padding: style.regSpace
		bottomPadding: 0

		Column {
			spacing: 0
			width: parent.width
			//height: childrenRect.height
			
			DynamicImage {id: dynamicImage}
			
			Column {
				id: column
				spacing: style.regSpace
				width: parent.width
				//height: childrenRect.height
				padding: style.regSpace
			}
		}
	}
}
