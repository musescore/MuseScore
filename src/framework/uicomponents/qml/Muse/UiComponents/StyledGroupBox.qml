/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

GroupBox {
    id: root

    padding: 12
    spacing: 4

    background: Rectangle {
        y: root.label && root.label.visible
           ? root.topInset + root.implicitLabelHeight + root.spacing
           : root.topInset

        width: parent.width
        height: root.height - y

        color: ui.theme.backgroundPrimaryColor
        border.color: ui.theme.strokeColor
        radius: 3
    }

    label: Loader {
        sourceComponent: root.title ? titleLabel : emptyLabel
    }

    Component {
        id: titleLabel
        StyledTextLabel {
            x: root.leftInset
            y: root.topInset
            width: root.availableWidth
            text: root.title
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }
    }

    Component {
        id: emptyLabel
        Item {}
    }
}
