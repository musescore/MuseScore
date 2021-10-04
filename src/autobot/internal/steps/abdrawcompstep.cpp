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
#include "abdrawcompstep.h"

#include "engraving/infrastructure/draw/drawtypes.h"
#include "engraving/infrastructure/draw/utils/drawcomp.h"

#include "log.h"

using namespace mu::autobot;

std::string AbDrawCompStep::name() const
{
    return "DrawComp";
}

void AbDrawCompStep::doRun(IAbContextPtr ctx)
{
    draw::DrawDataPtr curBuf = ctx->findVal<draw::DrawDataPtr>(IAbContext::Key::CurDrawData);
    if (!curBuf) {
        LOGW() << "not set current draw buffer";
        doFinish(ctx, make_ret(Ret::Code::UnknownError)); //! TODO add specific error code
        return;
    }

    draw::DrawDataPtr refBuf = ctx->findVal<draw::DrawDataPtr>(IAbContext::Key::RefDrawData);
    if (!refBuf) {
        LOGW() << "not set reference draw buffer";
        doFinish(ctx, make_ret(Ret::Code::UnknownError)); //! TODO add specific error code
        return;
    }

    draw::DrawComp::Tolerance tolerance;
    tolerance.base = 0.01;
    draw::Diff diff = draw::DrawComp::compare(curBuf, refBuf, tolerance);
    ctx->setStepVal(IAbContext::Key::DiffDrawData, diff);

    if (diff.empty()) {
        LOGI() << "draw data equals";
    } else {
        LOGE() << "draw data not equals"
               << ", added objects: " << diff.dataAdded->objects.size()
               << ", removed objects: " << diff.dataRemoved->objects.size();
    }

    doFinish(ctx, make_ret(Ret::Code::Ok));
}
