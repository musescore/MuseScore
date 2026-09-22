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
import Muse.UiComponents

ButtonBox {
    id: root

    property bool convertEnabled: true

    signal cancelRequested()
    signal backRequested()
    signal convertRequested()

    FlatButton {
        text: qsTrc("global", "Cancel")

        buttonRole: ButtonBoxModel.CustomRole
        buttonId: ButtonBoxModel.CustomButton + 1
        isLeftSide: true

        onClicked: {
            root.cancelRequested()
        }
    }

    FlatButton {
        text: qsTrc("global", "Back")

        buttonRole: ButtonBoxModel.BackRole
        buttonId: ButtonBoxModel.CustomButton + 2

        onClicked: {
            root.backRequested()
        }
    }

    FlatButton {
        text: qsTrc("project/convert", "Convert")

        buttonRole: ButtonBoxModel.ApplyRole
        buttonId: ButtonBoxModel.CustomButton + 3

        accentButton: true
        enabled: root.convertEnabled

        onClicked: {
            root.convertRequested()
        }
    }
}
