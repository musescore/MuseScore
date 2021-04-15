/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.2
import MuseScore.UiComponents 1.0
import "../../common"
import "internal"

StyledPopup {
    id: root

    property QtObject model: undefined

    height: contentColumn.implicitHeight + topPadding + bottomPadding
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 16

        VerticalGapsSection {
            gapAbove: model ? model.gapAbove : null
            gapBelow: model ? model.gapBelow : null
        }

        SeparatorLine { anchors.margins: -10 }

        HorizontalMarginsSection {
            frameLeftMargin: model ? model.frameLeftMargin : null
            frameRightMargin: model ? model.frameRightMargin : null
        }

        VerticalMarginsSection {
            frameTopMargin: model ? model.frameTopMargin : null
            frameBottomMargin: model ? model.frameBottomMargin : null
        }
    }
}
