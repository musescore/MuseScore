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
#include "abdrawserializationstep.h"

#include <QFile>

#include "log.h"
#include "../draw/abpaintprovider.h"
#include "libmscore/draw/drawjson.h"

using namespace mu::autobot;

void AbDrawSerializationStep::doRun(AbContext ctx)
{
    io::path drawDataPath = configuration()->drawDataPath();
    if (!fileSystem()->exists(drawDataPath)) {
        Ret ret = fileSystem()->makePath(drawDataPath);
        if (!ret) {
            LOGE() << "failed make path: " << drawDataPath;
            doFinish(ctx);
            return;
        }
    }

    const draw::DrawData& buf = AbPaintProvider::instance()->notationViewDrawData();
    ctx.setVal<draw::DrawDataPtr>(AbContext::Key::CurDrawData, std::make_shared<draw::DrawData>(buf));

    QByteArray data = draw::DrawBufferJson::toJson(buf);

    io::path scorePath = ctx.val<io::path>(AbContext::Key::ScoreFile);
    io::path filePath = configuration()->scoreDrawData(scorePath);
    QFile file(filePath.toQString());
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open file to write draw data, path: " << filePath;
        doFinish(ctx);
        return;
    }

    file.write(data);

    doFinish(ctx);
}
