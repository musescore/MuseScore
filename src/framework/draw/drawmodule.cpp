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

#include "modularity/ioc.h"

#ifndef DRAW_NO_INTERNAL
#include "internal/qfontprovider.h"
#include "internal/qimageprovider.h"
#endif

using namespace mu::draw;

std::string DrawModule::moduleName() const
{
    return "draw";
}

void DrawModule::registerExports()
{
#ifndef DRAW_NO_INTERNAL
    mu::modularity::ioc()->registerExport<draw::IFontProvider>(moduleName(), new QFontProvider());
    mu::modularity::ioc()->registerExport<draw::IImageProvider>(moduleName(), new QImageProvider());
#endif
}
