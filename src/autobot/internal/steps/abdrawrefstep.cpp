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
#include "abdrawrefstep.h"

#include "log.h"
#include "io/path.h"
#include "libmscore/draw/drawjson.h"

using namespace mu::autobot;

void AbDrawRefStep::doRun(AbContext ctx)
{
    io::path scorePath = ctx.val<io::path>(AbContext::Key::ScoreFile);
    io::path filePath = configuration()->scoreDrawData(scorePath);
    if (!fileSystem()->exists(filePath)) {
        LOGE() << "failed open file to write draw data, path: " << filePath;
        doFinish(ctx);
        return;
    }

    RetVal<QByteArray> data = fileSystem()->readFile(filePath);
    if (!data.ret) {
        LOGE() << "failed read file, err: " << data.ret << ", file: " << filePath;
        doFinish(ctx);
    }

    RetVal<draw::DrawDataPtr> buf = draw::DrawBufferJson::fromJson(data.val);
    if (!buf.ret) {
        LOGE() << "failed parse, err: " << buf.ret.toString() << ", file: " << filePath;
        doFinish(ctx);
    }

    ctx.setVal<draw::DrawDataPtr>(AbContext::Key::RefDrawData, buf.val);
    doFinish(ctx);
}
