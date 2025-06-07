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
#pragma once

#include "../iwriter.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"

#include "writecontext.h"
#include "../compat/writescorehook.h"

namespace mu::engraving::write {
class Writer : public rw::IWriter, public muse::Injectable
{
    muse::Inject<muse::IApplication> application = { this };

public:

    Writer(const muse::modularity::ContextPtr& iocCtx);

    bool writeScore(Score* score, muse::io::IODevice* device, bool onlySelection, rw::WriteInOutData* out) override;

    static void write(Score* score, XmlWriter& xml, WriteContext& ctx, bool selectionOnly, compat::WriteScoreHook& hook);

    void writeSegments(XmlWriter& xml, SelectionFilter* filter, track_idx_t st, track_idx_t et, Segment* sseg, Segment* eseg, bool, bool,
                       Fraction& curTick) override;

private:
    void doWriteItem(const EngravingItem* item, XmlWriter& xml) override;
};
}
