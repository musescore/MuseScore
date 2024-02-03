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

Item {
    id: root

    height: bracketBox.height //childrenRect.height
    //requires indicated width

    property alias checked: checkBox.checked
    property alias currentValue: bracketBox.currentValue

    signal clicked
    signal setv(bool checked, int value)

    MU.CheckBox {
        id: checkBox
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Add Courtesy Accidentals")
        onClicked: {checked = !checked; root.clicked()}
        signal setv(bool checked)
        onSetv: function(value) {checked = value; root.clicked()}
    }
    BracketBox {
        id: bracketBox
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        enabled: checkBox.checked
        onActivated: root.clicked()
    }
    onSetv: function(checked, value) {
        checkBox.setv(checked)
        bracketBox.setv(value)
    }
}
