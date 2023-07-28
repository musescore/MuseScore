/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "braillemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "project/inotationwritersregister.h"

#include "internal/brailleconfiguration.h"
#include "internal/brailleconverter.h"
#include "internal/notationbraille.h"
#include "internal/braillewriter.h"

#include "inotationbraille.h"

#include "view/braillemodel.h"

using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

namespace mu::braille {
std::string BrailleModule::moduleName() const
{
    return "braille";
}

void BrailleModule::resolveImports()
{
    auto writers = modularity::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "brf" }, std::make_shared<BrailleWriter>());
    }
}

void BrailleModule::registerExports()
{
    m_brailleConfiguration = std::make_shared<BrailleConfiguration>();
    m_brailleConverter = std::make_shared<BrailleConverter>();
    m_notationBraille = std::make_shared<NotationBraille>();

    modularity::ioc()->registerExport<IBrailleConfiguration>(moduleName(), m_brailleConfiguration);
    modularity::ioc()->registerExport<IBrailleConverter>(moduleName(), m_brailleConverter);
    modularity::ioc()->registerExport<INotationBraille>(moduleName(), m_notationBraille);
}

void BrailleModule::registerUiTypes()
{
    using namespace notation;

    qmlRegisterType<BrailleModel>("MuseScore.Braille", 1, 0, "BrailleModel");

    modularity::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(braille_QML_IMPORT);
}

void BrailleModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_brailleConfiguration->init();
    m_notationBraille->init();
}
}
