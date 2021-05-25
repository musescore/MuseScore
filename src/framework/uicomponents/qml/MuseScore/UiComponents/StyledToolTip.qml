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
import MuseScore.UiComponents 1.0

StyledPopupView {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property string shortcut: ""

    margins: 8

    contentWidth: Math.min(content.childrenRect.width, 300 - margins * 2)
    contentHeight: content.childrenRect.height

    Column {
        id: content

        anchors.fill: parent

        spacing: 4

        Row {
            spacing: 6

            StyledTextLabel {
                id: titleLabel

                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            StyledTextLabel {
                id: shortcutLabel

                text: "(" + root.shortcut + ")"
                horizontalAlignment: Text.AlignLeft
                opacity: 0.8

                visible: Boolean(root.shortcut)
            }
        }

        StyledTextLabel {
            id: descriptionLabel

            horizontalAlignment: Text.AlignLeft

            visible: Boolean(root.description)
        }
    }
}
