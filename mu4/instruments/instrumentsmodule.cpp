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
#include "instrumentsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/instrumentsreader.h"
#include "internal/instrumentsrepository.h"
#include "internal/instrumentsconfiguration.h"

#include "view/instrumentlistmodel.h"
#include "ui/iinteractiveuriregister.h"

using namespace mu::instruments;
using namespace mu::framework;

static InstrumentsRepository* m_instrumentsRepository = new InstrumentsRepository();

static void instruments_init_qrc()
{
    Q_INIT_RESOURCE(instruments);
}

std::string InstrumentsModule::moduleName() const
{
    return "instruments";
}

void InstrumentsModule::registerExports()
{
    ioc()->registerExport<IInstrumentsConfiguration>(moduleName(), new InstrumentsConfiguration());
    ioc()->registerExport<IInstrumentsRepository>(moduleName(), m_instrumentsRepository);
    ioc()->registerExport<IInstrumentsReader>(moduleName(), new InstrumentsReader());
}

void InstrumentsModule::resolveImports()
{
}

void InstrumentsModule::registerResources()
{
    instruments_init_qrc();
}

void InstrumentsModule::registerUiTypes()
{
    qmlRegisterType<InstrumentListModel>("MuseScore.Instruments", 1, 0, "InstrumentListModel");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(instruments_QML_IMPORT);
}

void InstrumentsModule::onInit()
{
    m_instrumentsRepository->init();
}
