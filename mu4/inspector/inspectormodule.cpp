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

#include "inspectormodule.h"
#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::inspector;

static void inspector_init_qrc()
{
    Q_INIT_RESOURCE(inspector_resources);
}

std::string InspectorModule::moduleName() const
{
    return "inspector";
}

#include "iinspectoradapter.h"
#include "internal/compatibility/mu4inspectoradapter.h"

void InspectorModule::registerExports()
{
#ifdef BUILD_UI_MU4
    static std::shared_ptr<MU4InspectorAdapter> adapter = std::make_shared<MU4InspectorAdapter>();

    mu::framework::ioc()->registerExport<mu::inspector::IInspectorAdapter>(moduleName(), adapter);
#endif
}

void InspectorModule::resolveImports()
{
}

void InspectorModule::registerResources()
{
    inspector_init_qrc();
}

#include "models/abstractinspectormodel.h"
#include "models/inspectorlistmodel.h"

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
#include "types/tremolotypes.h"

void InspectorModule::registerUiTypes()
{
    qmlRegisterType<InspectorListModel>("MuseScore.Inspector", 1, 0, "InspectorListModel");
    qmlRegisterUncreatableType<AbstractInspectorModel>("MuseScore.Inspector", 1, 0, "Inspector", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DirectionTypes>("MuseScore.Inspector", 1, 0, "DirectionTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<NoteHeadTypes>("MuseScore.Inspector", 1, 0, "NoteHead", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BeamTypes>("MuseScore.Inspector", 1, 0, "Beam", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<HairpinTypes>("MuseScore.Inspector", 1, 0, "Hairpin", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<OrnamentTypes>("MuseScore.Inspector", 1, 0, "OrnamentTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<DynamicTypes>("MuseScore.Inspector", 1, 0, "Dynamic", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<GlissandoTypes>("MuseScore.Inspector", 1, 0, "Glissando", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FermataTypes>("MuseScore.Inspector", 1, 0, "FermataTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<BarlineTypes>("MuseScore.Inspector", 1, 0, "BarlineTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<MarkerTypes>("MuseScore.Inspector", 1, 0, "MarkerTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<KeySignatureTypes>("MuseScore.Inspector", 1, 0, "KeySignatureTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AccidentalTypes>("MuseScore.Inspector", 1, 0, "AccidentalTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<FretDiagramTypes>("MuseScore.Inspector", 1, 0, "FretDiagramTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<PedalTypes>("MuseScore.Inspector", 1, 0, "PedalTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TextTypes>("MuseScore.Inspector", 1, 0, "TextTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<CrescendoTypes>("MuseScore.Inspector", 1, 0, "CrescendoTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ArticulationTypes>("MuseScore.Inspector", 1, 0, "ArticulationTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<AmbitusTypes>("MuseScore.Inspector", 1, 0, "AmbitusTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<ChordSymbolTypes>("MuseScore.Inspector", 1, 0, "ChordSymbolTypes", "Not creatable as it is an enum type");
    qmlRegisterType<FretCanvas>("MuseScore.Inspector", 1, 0, "FretCanvas");
    qmlRegisterUncreatableType<ScoreAppearanceTypes>("MuseScore.Inspector", 1, 0, "ScoreAppearanceTypes", "Not creatable...");
    qmlRegisterType<GridCanvas>("MuseScore.Inspector", 1, 0, "GridCanvas");
    qmlRegisterUncreatableType<BendTypes>("MuseScore.Inspector", 1, 0, "BendTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TremoloBarTypes>("MuseScore.Inspector", 1, 0, "TremoloBarTypes", "Not creatable as it is an enum type");
    qmlRegisterUncreatableType<TremoloTypes>("MuseScore.Inspector", 1, 0, "TremoloTypes", "Not creatable as it is an enum type");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(inspector_QML_IMPORT);
}
