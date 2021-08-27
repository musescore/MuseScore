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
import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Diagnostics 1.0

Rectangle {

    id: root

    objectName: "DiagnosticEngravingElementsPanel"

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        elementsModel.init()
        elementsModel.reload()
    }

    DiagnosticEngravingElementsModel {
        id: elementsModel
    }

    Item {
        id: tools
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        height: childrenRect.height

        FlatButton {
            id: reloadBtn
            text: "Reload"
            onClicked: elementsModel.reload()
        }

        StyledTextLabel {
            anchors.top: reloadBtn.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: implicitHeight
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignLeft
            text: elementsModel.info
        }
    }

    ListView {
        anchors.top: tools.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true

        model: elementsModel

        delegate: Item {
            id: delgt
            width: parent.width
            height: 48

            property var item: itemData

            StyledTextLabel {
                anchors.fill: parent
                anchors.margins: 8
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideNone
                text: model.index + ": " + delgt.item.name
            }
        }
    }
}
