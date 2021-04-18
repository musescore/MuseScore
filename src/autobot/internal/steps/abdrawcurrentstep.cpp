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

std::string AbDrawCurrentStep::name() const
{
    return "DrawCurrent";
}

void AbDrawCurrentStep::doRun(IAbContextPtr ctx)
{
    const DrawData& buf = AbPaintProvider::instance()->notationViewDrawData();
    ctx->setStepVal(IAbContext::Key::CurDrawData, std::make_shared<DrawData>(buf));

    if (!m_isDoSave) {
        doFinish(ctx, make_ret(Ret::Code::Ok));
        return;
    }

    io::path drawDataPath = configuration()->drawDataPath();
    if (!fileSystem()->exists(drawDataPath)) {
        Ret ret = fileSystem()->makePath(drawDataPath);
        if (!ret) {
            LOGE() << "failed make path: " << drawDataPath;
            doFinish(ctx, ret);
            return;
        }
    }

    QByteArray data = draw::DrawBufferJson::toJson(buf);

    io::path scorePath = ctx->globalVal<io::path>(IAbContext::Key::FilePath);
    io::path filePath = configuration()->fileDrawDataPath(scorePath);
    QFile file(filePath.toQString());
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open file to write draw data, path: " << filePath;
        doFinish(ctx, make_ret(Ret::Code::UnknownError)); //! TODO add specific error code
        return;
    }

    file.write(data);

    doFinish(ctx, make_ret(Ret::Code::Ok));
}
