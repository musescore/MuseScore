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
#include "notationdomainmodule.h"

#include "modularity/ioc.h"
#include "internal/notationcreator.h"
#include "internal/notation.h"
#include "internal/notationactioncontroller.h"
#include "internal/notationconfiguration.h"

#include "actions/iactionsregister.h"
#include "internal/notationactions.h"
#include "internal/notationreadersregister.h"
#include "internal/mscznotationreader.h"

using namespace mu::domain::notation;

static NotationConfiguration* m_configuration = new NotationConfiguration();

std::string NotationDomainModule::moduleName() const
{
    return "notation\
    ";
}

void NotationDomainModule::registerExports()
{
    framework::ioc()->registerExport<INotationCreator>(moduleName(), new NotationCreator());
    framework::ioc()->registerExport<INotationConfiguration>(moduleName(), m_configuration);

    std::shared_ptr<INotationReadersRegister> readers = std::make_shared<NotationReadersRegister>();
    readers->reg({ "mscz", "mscx" }, std::make_shared<MsczNotationReader>());
    framework::ioc()->registerExport<INotationReadersRegister>(moduleName(), readers);
}

void NotationDomainModule::resolveImports()
{
    auto ar = framework::ioc()->resolve<actions::IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<NotationActions>());
    }
}

void NotationDomainModule::onInit()
{
    Notation::init();
    NotationActionController::instance(); //! NOTE Only need to create
    m_configuration->init();
}
