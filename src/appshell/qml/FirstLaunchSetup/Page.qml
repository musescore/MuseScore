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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0

Item {
    id: root

    property alias title: titleLabel.text
    property alias explanation: explanationLabel.text

    property NavigationSection navigationSection: null

    default property alias content: contentItem.data

    property real titleContentSpacing: 24

    property string extraButtonTitle: ""
    signal extraButtonClicked()

    anchors.fill: parent

    Column {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: childrenRect.height
        spacing: 6

        StyledTextLabel {
            id: titleLabel
            anchors.horizontalCenter: parent.horizontalCenter
            font: ui.theme.largeBodyBoldFont
        }

        StyledTextLabel {
            id: explanationLabel
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
        }
    }

    Item {
        id: contentItem

        anchors.top: header.bottom
        anchors.topMargin: root.titleContentSpacing
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
