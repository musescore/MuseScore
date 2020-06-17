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

#include "inspectorssetup.h"

std::string InspectorsSetup::moduleName() const
{
    return "inspectors";
}

void InspectorsSetup::registerExports()
{
}

void InspectorsSetup::registerResources()
{
    Q_INIT_RESOURCE(inspectors_resources);
}

#include "models/abstractinspectormodel.h"

#include "view/widgets/fretcanvas.h"
#include "view/widgets/gridcanvas.h"

#include "types/stemtypes.h"
#include "types/noteheadtypes.h"
#include "types/beamtypes.h"
#include "types/hairpintypes.h"
#include "types/ornamenttypes.h"
#include "types/dynamictypes.h"
#include "types/glissandotypes.h"
#include "types/fermatatypes.h"
#include "types/barlinetypes.h"
#include "types/markertypes.h"
#include "types/keysignaturetypes.h"
#include "types/accidentaltypes.h"
#include "types/fretdiagramtypes.h"
#include "types/pedaltypes.h"
#include "types/texttypes.h"
#include "types/crescendotypes.h"
#include "types/articulationtypes.h"
#include "types/ambitustypes.h"
#include "types/chordsymboltypes.h"
#include "types/scoreappearancetypes.h"
#include "types/bendtypes.h"
#include "types/tremolobartypes.h"

void InspectorsSetup::registerUiTypes()
{
    qmlRegisterUncreatableType<AbstractInspectorModel>("MuseScore.Inspectors", 3, 3, "Inspector",
                                                       "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DirectionTypes>("MuseScore.Inspectors", 3, 3, "DirectionTypes",
                                               "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<NoteHeadTypes>("MuseScore.Inspectors", 3, 3, "NoteHead",
                                              "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BeamTypes>("MuseScore.Inspectors", 3, 3, "Beam", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<HairpinTypes>("MuseScore.Inspectors", 3, 3, "Hairpin",
                                             "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<OrnamentTypes>("MuseScore.Inspectors", 3, 3, "OrnamentTypes",
                                              "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DynamicTypes>("MuseScore.Inspectors", 3, 3, "Dynamic",
                                             "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<GlissandoTypes>("MuseScore.Inspectors", 3, 3, "Glissando",
                                               "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FermataTypes>("MuseScore.Inspectors", 3, 3, "FermataTypes",
                                             "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BarlineTypes>("MuseScore.Inspectors", 3, 3, "BarlineTypes",
                                             "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<MarkerTypes>("MuseScore.Inspectors", 3, 3, "MarkerTypes",
                                            "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<KeySignatureTypes>("MuseScore.Inspectors", 3, 3, "KeySignatureTypes",
                                                  "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AccidentalTypes>("MuseScore.Inspectors", 3, 3, "AccidentalTypes",
                                                "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FretDiagramTypes>("MuseScore.Inspectors", 3, 3, "FretDiagramTypes",
                                                 "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<PedalTypes>("MuseScore.Inspectors", 3, 3, "PedalTypes",
                                           "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TextTypes>("MuseScore.Inspectors", 3, 3, "TextTypes",
                                          "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<CrescendoTypes>("MuseScore.Inspectors", 3, 3, "CrescendoTypes",
                                               "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ArticulationTypes>("MuseScore.Inspectors", 3, 3, "ArticulationTypes",
                                                  "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AmbitusTypes>("MuseScore.Inspectors", 3, 3, "AmbitusTypes",
                                             "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ChordSymbolTypes>("MuseScore.Inspectors", 3, 3, "ChordSymbolTypes",
                                                 "Not creatable as it is an enum type");
    qmlRegisterType<Ms::FretCanvas>("MuseScore.Inspectors", 3, 3, "FretCanvas");
    qmlRegisterUncreatableType<ScoreAppearanceTypes>("MuseScore.Inspectors", 3, 3, "ScoreAppearanceTypes",
                                                     "Not creatable as it is an enum type");
    qmlRegisterType<Ms::GridCanvas>("MuseScore.Inspectors", 3, 3, "GridCanvas");
    qmlRegisterUncreatableType<BendTypes>("MuseScore.Inspectors", 3, 3, "BendTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TremoloBarTypes>("MuseScore.Inspectors", 3, 3, "TremoloBarTypes", "Not creatable as it is an enum type");
}
