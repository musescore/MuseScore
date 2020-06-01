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

QString InspectorsSetup::moduleName() const
{
    return QStringLiteral("inspectors");
}

void InspectorsSetup::registerExports()
{
}

void InspectorsSetup::registerResources()
{
    Q_INIT_RESOURCE(inspectors_resources);
}

#include "utils/doubleinputvalidator.h"
#include "utils/intinputvalidator.h"
#include "models/abstractinspectormodel.h"
#include "types/stemtypes.h"
#include "types/noteheadtypes.h"
#include "types/beamtypes.h"
#include "types/hairpintypes.h"
#include "types/ornamenttypes.h"
#include "types/dynamictypes.h"
#include "types/glissandotypes.h"
#include "types/fermatatypes.h"
#include "types/iconnames.h"
#include "types/barlinetypes.h"
#include "types/markertypes.h"
#include "types/keysignaturetypes.h"
#include "types/accidentaltypes.h"
#include "types/fretdiagramtypes.h"
#include "types/pedaltypes.h"
#include "types/texttypes.h"
#include "types/crescendotypes.h"
#include "types/articulationtypes.h"

void InspectorsSetup::registerQmlTypes()
{
    qmlRegisterType<DoubleInputValidator>("MuseScore.Inspectors", 3, 3, "DoubleInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.Inspectors", 3, 3, "IntInputValidator");
    qmlRegisterUncreatableType<AbstractInspectorModel>("MuseScore.Inspectors", 3, 3, "Inspector", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<StemTypes>("MuseScore.Inspectors", 3, 3, "Stem", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<NoteHeadTypes>("MuseScore.Inspectors", 3, 3, "NoteHead", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BeamTypes>("MuseScore.Inspectors", 3, 3, "Beam", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<HairpinTypes>("MuseScore.Inspectors", 3, 3, "Hairpin", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<OrnamentTypes>("MuseScore.Inspectors", 3, 3, "OrnamentTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DynamicTypes>("MuseScore.Inspectors", 3, 3, "Dynamic", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<GlissandoTypes>("MuseScore.Inspectors", 3, 3, "Glissando", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FermataTypes>("MuseScore.Inspectors", 3, 3, "FermataTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<IconNameTypes>("MuseScore.Inspectors", 3, 3, "IconNameTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BarlineTypes>("MuseScore.Inspectors", 3, 3, "BarlineTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<MarkerTypes>("MuseScore.Inspectors", 3, 3, "MarkerTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<KeySignatureTypes>("MuseScore.Inspectors", 3, 3, "KeySignatureTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AccidentalTypes>("MuseScore.Inspectors", 3, 3, "AccidentalTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FretDiagramTypes>("MuseScore.Inspectors", 3, 3, "FretDiagramTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<PedalTypes>("MuseScore.Inspectors", 3, 3, "PedalTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TextTypes>("MuseScore.Inspectors", 3, 3, "TextTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<CrescendoTypes>("MuseScore.Inspectors", 3, 3, "CrescendoTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ArticulationTypes>("MuseScore.Inspectors", 3, 3, "ArticulationTypes", "Not creatable as it is an enum type");
}
