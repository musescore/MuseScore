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
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0

FlatButton {
    id: root

    default property StyledPopupView popup

    property int popupPositionX: 0
    property int popupPositionY: height
    property int popupAvailableWidth: 0
    readonly property int popupContentHeight: popup.isOpened ? popup.contentHeight + popup.arrowHeight : 0

    Layout.fillWidth: true
    Layout.minimumWidth: (popupAvailableWidth - 4) / 2

    onVisibleChanged: {
        if (!visible) {
            popup.close()
        }
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }
}
