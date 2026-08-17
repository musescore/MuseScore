/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 0

    readonly property bool shouldLoadPanel: width > 0 && height > 0

    function updateLoadedItem() {
        if (!videoPanelLoader.item) {
            return
        }

        videoPanelLoader.item.navigationSection = root.navigationSection
        videoPanelLoader.item.contentNavigationPanelOrderStart = root.contentNavigationPanelOrderStart
    }

    onNavigationSectionChanged: updateLoadedItem()
    onContentNavigationPanelOrderStartChanged: updateLoadedItem()

    Loader {
        id: videoPanelLoader

        anchors.fill: parent
        active: root.shouldLoadPanel
        asynchronous: true
        source: root.shouldLoadPanel ? "VideoPanel.qml" : ""

        onLoaded: {
            root.updateLoadedItem()
        }
    }

    Rectangle {
        anchors.fill: parent

        visible: root.shouldLoadPanel && videoPanelLoader.status === Loader.Error
        color: ui.theme.backgroundPrimaryColor

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.max(0, Math.min(parent.width - 48, 560))
            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font: ui.theme.headerBoldFont
                text: qsTrc("playback", "Video playback is unavailable")
            }

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                opacity: 0.75
                text: qsTrc("playback", "MuseScore could not load the Qt Multimedia module required by the Video panel. This build may be missing QtMultimedia in its package.")
            }
        }
    }
}
