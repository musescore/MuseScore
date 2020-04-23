import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3

import "../common"
import "notes"
import "fermatas"
import "tempos"
import "glissandos"

FocusableItem {
    id: root

    property QtObject model: undefined

    implicitHeight: grid.implicitHeight

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.rightMargin: 48

    GridLayout {
        id: grid

        width: parent.width

        columns: 2

        FlatButton {
            id: noteSettingsButton

            icon: "qrc:/resources/icons/note.svg"
            iconPixelSize: 16
            text: qsTr("Note settings")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.noteSettingsModel ? !root.model.noteSettingsModel.beamSettingsModel.isEmpty ||
                                                                  !root.model.noteSettingsModel.headSettingsModel.isEmpty ||
                                                                  !root.model.noteSettingsModel.stemSettingsModel.isEmpty ||
                                                                  !root.model.noteSettingsModel.hookSettingsModel.isEmpty
                                                                : false

            onVisibleChanged: {
                if (!visible) {
                    notePopup.close()
                }
            }

            onClicked: {
                if (!notePopup.isOpened) {
                    notePopup.open()
                } else {
                    notePopup.close()
                }
            }

            NotePopup {
                id: notePopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2

                width: root.width

                model: root.model ? root.model.noteSettingsModel : null
            }
        }

        FlatButton {
            id: fermataSettingsButton

            icon: "qrc:/resources/icons/fermata.svg"
            iconPixelSize: 16
            text: qsTr("Fermata")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.fermataSettingsModel ? !root.model.fermataSettingsModel.isEmpty : false
            onVisibleChanged: {
                if (!visible) {
                    fermataPopup.close()
                }
            }

            onClicked: {
                if (!fermataPopup.isOpened) {
                    fermataPopup.open()
                } else {
                    fermataPopup.close()
                }
            }

            FermataPopup {
                id: fermataPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2

                width: root.width

                model: root.model ? root.model.fermataSettingsModel : null
            }
        }

        FlatButton {
            id: glissandoSettingsButton

            icon: "qrc:/resources/icons/glissando.svg"
            iconPixelSize: 16
            text: qsTr("Glissando")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.glissandoSettingsModel ? !root.model.glissandoSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    glissandoPopup.close()
                }
            }

            onClicked: {
                if (!glissandoPopup.isOpened) {
                    glissandoPopup.open()
                } else {
                    glissandoPopup.close()
                }
            }

            GlissandoPopup {
                id: glissandoPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2

                width: root.width

                model: root.model ? root.model.glissandoSettingsModel : null
            }
        }

        FlatButton {
            id: tempoSettingsButton

            icon: "qrc:/resources/icons/tempo.svg"
            iconPixelSize: 16
            text: qsTr("Tempo")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.tempoSettingsModel ? !root.model.tempoSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    tempoPopup.close()
                }
            }

            onClicked: {
                if (!tempoPopup.isOpened) {
                    tempoPopup.open()
                } else {
                    tempoPopup.close()
                }
            }

            TempoPopup {
                id: tempoPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.tempoSettingsModel : null
            }
        }
    }
}
