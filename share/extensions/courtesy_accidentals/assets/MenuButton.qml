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

Item {
    height: toggle.height + 2 * style.regSpace
    width: parent.width //fix

    property alias title: toggle.title
    property alias isExpanded: toggle.isExpanded

    MU.ExpandableBlankSection {
        id: toggle
		isExpanded: false
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: style.regSpace
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: toggle.isExpanded = !toggle.isExpanded
    }
}
