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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Column {
    id: root

    property alias title: titleInfo.info
    property alias subtitle: subtitleInfo.info
    property alias composer: composerInfo.info
    property alias lyricist: lyricistInfo.info
    property alias copyright: copyrightInfo.info

    spacing: 20

    Row {
        anchors.left: parent.left
        anchors.right: parent.right

        property real childWidth: (width / 2) - 15
        height: 60

        spacing: 20

        GeneralInfoItem {
            id: titleInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("project", "Title")
        }
        GeneralInfoItem {
            id: composerInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("project", "Composer")
        }
    }

    Row {
        anchors.left: parent.left
        anchors.right: parent.right

        property real childWidth: (width / 3) - 15
        height: 60

        spacing: 20

        GeneralInfoItem {
            id: subtitleInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("project", "Subtitle")
        }

        GeneralInfoItem {
            id: lyricistInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("project", "Lyricist")
        }

        GeneralInfoItem {
            id: copyrightInfo

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.childWidth

            title: qsTrc("project", "Copyright")
        }
    }
}
