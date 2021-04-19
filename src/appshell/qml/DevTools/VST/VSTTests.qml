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
import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Vst 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    VstPluginListModelExample {
        id: pluginListModel

        Component.onCompleted: {
            load()
        }
    }

    Column {
        anchors {
            top: parent.top
            topMargin: 24
            left: parent.left
            leftMargin: 24
        }

        ListView {

            width: 560
            height: 226

            model: pluginListModel

            spacing: 12

            delegate: RoundedRadioButton {
                width: parent.width

                checked: pluginListModel.selectedItemIndex === model.index
                text: nameRole

                onClicked: {
                    pluginListModel.selectedItemIndex = index
                }
            }
        }

        FlatButton {
            width: 560
            height: 80
            text: "Show editor"

            onClicked: {
                pluginListModel.showPluginEditor()
            }
        }
    }
}
