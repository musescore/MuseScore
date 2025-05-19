/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

// Based on qrc:/kddockwidgets/private/quick/qml/TitleBarBase.qml, but adapted to our needs
MouseArea {
    objectName: "titleBarMouseArea"

    required property QtObject titleBarCpp

    /// Set to true if you're using a custom MouseEventRedirector in your code
    property bool hasCustomMouseEventRedirector: false

    onTitleBarCppChanged: {
        if (titleBarCpp && !hasCustomMouseEventRedirector) {
            titleBarCpp.redirectMouseEvents(this)
        }
    }
}
