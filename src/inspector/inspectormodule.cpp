/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "inspectormodule.h"
#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "models/abstractinspectormodel.h"
#include "models/inspectorlistmodel.h"
#include "models/inspectormodelcreator.h"
#include "models/measures/measuressettingsmodel.h"
#include "models/notation/notes/noteheads/noteheadgroupsmodel.h"
#include "models/inspectorpopupcontroller.h"

#include "view/widgets/fretcanvas.h"
#include "view/widgets/bendgridcanvas.h"
#include "view/widgets/gridcanvas.h"

#include "types/directiontypes.h"
#include "types/slurtietypes.h"
#include "types/noteheadtypes.h"
#include "types/beamtypes.h"
#include "types/hairpintypes.h"
#include "types/ornamenttypes.h"
#include "types/dynamictypes.h"
#include "types/glissandotypes.h"
#include "types/commontypes.h"
#include "types/barlinetypes.h"
#include "types/markertypes.h"
#include "types/keysignaturetypes.h"
#include "types/accidentaltypes.h"
#include "types/fretdiagramtypes.h"
#include "types/texttypes.h"
#include "types/articulationtypes.h"
#include "types/ambitustypes.h"
#include "types/chordsymboltypes.h"
#include "types/bendtypes.h"
#include "types/tremolobartypes.h"
#include "types/tremolotypes.h"
#include "types/voicetypes.h"
#include "types/linetypes.h"

using namespace mu::inspector;
using namespace muse::modularity;

static void inspector_init_qrc()
{
    Q_INIT_RESOURCE(inspector_resources);
}

std::string InspectorModule::moduleName() const
{
    return "inspector";
}

void InspectorModule::registerExports()
{
    ioc()->registerExport<IInspectorModelCreator>(moduleName(), new InspectorModelCreator());
}

void InspectorModule::registerResources()
{
    inspector_init_qrc();
}

void InspectorModule::registerUiTypes()
{
    qmlRegisterType<InspectorListModel>("MuseScore.Inspector", 1, 0, "InspectorListModel");
    qmlRegisterUncreatableType<AbstractInspectorModel>("MuseScore.Inspector", 1, 0, "Inspector", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<PropertyItem>("MuseScore.Inspector", 1, 0, "PropertyItem", "Not creatable from QML");

    qmlRegisterUncreatableType<MeasuresSettingsModel>("MuseScore.Inspector", 1, 0, "MeasuresSettingsModel", "Not creatable from QML");
    qmlRegisterType<NoteheadGroupsModel>("MuseScore.Inspector", 1, 0, "NoteheadGroupsModel");

    qmlRegisterUncreatableType<DirectionTypes>("MuseScore.Inspector", 1, 0, "DirectionTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<SlurTieTypes>("MuseScore.Inspector", 1, 0, "SlurTieTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<NoteHeadTypes>("MuseScore.Inspector", 1, 0, "NoteHead", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BeamTypes>("MuseScore.Inspector", 1, 0, "Beam", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<HairpinTypes>("MuseScore.Inspector", 1, 0, "Hairpin", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<OrnamentTypes>("MuseScore.Inspector", 1, 0, "OrnamentTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DynamicTypes>("MuseScore.Inspector", 1, 0, "Dynamic", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<GlissandoTypes>("MuseScore.Inspector", 1, 0, "Glissando", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<CommonTypes>("MuseScore.Inspector", 1, 0, "CommonTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BarlineTypes>("MuseScore.Inspector", 1, 0, "BarlineTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<MarkerTypes>("MuseScore.Inspector", 1, 0, "MarkerTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<KeySignatureTypes>("MuseScore.Inspector", 1, 0, "KeySignatureTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AccidentalTypes>("MuseScore.Inspector", 1, 0, "AccidentalTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FretDiagramTypes>("MuseScore.Inspector", 1, 0, "FretDiagramTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<LineTypes>("MuseScore.Inspector", 1, 0, "LineTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TextTypes>("MuseScore.Inspector", 1, 0, "TextTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ArticulationTypes>("MuseScore.Inspector", 1, 0, "ArticulationTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AmbitusTypes>("MuseScore.Inspector", 1, 0, "AmbitusTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ChordSymbolTypes>("MuseScore.Inspector", 1, 0, "ChordSymbolTypes", "Not creatable as it is an enum type");
    qmlRegisterType<FretCanvas>("MuseScore.Inspector", 1, 0, "FretCanvas");
    qmlRegisterType<BendGridCanvas>("MuseScore.Inspector", 1, 0, "BendGridCanvas");
    qmlRegisterUncreatableType<BendTypes>("MuseScore.Inspector", 1, 0, "BendTypes", "Not creatable as it is an enum type");
    qmlRegisterType<GridCanvas>("MuseScore.Inspector", 1, 0, "GridCanvas");
    qmlRegisterUncreatableType<TremoloBarTypes>("MuseScore.Inspector", 1, 0, "TremoloBarTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TremoloTypes>("MuseScore.Inspector", 1, 0, "TremoloTypes", "Not creatable as it is an enum type");
    qmlRegisterType<InspectorPopupController>("MuseScore.Inspector", 1, 0, "InspectorPopupController");
    qmlRegisterUncreatableType<VoiceTypes>("MuseScore.Inspector", 1, 0, "VoiceTypes", "Not creatable as it is an enum type");

    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(inspector_QML_IMPORT);
}
