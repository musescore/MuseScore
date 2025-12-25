/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
import QtQuick.Layouts 1.15

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

ColumnLayout {
    id: root

    property StyleItem lineStyle: null
    property StyleItem dashLineLength: null
    property StyleItem dashGapLength: null
    property StyleItem lineWidth: null

    width: parent.width
    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    RadioButtonSelectorWithReset {
        id: styleSection
        styleItem: root.lineStyle
        label: qsTrc("notation", "Line style:")

        model: [
            { iconCode: IconCode.LINE_NORMAL, value: 0, title: qsTrc("notation", "Normal") },
            { iconCode: IconCode.LINE_DASHED, value: 1, title: qsTrc("notation", "Dashed") },
            { iconCode: IconCode.LINE_DOTTED, value: 2, title: qsTrc("notation", "Dotted") },
        ]
    }

    StyleSpinboxWithReset {
        styleItem: root.lineWidth
        label: qsTrc("notation", "Line thickness:")
        suffix: qsTrc("global", "sp")
    }

    StyleSpinboxWithReset {
        styleItem: root.dashLineLength
        label: qsTrc("notation", "Dash (dashed line):")
        step: 0.1
    }

    StyleSpinboxWithReset {
        styleItem: root.dashGapLength
        label: qsTrc("notation", "Gap (dashed line):")
        step: 0.1
    }
}
