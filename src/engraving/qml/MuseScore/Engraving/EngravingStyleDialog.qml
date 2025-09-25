/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
import Muse.Ui
import Muse.UiComponents

StyledDialogView {
    id: root

    title: "Diagnostic: Engraving style options list"

    contentHeight: 900
    contentWidth: 500
    resizable: true

    //! NOTE It is necessary that it can be determined that this is an object for diagnostics
    contentItem.objectName: panel.objectName

    margins: 12

    EngravingStylePanel {
        id: panel
        anchors.fill: parent
    }
}
