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
#include "abdrawcurrentstep.h"

#include <QFile>

#include "log.h"
#include "../draw/abpaintprovider.h"
#include "libmscore/draw/drawjson.h"

using namespace mu::autobot;
using namespace mu::draw;

AbDrawCurrentStep::AbDrawCurrentStep(bool isDoSave)
    : m_isDoSave(isDoSave)
{
}

void AbDrawCurrentStep::doRun(AbContext ctx)
{
    const DrawData& buf = AbPaintProvider::instance()->notationViewDrawData();
    ctx.setVal<DrawDataPtr>(AbContext::Key::CurDrawData, std::make_shared<DrawData>(buf));

    if (!m_isDoSave) {
        doFinish(ctx);
        return;
    }

    io::path drawDataPath = configuration()->drawDataPath();
    if (!fileSystem()->exists(drawDataPath)) {
        Ret ret = fileSystem()->makePath(drawDataPath);
        if (!ret) {
            LOGE() << "failed make path: " << drawDataPath;
            doFinish(ctx);
            return;
        }
    }

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
