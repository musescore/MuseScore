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
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Item {
    id: root

    property QtObject proxyModel: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 4

        DynamicsExpandableBlank {
            id: dynamicSection
            navigation.panel: root.navigationPanel
            navigation.row: root.navigationRowStart

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_DYNAMIC) : null
        }

        HairpinsExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.row: dynamicSection.navigationRowEnd + 1

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_HAIRPIN) : null
        }
    }
}

