/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root

    property int navigationOrderStart: 0
    property int navigationOrderEnd: navigationPanel.order

    visible: false
    height: 50

    color: ui.theme.backgroundPrimaryColor

    signal closed()

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "SearchPopup"
        enabled: root.visible
        direction: NavigationPanel.Horizontal
        order: root.navigationOrderStart
        accessible.name: titleLabel.text
    }

    QtObject {
        id: privateProperties

        function show() {
            visible = true
            Qt.callLater(textInputField.navigation.requestActive)
        }

        function hide() {
            visible = false
            root.closed()
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
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        spacing: 8

        FlatButton {
            id: closeButton

            icon: IconCode.CLOSE_X_ROUNDED

            navigation.panel: root.navigationPanel
            navigation.order: 2
            navigation.accessible.name: qsTrc("notation", "Close Search")
            navigation.enabled: root.visible

            onClicked: {
                privateProperties.hide()
            }
        }
        
        StyledTextLabel {
            id: titleLabel
            anchors.verticalCenter: parent.verticalCenter 
            text: qsTrc("notation", "Find / Go to:")
        }

        TextInputField {
            id: textInputField

            width: 500

            navigation.panel: root.navigationPanel
            navigation.order: 1

            onTextChanged: function(newTextValue) {
                model.search(newTextValue)
            }

            onAccepted: {
                Qt.callLater(privateProperties.hide)
            }

            onEscaped: {
                Qt.callLater(privateProperties.hide)
            }
        }
    }
}
