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
#include "abdrawrefstep.h"

#include "log.h"
#include "io/path.h"
#include "engraving/infrastructure/draw/utils/drawjson.h"

using namespace mu::autobot;

std::string AbDrawRefStep::name() const
{
    return "DrawRef";
}

void AbDrawRefStep::doRun(IAbContextPtr ctx)
{
    io::path scorePath = ctx->globalVal<io::path>(IAbContext::Key::FilePath);
    io::path filePath = configuration()->fileDrawDataPath(scorePath);
    Ret ret = fileSystem()->exists(filePath);
    if (!ret) {
        LOGE() << "failed open file to write draw data, path: " << filePath;
        doFinish(ctx, ret);
        return;
    }

    RetVal<QByteArray> data = fileSystem()->readFile(filePath);
    if (!data.ret) {
        LOGE() << "failed read file, err: " << data.ret << ", file: " << filePath;
        doFinish(ctx, data.ret);
    }

    RetVal<draw::DrawDataPtr> buf = draw::DrawBufferJson::fromJson(data.val);
    if (!buf.ret) {
        LOGE() << "failed parse, err: " << buf.ret.toString() << ", file: " << filePath;
        doFinish(ctx, buf.ret);
    }

    ctx->setStepVal(IAbContext::Key::RefDrawData, buf.val);
    doFinish(ctx, make_ret(Ret::Code::Ok));
}
