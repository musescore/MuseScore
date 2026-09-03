/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <memory>

#include "global/modularity/ioc.h"
#include "draw/iimageprovider.h"

#include "../iengravingconfiguration.h"
#include "../infrastructure/mscwriter.h"
#include "../rendering/iscorerenderer.h"

namespace muse::draw {
class Pixmap;
}

namespace mu::engraving::write {
class WriteContext;
}

namespace mu::engraving {
class MasterScore;
class Score;
class MscSaver : public muse::Contextable
{
    muse::GlobalInject<muse::draw::IImageProvider> imageProvider;
    muse::GlobalInject<IEngravingConfiguration> configuration;
    muse::GlobalInject<rendering::IScoreRenderer> scoreRenderer;
public:
    MscSaver(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    bool writeMscz(MasterScore* score, MscWriter& mscWriter, bool createThumbnail, const write::WriteContext* ctx = nullptr);

    bool exportPart(Score* partScore, MscWriter& mscWriter);

private:
    std::shared_ptr<muse::draw::Pixmap> createThumbnail(Score* score);
};

/* Multimeasure rest layout information is only generated for visible parts, so before
 * serializing we must unhide every hidden part and relayout, then rollback the change
 * once everything has been written. This RAII wrapper does that, then rolls back
 * on destruction. Must be created and destroyed on the thread that owns the score
 * because mutating and re-laying out a score is not thread safe, so it cannot happen
 * while the excerpts are being serialised in parallel.
 */
class UnhidePartsForWrite
{
public:
    UnhidePartsForWrite(Score* score);
    UnhidePartsForWrite(const UnhidePartsForWrite&) = delete;
    UnhidePartsForWrite& operator=(const UnhidePartsForWrite&) = delete;
    ~UnhidePartsForWrite();

    void rollback();

private:
    Score* m_score = nullptr;
};
}
