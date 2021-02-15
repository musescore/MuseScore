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
#include "cloudstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "accountcontrollerstub.h"
#include "mp3exporterstub.h"

using namespace mu::cloud;
using namespace mu::framework;

static void cloud_init_qrc()
{
    Q_INIT_RESOURCE(cloud);
}

std::string CloudStubModule::moduleName() const
{
    return "cloud_stub";
}

void CloudStubModule::registerExports()
{
    ioc()->registerExport<IAccountController>(moduleName(), new AccountControllerStub());
    ioc()->registerExport<IMp3Exporter>(moduleName(), new Mp3Exporter());
}

void CloudStubModule::registerResources()
{
    cloud_init_qrc();
}

void CloudStubModule::registerUiTypes()
{
    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(cloud_QML_IMPORT);
}
