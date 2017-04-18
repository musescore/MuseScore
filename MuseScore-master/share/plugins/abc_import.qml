//=============================================================================
//
//  ABC Import Plugin
//  Based on ABC Import by Nicolas Froment (lasconic)
//  Copyright (2013) Stephane Groleau (vgstef)
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

import QtQuick 2.1
import QtQuick.Dialogs 1.0
import QtQuick.Controls 1.0
import MuseScore 1.0
import FileIO 1.0

MuseScore {
    menuPath: "Plugins.ABC Import"
    version: "2.0"
    description: qsTr("This plugin imports ABC text from a file or the clipboard. Internet connection is required.")
    requiresScore: false
    pluginType: "dialog"

    id:window
    width:  800; height: 500;
    onRun: {}

    FileIO {
        id: myFileAbc
        onError: console.log(msg + "  Filename = " + myFileAbc.source)
        }

    FileIO {
        id: myFile
        source: tempPath() + "/my_file.xml"
        onError: console.log(msg)
        }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        onAccepted: {
            var filename = fileDialog.fileUrl
            //console.log("You chose: " + filename)

            if(filename){
                myFileAbc.source = filename
                //read abc file and put it in the TextArea
                abcText.text = myFileAbc.read()
                }
            }
        }

    Label {
        id: textLabel
        wrapMode: Text.WordWrap
        text: qsTr("Paste your ABC tune here (or click button to load a file)\nYou need to be connected to internet for this plugin to work")
        font.pointSize:12
        anchors.left: window.left
        anchors.top: window.top
        anchors.leftMargin: 10
        anchors.topMargin: 10
        }

    // Where people can paste their ABC tune or where an ABC file is put when opened
    TextArea {
        id:abcText
        anchors.top: textLabel.bottom
        anchors.left: window.left
        anchors.right: window.right
        anchors.bottom: buttonOpenFile.top
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        width:parent.width
        height:400
        wrapMode: TextEdit.WrapAnywhere
        textFormat: TextEdit.PlainText
        }

    Button {
        id : buttonOpenFile
        text: qsTr("Open file")
        anchors.bottom: window.bottom
        anchors.left: abcText.left
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.leftMargin: 10
        onClicked: {
            fileDialog.open();
            }
        }

    Button {
        id : buttonConvert
        text: qsTr("Import")
        anchors.bottom: window.bottom
        anchors.right: abcText.right
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.rightMargin: 10
        onClicked: {
            var content = "content=" + encodeURIComponent(abcText.text)
            //console.log("content : " + content)

            var request = new XMLHttpRequest()
            request.onreadystatechange = function() {
                if (request.readyState == XMLHttpRequest.DONE) {
                    var response = request.responseText
                    //console.log("responseText : " + response)
                    myFile.write(response)
                    readScore(myFile.source)
                        Qt.quit()
                    }
                }
            request.open("POST", "http://abc2xml.appspot.com/abcrenderer", true)
            request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
            request.send(content)
            }
        }

    Button {
        id : buttonCancel
        text: qsTr("Cancel")
        anchors.bottom: window.bottom
        anchors.right: buttonConvert.left
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        onClicked: {
                Qt.quit();
            }
        }
    }
