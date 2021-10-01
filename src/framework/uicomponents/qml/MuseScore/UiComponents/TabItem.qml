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
import QtQuick.Controls 1.5
import MuseScore.Ui 1.0

Tab {
    id: root

    property alias navigation: navCtrl
    property bool checked: false

    signal navigationTriggered()

    property NavigationControl _nav: NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "TabItem"
        accessible.name: root.title
        accessible.role: MUAccessible.RadioButton
        accessible.checked: root.checked
        onTriggered: root.navigationTriggered()
    }

}
