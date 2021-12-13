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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledDialogView {
    id: root

    property alias type: content.type

    property alias title: content.title
    property alias text: content.text
    property alias textFormat: content.textFormat

    property alias withIcon: content.withIcon
    property alias iconCode: content.iconCode

    property alias withShowAgain: content.withShowAgain

    property var buttons: [ { "buttonId": 1, "title": qsTrc("global", "OK") } ]
    property alias defaultButtonId: content.defaultButtonId

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight

    margins: 16

    onOpened: {
        content.onOpened()
    }

    StandardDialogPanel {
        id: content
        anchors.fill: parent

        navigation.section: root.navigationSection
        buttons: root.buttons

        onClicked: function(buttonId, showAgain) {
            root.ret = { "errcode": 0, "value": { "buttonId": buttonId, "showAgain": showAgain}}
            root.hide()
        }
    }
}
