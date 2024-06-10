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
#include "scoreaccess.h"
#include "style/defaultstyle.h"
#include "dom/masterscore.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;

MasterScore* ScoreAccess::createMasterScore(const muse::modularity::ContextPtr& iocCtx)
{
    return new MasterScore(iocCtx);
}

MasterScore* ScoreAccess::createMasterScoreWithBaseStyle(const muse::modularity::ContextPtr& iocCtx)
{
    return new MasterScore(iocCtx, DefaultStyle::baseStyle());
}

MasterScore* ScoreAccess::createMasterScoreWithDefaultStyle(const muse::modularity::ContextPtr& iocCtx)
{
    return new MasterScore(iocCtx, DefaultStyle::defaultStyle());
}

MasterScore* ScoreAccess::createMasterScore(const muse::modularity::ContextPtr& iocCtx, const MStyle& style)
{
    return new MasterScore(iocCtx, style);
}
