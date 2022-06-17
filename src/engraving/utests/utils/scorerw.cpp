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

#include "scorerw.h"

#include "io/file.h"
#include "io/buffer.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/writescorehook.h"
#include "engraving/infrastructure/io/localfileinfoprovider.h"
#include "engraving/rw/xml.h"
#include "engraving/libmscore/factory.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

String ScoreRW::rootPath()
{
    return String(engraving_utests_DATA_ROOT);
}

MasterScore* ScoreRW::readScore(const String& name, bool isAbsolutePath)
{
    io::path_t path = isAbsolutePath ? name : (rootPath() + u"/" + name);
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));
    std::string suffix = io::suffix(path);

    ScoreLoad sl;
    Score::FileError rv;
    if (suffix == "mscz" || suffix == "mscx") {
        rv = compat::loadMsczOrMscx(score, path.toString(), false);
    } else {
        rv = Score::FileError::FILE_UNKNOWN_TYPE;
    }

    if (rv != Score::FileError::FILE_NO_ERROR) {
        LOGE() << "can't load score, path: " << path;
        delete score;
        score = nullptr;
    } else {
        for (Score* s : score->scoreList()) {
            s->doLayout();
        }
    }

    return score;
}

bool ScoreRW::saveScore(Score* score, const String& name)
{
    File file(name);
    if (file.exists()) {
        file.remove();
    }

    if (!file.open(IODevice::ReadWrite)) {
        return false;
    }
    compat::WriteScoreHook hook;
    return score->writeScore(&file, false, false, hook);
}

EngravingItem* ScoreRW::writeReadElement(EngravingItem* element)
{
    //
    // write element
    //
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);
    xml.startDocument();
    element->write(xml);
    xml.flush();
    buffer.close();

    //
    // read element
    //

    XmlReader e(buffer.data());
    e.readNextStartElement();
    element = Factory::createItemByName(e.name(), element->score()->dummy());
    element->read(e);
    return element;
}

bool ScoreRW::saveMimeData(ByteArray mimeData, const String& saveName)
{
    File f(saveName);
    if (!f.open(IODevice::WriteOnly)) {
        return false;
    }

    size_t size = f.write(mimeData);
    return size == mimeData.size();
}
