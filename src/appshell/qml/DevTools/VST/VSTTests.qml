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
import MuseScore.VST 1.0

Rectangle {

    color: "#0fb9b1"

    VSTDevTools {
        id: devtools
    }

    Column {
        id: column
        padding: 5
        anchors.fill: parent
        spacing: 2

        Row {
            id: selector
            height: 75
            anchors.right: parent.right
            anchors.left: parent.left

            StyledComboBox {
                id: pluginSelector
                width: parent.width / 2

                valueRoleName: "uid"
                textRoleName: "name"

                model: devtools.plugins
            }

            FlatButton {
                text: "Add instance"
                onClicked: devtools.addInstance(pluginSelector.currentIndex)
            }
        }

        Row {
            id: instances
            anchors.top: selector.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Column {
                anchors.fill: parent

                Repeater {
                    anchors.fill: parent
                    model: devtools.instances
                    Row {
                        height: 50
                        anchors.left: parent.left
                        anchors.right: parent.right

                        StyledTextLabel {
                            id: element
                            text: modelData.name;
                            width: parent.width - 240
                        }

                        FlatButton {
                            width: 120
                            text: "edit"
                            onClicked: devtools.showEditor(index)

                        }

                        FlatButton {
                            width: 120
                            text: "play"
                            onClicked: devtools.play(index)
                        }

                    }
                }
            }
        }
    }
}
