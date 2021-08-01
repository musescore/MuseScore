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
import QtQuick.Controls 2.0

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

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

    QtObject {
        id: prv

        function navigationCol() {
            return root.navigationColumn
        }

        function navigationRow(r) {
            //! NOTE 100 - to make unique, let's assume that there can be no more than 100 controls in one expandable block.
            return root.navigationRowOffset + r * 100
        }
    }

    Column {
        id: contentColumn

        width: parent.width
        height: childrenRect.height

        spacing: 4

        NoteExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: prv.navigationCol()
            navigation.row: prv.navigationRow(1)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_NOTE) : null
        }

        ArpeggioExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: prv.navigationCol()
            navigation.row: prv.navigationRow(2)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_ARPEGGIO) : null
        }

        FermataExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: prv.navigationCol()
            navigation.row: prv.navigationRow(3)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_FERMATA) : null
        }

        PausesExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: prv.navigationCol()
            navigation.row: prv.navigationRow(4)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_BREATH) : null
        }

        GlissandoExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: prv.navigationCol()
            navigation.row: prv.navigationRow(5)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_GLISSANDO) : null
        }

        ChordSymbolsExpandableBlank {
            navigation.panel: root.navigationPanel
            navigation.column: root.navigationCol()
            navigation.row: root.navigationRow(6)

            model: proxyModel ? proxyModel.modelByType(Inspector.TYPE_CHORD_SYMBOL) : null
        }
    }
}

