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
#include "types/direction.h"
#include "types/noteheadtypes.h"
#include "types/beamtypes.h"
#include "types/hairpintypes.h"
#include "types/articulationtypes.h"
#include "types/dynamictypes.h"
#include "types/glissandotypes.h"
#include "types/fermatatypes.h"

void InspectorsSetup::registerQmlTypes()
{
    qmlRegisterType<DoubleInputValidator>("MuseScore.Inspectors", 3, 3, "DoubleInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.Inspectors", 3, 3, "IntInputValidator");
    qmlRegisterUncreatableType<AbstractInspectorModel>("MuseScore.Inspectors", 3, 3, "Inspector", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<StemTypes>("MuseScore.Inspectors", 3, 3, "Stem", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<NoteHeadTypes>("MuseScore.Inspectors", 3, 3, "NoteHead", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BeamTypes>("MuseScore.Inspectors", 3, 3, "Beam", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<HairpinTypes>("MuseScore.Inspectors", 3, 3, "Hairpin", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ArticulationTypes>("MuseScore.Inspectors", 3, 3, "Articulation", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DynamicTypes>("MuseScore.Inspectors", 3, 3, "Dynamic", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<GlissandoTypes>("MuseScore.Inspectors", 3, 3, "Glissando", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FermataTypes>("MuseScore.Inspectors", 3, 3, "FermataTypes", "Not creatable as it is an enum type");
}
