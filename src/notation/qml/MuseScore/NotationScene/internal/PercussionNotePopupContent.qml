/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Row {
    id: root

    height: 24
    spacing: 2

    PercussionNotePopupContentModel {
        id: contentModel

        Component.onCompleted: {
            contentModel.init()
        }
    }

    Row {
        id: buttonRow

        visible: contentModel.shouldShowButtons

        height: root.height
        spacing: 2

        FlatButton {
            id: prevButton

            height: buttonRow.height
            width: 16

            icon: IconCode.CHEVRON_LEFT

            onClicked: {
                contentModel.prevDrumNote()
            }
        }

        FlatButton {
            id: nextButton

            height: buttonRow.height
            width: 16

            icon: IconCode.CHEVRON_RIGHT

            onClicked: {
                contentModel.nextDrumNote()
            }
        }
    }

    Row {
        id: labelRow

        height: root.height
        spacing: 4

        rightPadding: 4
        leftPadding: 4

        StyledTextLabel {
            id: percussionNoteName

            height: labelRow.height
            verticalAlignment: Text.AlignVCenter

            text: contentModel.percussionNoteName
            font: ui.theme.bodyBoldFont
        }

        StyledTextLabel {
            id: keyboardShortcut

            height: labelRow.height
            verticalAlignment: Text.AlignVCenter

            text: contentModel.keyboardShortcut
            font: ui.theme.bodyFont
            opacity: 0.8
        }
    }
}

