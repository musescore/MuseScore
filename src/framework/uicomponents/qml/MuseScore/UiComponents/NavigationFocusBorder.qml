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

Rectangle {
    id: root

    property NavigationControl navigationCtrl: null
    property bool drawOutsideParent: true

    anchors.fill: parent
    anchors.margins: drawOutsideParent ? -border.width : 0

    visible: navigationCtrl ? navigationCtrl.highlight : false

    color: "transparent"

    border.color: ui.theme.fontPrimaryColor
    border.width: ui.theme.navCtrlBorderWidth
    radius: Number(parent.radius) > 0
            ? (drawOutsideParent ? parent.radius + root.border.width : parent.radius)
            : 0
}
