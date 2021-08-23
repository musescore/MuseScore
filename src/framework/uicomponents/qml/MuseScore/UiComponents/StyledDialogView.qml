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
import QtGraphicalEffects 1.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

DialogView {
    id: root

    default property alias contentData: contentBody.data

    property alias background: contentBackground

    property alias width: rootContainer.width
    property alias height: rootContainer.height

    property int margins: 0

    property int contentWidth: 240
    property int contentHeight: contentBody.childrenRect.height

    property alias navigation: navSec
    property bool isDoActiveParentOnClose: true

    property NavigationSection navigationSection: NavigationSection {
        id: navSec
        name: root.objectName !== "" ? root.objectName : "StyledDialogView"
        type: NavigationSection.Exclusive
        enabled: root.isOpened
        order: 1

        onActiveChanged: {
            if (navSec.active) {
                rootContainer.forceActiveFocus()
            }
        }

        onNavigationEvent: {
            if (event.type === NavigationEvent.Escape) {
                root.close()
            }
        }
    }

    onClosed: {
        if (root.isDoActiveParentOnClose && root.navigationParentControl) {
            Qt.callLater(root.navigationParentControl.requestActive)
        }
    }

    contentItem: FocusScope {
        id: rootContainer
        width: contentBody.width + root.margins * 2
        height: contentBody.height + root.margins * 2

        implicitWidth: contentBody.implicitWidth + root.margins * 2
        implicitHeight: contentBody.implicitHeight + root.margins * 2

        Rectangle {
            id: contentBackground
            anchors.fill: parent
            color: ui.theme.backgroundPrimaryColor
        }

        Item {
            id: contentBody
            anchors.fill: parent
            anchors.margins: root.margins

            implicitWidth: root.contentWidth
            implicitHeight: root.contentHeight
        }
    }
}
