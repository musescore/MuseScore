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
import "pedals"

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

        NoteSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_NOTE) : null
        }

        FermataSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FERMATA) : null
        }

        GlissandoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_GLISSANDO) : null
        }

        TempoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEMPO) : null
        }

        BarlineSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
            staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null
        }

        SectionBreakSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_SECTIONBREAK) : null
        }

        MarkerSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MARKER) : null
        }

        JumpSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_JUMP) : null
        }

        KeySignatureSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_KEYSIGNATURE) : null
        }

        AccidentalSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ACCIDENTAL) : null
        }
		
        FretDiagramSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FRET_DIAGRAM) : null
        }

        PedalSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_PEDAL) : null
        }
    }
}
