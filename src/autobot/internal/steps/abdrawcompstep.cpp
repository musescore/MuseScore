//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "abdrawcompstep.h"

#include "libmscore/draw/drawtypes.h"

#include "log.h"

using namespace mu::autobot;

void AbDrawCompStep::doRun(AbContext ctx)
{
    draw::DrawBufferPtr curBuf = ctx.val<draw::DrawBufferPtr>(AbContext::Key::CurDrawBuf);
    if (!curBuf) {
        LOGW() << "not set current draw buffer";
        doFinish(ctx);
    }

    draw::DrawBufferPtr refBuf = ctx.val<draw::DrawBufferPtr>(AbContext::Key::RefDrawBuf);
    if (!refBuf) {
        LOGW() << "not set reference draw buffer";
        doFinish(ctx);
    }

    //! NOTE This is just a very simple comparator,
    //! In the next PR will be doing a real comparator
    if (curBuf->objects.size() != refBuf->objects.size()) {
        LOGE() << "the current and reference draw buffers are different";
    }

    doFinish(ctx);
}
