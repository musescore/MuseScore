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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    id: root

    property int currentZoom: 0

    signal zoomInRequested()
    signal zoomOutRequested()

    spacing: 4

    FlatButton {
        icon: IconCode.ZOOM_IN

        normalStateColor: "transparent"

        onClicked: {
            root.zoomInRequested()
        }
    }

    FlatButton {
        icon: IconCode.ZOOM_OUT

        normalStateColor: "transparent"

        onClicked: {
            root.zoomOutRequested()
        }
    }

    StyledTextLabel {
        height: parent.height

        width: 60

        text: root.currentZoom + " %"
    }
}
