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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.4
import MuseScore.Ui 1.0

TabView {
    id: root

    readonly property int tabBarHeight: 24

    width: parent.width

    Rectangle {
        id: selectionHighlighting

        x: {
            if (root.currentIndex < 0) {
                return
            }

            root.currentIndex * (root.width / count)
        }

        height: 3
        width: parent.width / count

        color: ui.theme.accentColor

        radius: 2

        Behavior on x {
            NumberAnimation {
                duration: 150
            }
        }
    }

    style: TabViewStyle {
        frameOverlap: 1

        tab: Column {

            height: tabBarHeight
            width: styleData.availableWidth / count

            StyledTextLabel {
                id: titleLabel

                width: parent.width

                text: styleData.title
                font: ui.theme.bodyBoldFont
                opacity: styleData.selected ? ui.theme.buttonOpacityHit : ui.theme.buttonOpacityNormal
            }
        }

        frame: Rectangle {
            id: backgroundRect

            anchors.fill: parent

            color: ui.theme.backgroundPrimaryColor
        }
    }
}
