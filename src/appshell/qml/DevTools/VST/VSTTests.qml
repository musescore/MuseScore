//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
