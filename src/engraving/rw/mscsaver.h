/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "global/modularity/ioc.h"
#include "draw/iimageprovider.h"

#include "../infrastructure/mscwriter.h"

namespace mu::engraving::write {
struct WriteRange;
}

namespace mu::engraving {
class MasterScore;
class Score;
class MscSaver : public muse::Injectable
{
    muse::Inject<muse::draw::IImageProvider> imageProvider = { this };
public:
    MscSaver(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    bool writeMscz(MasterScore* score, MscWriter& mscWriter, bool createThumbnail, const write::WriteRange* range = nullptr);

    bool exportPart(Score* partScore, MscWriter& mscWriter);
};
}
