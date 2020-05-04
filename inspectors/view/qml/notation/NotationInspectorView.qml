import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3

import "../common"
import "notes"
import "fermatas"
import "tempos"
import "glissandos"
import "barlines"
import "sectionbreaks"
import "markers"
import "jumps"
import "keysignatures"
import "accidentals"

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

            icon: IconNameTypes.MUSIC_NOTES
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

            icon: IconNameTypes.FERMATA
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

            icon: IconNameTypes.GLISSANDO
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

            icon: IconNameTypes.METRONOME
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

        FlatButton {
            id: barlineSettingsButton

            icon: IconNameTypes.SECTION_BREAK
            iconPixelSize: 16
            text: qsTr("Barlines")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.barlineSettingsModel ? !root.model.barlineSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    barlinePopup.close()
                }
            }

            onClicked: {
                if (!barlinePopup.isOpened) {
                    barlinePopup.open()
                } else {
                    barlinePopup.close()
                }
            }

            BarlinePopup {
                id: barlinePopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                barlineSettingsModel: root.model ? root.model.barlineSettingsModel : null
                staffSettingsModel: root.model ? root.model.staffSettingsModel : null
            }
        }

        FlatButton {
            id: sectionBreakSettingsButton

            icon: IconNameTypes.SECTION_BREAK
            iconPixelSize: 16
            text: qsTr("Section break")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.sectionBreakSettingsModel ? !root.model.sectionBreakSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    sectionBreakPopup.close()
                }
            }

            onClicked: {
                if (!sectionBreakPopup.isOpened) {
                    sectionBreakPopup.open()
                } else {
                    sectionBreakPopup.close()
                }
            }

            SectionBreakPopup {
                id: sectionBreakPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.sectionBreakSettingsModel : null
            }
        }

        FlatButton {
            id: markerSettingsButton

            icon: IconNameTypes.MARKER
            iconPixelSize: 16
            text: qsTr("Markers")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.markerSettingsModel ? !root.model.markerSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    markerPopup.close()
                }
            }

            onClicked: {
                if (!markerPopup.isOpened) {
                    markerPopup.open()
                } else {
                    markerPopup.close()
                }
            }

            MarkerPopup {
                id: markerPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.markerSettingsModel : null
            }
        }

        FlatButton {
            id: jumpSettingsButton

            icon: IconNameTypes.JUMP
            iconPixelSize: 16
            text: qsTr("Jumps")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.jumpSettingsModel ? !root.model.jumpSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    jumpPopup.close()
                }
            }

            onClicked: {
                if (!jumpPopup.isOpened) {
                    jumpPopup.open()
                } else {
                    jumpPopup.close()
                }
            }

            JumpPopup {
                id: jumpPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.jumpSettingsModel : null
            }
        }

        FlatButton {
            id: keySignatureSettingsButton

            icon: IconNameTypes.KEY_SIGNATURE
            iconPixelSize: 16
            text: qsTr("Key signatures")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.keySignatureSettingsModel ? !root.model.keySignatureSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    keySignaturePopup.close()
                }
            }

            onClicked: {
                if (!keySignaturePopup.isOpened) {
                    keySignaturePopup.open()
                } else {
                    keySignaturePopup.close()
                }
            }

            KeySignaturePopup {
                id: keySignaturePopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.keySignatureSettingsModel : null
            }
        }

        FlatButton {
            id: accidentalSettingsButton

            icon: IconNameTypes.ACCIDENTAL_SHARP
            iconPixelSize: 16
            text: qsTr("Accidentals")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: root.model && root.model.accidentalSettingsModel ? !root.model.accidentalSettingsModel.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    accidentalPopup.close()
                }
            }

            onClicked: {
                if (!accidentalPopup.isOpened) {
                    accidentalPopup.open()
                } else {
                    accidentalPopup.close()
                }
            }

            AccidentalPopup {
                id: accidentalPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.accidentalSettingsModel : null
            }
        }
    }
}
