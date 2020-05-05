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
import "fretdiagrams"

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

            visible: notePopup.model ? !notePopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_NOTE) : null
            }
        }

        FlatButton {
            id: fermataSettingsButton

            icon: IconNameTypes.FERMATA
            iconPixelSize: 16
            text: qsTr("Fermata")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: fermataPopup.model ? !fermataPopup.model.isEmpty : false
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

                model: root.model ? root.model.modelByType(Inspector.TYPE_FERMATA) : null
            }
        }

        FlatButton {
            id: glissandoSettingsButton

            icon: IconNameTypes.GLISSANDO
            iconPixelSize: 16
            text: qsTr("Glissando")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: glissandoPopup.model ? !glissandoPopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_GLISSANDO) : null
            }
        }

        TempoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEMPO) : null
        }

        FlatButton {
            id: barlineSettingsButton

            icon: IconNameTypes.SECTION_BREAK
            iconPixelSize: 16
            text: qsTr("Barlines")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: barlinePopup.barlineSettingsModel ? !barlinePopup.barlineSettingsModel.isEmpty : false

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

                barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
                staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null
            }
        }

        FlatButton {
            id: sectionBreakSettingsButton

            icon: IconNameTypes.SECTION_BREAK
            iconPixelSize: 16
            text: qsTr("Section break")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: sectionBreakPopup.model ? !sectionBreakPopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_SECTIONBREAK) : null
            }
        }

        FlatButton {
            id: markerSettingsButton

            icon: IconNameTypes.MARKER
            iconPixelSize: 16
            text: qsTr("Markers")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: markerPopup.model ? !markerPopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_MARKER) : null
            }
        }

        FlatButton {
            id: jumpSettingsButton

            icon: IconNameTypes.JUMP
            iconPixelSize: 16
            text: qsTr("Jumps")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: jumpPopup.model ? !jumpPopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_JUMP) : null
            }
        }

        FlatButton {
            id: keySignatureSettingsButton

            icon: IconNameTypes.KEY_SIGNATURE
            iconPixelSize: 16
            text: qsTr("Key signatures")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: keySignaturePopup.model ? !keySignaturePopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_KEYSIGNATURE) : null
            }
        }

        FlatButton {
            id: accidentalSettingsButton

            icon: IconNameTypes.ACCIDENTAL_SHARP
            iconPixelSize: 16
            text: qsTr("Accidentals")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: accidentalPopup.model ? !accidentalPopup.model.isEmpty : false

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

                model: root.model ? root.model.modelByType(Inspector.TYPE_ACCIDENTAL) : null
            }
        }
		
        FlatButton {
            id: fretDiagramSettingsButton

            icon: IconNameTypes.FRET_DIAGRAM
            iconPixelSize: 16
            text: qsTr("Fretboard Diagrams")

            Layout.fillWidth: true
            Layout.minimumWidth: root.width / 2

            visible: fretDiagramPopup.model ? !fretDiagramPopup.model.isEmpty : false

            onVisibleChanged: {
                if (!visible) {
                    fretDiagramPopup.close()
                }
            }

            onClicked: {
                if (!fretDiagramPopup.isOpened) {
                    fretDiagramPopup.open()
                } else {
                    fretDiagramPopup.close()
                }
            }

            FretDiagramPopup {
                id: fretDiagramPopup

                x: mapToGlobal(grid.x, grid.y).x - mapToGlobal(parent.x, parent.y).x
                y: parent.height
                arrowX: parent.x + parent.width / 2
                width: root.width

                model: root.model ? root.model.modelByType(Inspector.TYPE_FRET_DIAGRAM) : null
            }
        }
    }
}
