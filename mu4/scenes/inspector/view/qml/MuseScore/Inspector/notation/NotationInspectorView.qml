import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

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
import "spacers"
import "clefs"
import "hairpins"
import "crescendos"
import "stafftype"
import "frames"
import "articulations"
import "ornaments"
import "ambituses"
import "images"
import "chordsymbols"
import "brackets"
import "timesignatures"
import "bends"
import "tremolobars"
import "mmrests"
import "tremolos"

InspectorSectionView {
    id: root

    implicitHeight: grid.implicitHeight

    function updateContentHeight(popupContentHeight) {
        root.contentHeight = implicitHeight + popupContentHeight
    }

    GridLayout {
        id: grid

        width: parent.width

        columns: 2
        columnSpacing: 4

        NoteSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_NOTE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        FermataSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FERMATA) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        GlissandoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_GLISSANDO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TempoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEMPO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BarlineSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            barlineSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_BARLINE) : null
            staffSettingsModel: root.model ? root.model.modelByType(Inspector.TYPE_STAFF) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        SectionBreakSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_SECTIONBREAK) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        MarkerSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MARKER) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        JumpSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_JUMP) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        KeySignatureSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_KEYSIGNATURE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        AccidentalSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ACCIDENTAL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }
		
        FretDiagramSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_FRET_DIAGRAM) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }
		
        PedalSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_PEDAL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        SpacerSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_SPACER) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ClefSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CLEF) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        HairpinSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_HAIRPIN) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        CrescendoSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CRESCENDO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        StaffTypeSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_STAFF_TYPE_CHANGES) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TextFrameSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TEXT_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        VerticalFrameSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_VERTICAL_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        HorizontalFrameSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_HORIZONTAL_FRAME) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ArticulationSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ARTICULATION) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        OrnamentSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_ORNAMENT) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        AmbitusSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_AMBITUS) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ImageSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_IMAGE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        ChordSymbolSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_CHORD_SYMBOL) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BracketSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BRACKET) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BraceSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BRACE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TimeSignatureSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TIME_SIGNATURE) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        MMRestSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_MMREST) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        BendSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_BEND) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TremoloBarSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TREMOLOBAR) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }

        TremoloSettings {
            popupPositionX: mapToGlobal(grid.x, grid.y).x - mapToGlobal(x, y).x
            popupAvailableWidth: root.width
            model: root.model ? root.model.modelByType(Inspector.TYPE_TREMOLO) : null
            onPopupContentHeightChanged: updateContentHeight(popupContentHeight)
        }
    }
}
