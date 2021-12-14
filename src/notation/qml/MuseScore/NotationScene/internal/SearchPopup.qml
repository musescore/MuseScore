/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.12

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    visible: false
    height: 50

    color: ui.theme.backgroundPrimaryColor

    QtObject {
        id: privateProperties

        function show() {
            visible = true
            Qt.callLater(textInputField.forceActiveFocus)
        }

        function hide() {
            visible = false
        }
    }

    SearchPopupModel {
        id: model

        onShowPopupRequested: {
            privateProperties.show()
        }
    }

    Component.onCompleted: {
        model.load()
    }

    Row {
        anchors.verticalCenter: parent.verticalCenter

        spacing: 8

        FlatButton {
            id: closeButton

            icon: IconCode.CLOSE_X_ROUNDED

            onClicked: {
                privateProperties.hide()
            }
        }
        
        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter 
            text: qsTrc("notation", "Find / Go to:")
        }

        TextInputField {
            id: textInputField

            width: 500

            onCurrentTextEdited: function(newTextValue) {
                model.search(newTextValue)
            }
        }
    }
}
