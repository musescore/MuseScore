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

    width: parent.width
    spacing: 0

    MenuButton {
        id: menuButton
    }
    Column {
        id: column
        spacing: 0
        padding: style.regSpace
        topPadding: 0
        bottomPadding: 0
        width: parent.width
        visible: root.isExpanded
    }
}
