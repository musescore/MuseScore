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
import "../../common"
import "internal"

Item {
    id: root

    property QtObject proxyModel: null

    property NavigationPanel navigationPanel: null
    property int navigationColumn: 1
    property int navigationRowOffset: 1

    implicitHeight: contentColumn.height
    width: parent.width

    function navigationCol() {
        return root.navigationColumn
    }

    function navigationRow(r) {
        //! NOTE 100 - to make unique, let's assume that there can be no more than 100 controls in one expandable block.
        return root.navigationRowOffset + r * 100
    }

    Column {
        id: contentColumn

        width: parent.width

        spacing: 4

        DynamicsExpandableBlank {
            id: dynamicsExpandableBlank
            navigation.panel: root.navigationPanel
            navigation.column: root.navigationCol()
            navigation.row: root.navigationRow(1)
            model: proxyModel ? proxyModel.dynamicPlaybackModel : null
        }

        HairpinsExpandableBlank {
            id: hairpinsExpandableBlank
            navigation.panel: root.navigationPanel
            navigation.column: root.navigationCol()
            navigation.row: root.navigationRow(2)
            model: proxyModel ? proxyModel.hairpinPlaybackModel : null
        }
    }
}

