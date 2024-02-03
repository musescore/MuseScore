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

Rectangle {
    property int realWidth: 400
    property int cornerRadius: style.regSpace
    property var source: "logo.png" // something
    radius: cornerRadius
    border.color: ui.theme.accentColor
    border.width: 2
    //Layout.preferredWidth: realWidth
	width: parent.width
    height: image.height + cornerRadius
    //anchors.horizontalCenter: parent.horizontalCenter
    Image {
        id: image
        source: parent.source
        width: parent.width - cornerRadius
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit // ensure it fits
        mipmap: true // smoothing
    }
}
