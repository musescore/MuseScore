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

static const muse::modularity::ContextPtr utestCtx = std::make_shared<muse::modularity::Context>(1);

MasterScore* ScoreAccess::createMasterScore(const muse::modularity::ContextPtr& iocCtx)
{
    const muse::modularity::ContextPtr iocCtx_ = iocCtx ? iocCtx : utestCtx;
    return new MasterScore(iocCtx_);
}

MasterScore* ScoreAccess::createMasterScoreWithBaseStyle(const muse::modularity::ContextPtr& iocCtx)
{
    const muse::modularity::ContextPtr iocCtx_ = iocCtx ? iocCtx : utestCtx;
    return new MasterScore(iocCtx_, DefaultStyle::baseStyle());
}

MasterScore* ScoreAccess::createMasterScoreWithDefaultStyle(const muse::modularity::ContextPtr& iocCtx)
{
    const muse::modularity::ContextPtr iocCtx_ = iocCtx ? iocCtx : utestCtx;
    return new MasterScore(iocCtx_, DefaultStyle::defaultStyle());
}

MasterScore* ScoreAccess::createMasterScore(const muse::modularity::ContextPtr& iocCtx, const MStyle& style)
{
    const muse::modularity::ContextPtr iocCtx_ = iocCtx ? iocCtx : utestCtx;
    return new MasterScore(iocCtx_, style);
}
