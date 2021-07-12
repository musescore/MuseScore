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
#include "inspectormodelcreator.h"

#include "notation/notes/notesettingsproxymodel.h"
#include "notation/notes/noteheads/noteheadsettingsmodel.h"
#include "notation/notes/beams/beamsettingsmodel.h"
#include "notation/notes/hooks/hooksettingsmodel.h"
#include "notation/notes/stems/stemsettingsmodel.h"
#include "notation/fermatas/fermatasettingsmodel.h"
#include "notation/tempos/temposettingsmodel.h"
#include "notation/glissandos/glissandosettingsmodel.h"
#include "notation/barlines/barlinesettingsproxymodel.h"
#include "notation/staffs/staffsettingsmodel.h"
#include "notation/sectionbreaks/sectionbreaksettingsmodel.h"
#include "notation/markers/markersettingsmodel.h"
#include "notation/jumps/jumpsettingsmodel.h"
#include "notation/keysignatures/keysignaturesettingsmodel.h"
#include "notation/accidentals/accidentalsettingsmodel.h"
#include "notation/fretdiagrams/fretdiagramsettingsmodel.h"
#include "notation/pedals/pedalsettingsmodel.h"
#include "notation/spacers/spacersettingsmodel.h"
#include "notation/clefs/clefsettingsmodel.h"
#include "notation/hairpins/hairpinsettingsmodel.h"
#include "notation/crescendos/crescendosettingsmodel.h"
#include "notation/stafftype/stafftypesettingsmodel.h"
#include "notation/frames/textframesettingsmodel.h"
#include "notation/frames/verticalframesettingsmodel.h"
#include "notation/frames/horizontalframesettingsmodel.h"
#include "notation/articulations/articulationsettingsmodel.h"
#include "notation/ornaments/ornamentsettingsmodel.h"
#include "notation/ambituses/ambitussettingsmodel.h"
#include "notation/images/imagesettingsmodel.h"
#include "notation/chordsymbols/chordsymbolsettingsmodel.h"
#include "notation/brackets/bracketsettingsmodel.h"
#include "notation/brackets/bracesettingsmodel.h"
#include "notation/timesignatures/timesignaturesettingsmodel.h"
#include "notation/mmrests/mmrestsettingsmodel.h"
#include "notation/bends/bendsettingsmodel.h"
#include "notation/tremolobars/tremolobarsettingsmodel.h"
#include "notation/tremolos/tremolosettingsmodel.h"
#include "notation/measurerepeats/measurerepeatsettingsmodel.h"

using namespace mu::inspector;

AbstractInspectorModel* InspectorModelCreator::newInspectorModel(AbstractInspectorModel::InspectorModelType modelType, QObject* parent,
                                                                 IElementRepositoryService* repository) const
{
    switch (modelType) {
    case AbstractInspectorModel::InspectorModelType::TYPE_NOTE:
        return new NoteSettingsProxyModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_BEAM:
        return new NoteSettingsProxyModel(parent, repository, AbstractInspectorModel::InspectorModelType::TYPE_BEAM);
    case AbstractInspectorModel::InspectorModelType::TYPE_NOTEHEAD:
        return new NoteSettingsProxyModel(parent, repository, AbstractInspectorModel::InspectorModelType::TYPE_NOTEHEAD);
    case AbstractInspectorModel::InspectorModelType::TYPE_STEM:
        return new NoteSettingsProxyModel(parent, repository, AbstractInspectorModel::InspectorModelType::TYPE_STEM);
    case AbstractInspectorModel::InspectorModelType::TYPE_HOOK:
        return new NoteSettingsProxyModel(parent, repository, AbstractInspectorModel::InspectorModelType::TYPE_HOOK);
    case AbstractInspectorModel::InspectorModelType::TYPE_FERMATA:
        return new FermataSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_TEMPO:
        return new TempoSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_GLISSANDO:
        return new GlissandoSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_BARLINE:
        return new BarlineSettingsProxyModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_STAFF:
        return new StaffSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_MARKER:
        return new MarkerSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_SECTIONBREAK:
        return new SectionBreakSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_JUMP:
        return new JumpSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_KEYSIGNATURE:
        return new KeySignatureSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_ACCIDENTAL:
        return new AccidentalSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_FRET_DIAGRAM:
        return new FretDiagramSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_PEDAL:
        return new PedalSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_SPACER:
        return new SpacerSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_CLEF:
        return new ClefSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_HAIRPIN:
        return new HairpinSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_CRESCENDO:
        return new CrescendoSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_STAFF_TYPE_CHANGES:
        return new StaffTypeSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_TEXT_FRAME:
        return new TextFrameSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_VERTICAL_FRAME:
        return new VerticalFrameSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_HORIZONTAL_FRAME:
        return new HorizontalFrameSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_ARTICULATION:
        return new ArticulationSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_ORNAMENT:
        return new OrnamentSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_AMBITUS:
        return new AmbitusSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_IMAGE:
        return new ImageSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_CHORD_SYMBOL:
        return new ChordSymbolSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_BRACKET:
        return new BracketSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_BRACE:
        return new BraceSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_TIME_SIGNATURE:
        return new TimeSignatureSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_MMREST:
        return new MMRestSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_BEND:
        return new BendSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_TREMOLOBAR:
        return new TremoloBarSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_TREMOLO:
        return new TremoloSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_MEASURE_REPEAT:
        return new MeasureRepeatSettingsModel(parent, repository);
    case AbstractInspectorModel::InspectorModelType::TYPE_UNDEFINED:
        break;
    }

    return nullptr;
}
