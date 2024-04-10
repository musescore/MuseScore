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
#include "drawmodule.h"

#include "global/modularity/ioc.h"

#ifndef DRAW_NO_INTERNAL
#include "internal/qfontprovider.h"
#include "internal/qimageprovider.h"

#include "internal/fontprovider.h"
#include "internal/fontsengine.h"
#include "internal/fontsdatabase.h"
#endif

#include "muse_framework_config.h"

using namespace muse::draw;
using namespace muse::modularity;

std::string DrawModule::moduleName() const
{
    return "draw";
}

void DrawModule::registerExports()
{
#ifndef DRAW_NO_INTERNAL

    ioc()->registerExport<draw::IImageProvider>(moduleName(), new QImageProvider());

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    ioc()->registerExport<draw::IFontProvider>(moduleName(), new QFontProvider());
#else
    m_fontsEngine = std::make_shared<FontsEngine>();
    ioc()->registerExport<draw::IFontProvider>(moduleName(), new FontProvider());
    ioc()->registerExport<draw::IFontsEngine>(moduleName(), m_fontsEngine);
    ioc()->registerExport<draw::IFontsDatabase>(moduleName(), new FontsDatabase());
#endif // MUSE_MODULE_DRAW_USE_QTFONTMETRICS

#endif // DRAW_NO_INTERNAL
}

void DrawModule::onInit(const IApplication::RunMode&)
{
#ifndef DRAW_NO_INTERNAL
#ifndef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    m_fontsEngine->init();
#endif
#endif // DRAW_NO_INTERNAL
}
