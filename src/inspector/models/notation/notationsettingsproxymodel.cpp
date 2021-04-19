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
#include "notationsettingsproxymodel.h"

#include "notes/notesettingsproxymodel.h"
#include "fermatas/fermatasettingsmodel.h"
#include "tempos/temposettingsmodel.h"
#include "glissandos/glissandosettingsmodel.h"
#include "barlines/barlinesettingsmodel.h"
#include "staffs/staffsettingsmodel.h"
#include "sectionbreaks/sectionbreaksettingsmodel.h"
#include "markers/markersettingsmodel.h"
#include "jumps/jumpsettingsmodel.h"
#include "keysignatures/keysignaturesettingsmodel.h"
#include "accidentals/accidentalsettingsmodel.h"
#include "fretdiagrams/fretdiagramsettingsmodel.h"
#include "pedals/pedalsettingsmodel.h"
#include "spacers/spacersettingsmodel.h"
#include "clefs/clefsettingsmodel.h"
#include "hairpins/hairpinsettingsmodel.h"
#include "crescendos/crescendosettingsmodel.h"
#include "stafftype/stafftypesettingsmodel.h"
#include "frames/textframesettingsmodel.h"
#include "frames/verticalframesettingsmodel.h"
#include "frames/horizontalframesettingsmodel.h"
#include "articulations/articulationsettingsmodel.h"
#include "ornaments/ornamentsettingsmodel.h"
#include "ambituses/ambitussettingsmodel.h"
#include "images/imagesettingsmodel.h"
#include "chordsymbols/chordsymbolsettingsmodel.h"
#include "brackets/bracketsettingsmodel.h"
#include "brackets/bracesettingsmodel.h"
#include "timesignatures/timesignaturesettingsmodel.h"
#include "mmrests/mmrestsettingsmodel.h"
#include "bends/bendsettingsmodel.h"
#include "tremolobars/tremolobarsettingsmodel.h"
#include "tremolos/tremolosettingsmodel.h"
#include "measurerepeats/measurerepeatsettingsmodel.h"

using namespace mu::inspector;

NotationSettingsProxyModel::NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorProxyModel(parent)
{
    setSectionType(InspectorSectionType::SECTION_NOTATION);
    setTitle(QStringLiteral("Notation"));

    addModel(new NoteSettingsProxyModel(this, repository));
    addModel(new FermataSettingsModel(this, repository));
    addModel(new TempoSettingsModel(this, repository));
    addModel(new GlissandoSettingsModel(this, repository));
    addModel(new BarlineSettingsModel(this, repository));
    addModel(new StaffSettingsModel(this, repository));
    addModel(new SectionBreakSettingsModel(this, repository));
    addModel(new MarkerSettingsModel(this, repository));
    addModel(new JumpSettingsModel(this, repository));
    addModel(new KeySignatureSettingsModel(this, repository));
    addModel(new AccidentalSettingsModel(this, repository));
    addModel(new FretDiagramSettingsModel(this, repository));
    addModel(new PedalSettingsModel(this, repository));
    addModel(new SpacerSettingsModel(this, repository));
    addModel(new ClefSettingsModel(this, repository));
    addModel(new HairpinSettingsModel(this, repository));
    addModel(new CrescendoSettingsModel(this, repository));
    addModel(new StaffTypeSettingsModel(this, repository));
    addModel(new TextFrameSettingsModel(this, repository));
    addModel(new VerticalFrameSettingsModel(this, repository));
    addModel(new HorizontalFrameSettingsModel(this, repository));
    addModel(new ArticulationSettingsModel(this, repository));
    addModel(new OrnamentSettingsModel(this, repository));
    addModel(new AmbitusSettingsModel(this, repository));
    addModel(new ImageSettingsModel(this, repository));
    addModel(new ChordSymbolSettingsModel(this, repository));
    addModel(new BracketSettingsModel(this, repository));
    addModel(new BraceSettingsModel(this, repository));
    addModel(new TimeSignatureSettingsModel(this, repository));
    addModel(new BendSettingsModel(this, repository));
    addModel(new TremoloBarSettingsModel(this, repository));
    addModel(new MMRestSettingsModel(this, repository));
    addModel(new TremoloSettingsModel(this, repository));
    addModel(new MeasureRepeatSettingsModel(this, repository));
}
