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

    QByteArray data = AbPaintProvider::instance()->serialize();

    io::path scorePath = ctx.val<io::path>(AbContext::Key::ScoreFile);
    io::path filePath = drawDataPath + "/" + io::basename(scorePath) + ".json";
    QFile file(filePath.toQString());
    if (!file.open(QIODevice::WriteOnly)) {
        LOGE() << "failed open file to write draw data, path: " << filePath;
        doFinish(ctx);
        return;
    }

    file.write(data);

    doFinish(ctx);
}
