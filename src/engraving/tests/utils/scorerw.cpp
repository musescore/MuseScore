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
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/400/tread.h"
#include "engraving/rw/400/twrite.h"
#include "engraving/rw/rwregister.h"
#include "engraving/libmscore/factory.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

String ScoreRW::m_rootPath;

void ScoreRW::setRootPath(const String& path)
{
    m_rootPath = path;
}

String ScoreRW::rootPath()
{
    return m_rootPath;
}

MasterScore* ScoreRW::readScore(const String& name, bool isAbsolutePath, ImportFunc importFunc)
{
    io::path_t path = isAbsolutePath ? name : (rootPath() + u"/" + name);
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle();
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));
    std::string suffix = io::suffix(path);

    ScoreLoad sl;
    Err rv;
    if (suffix == "mscz" || suffix == "mscx") {
        rv = static_cast<Err>(compat::loadMsczOrMscx(score, path.toString(), false).code());
    } else if (importFunc) {
        rv = importFunc(score, path);
    } else {
        rv = Err::FileUnknownType;
    }

    if (rv != Err::NoError) {
        LOGE() << "can't load score, path: " << path;
        delete score;
        return nullptr;
    }

    for (Score* s : score->scoreList()) {
        s->doLayout();
    }

    // While reading the score, some elements might use `score->repeatList()` (which is incorrect
    // anyway, because the repeatList will be incomplete because the score is incomplete, but some
    // elements still do it).
    // `score->repeatList()` calls `_repeatList->update()`; the repeat list then thinks that it is
    // up-to-date from that point. But we weren't finished reading the score, so the score will still
    // change. We need to tell the repeat list about that, so that it will be updated next time
    // someone uses it.
    score->setPlaylistDirty();

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

    return rw::RWRegister::latestWriter()->writeScore(score, &file, false);
}

EngravingItem* ScoreRW::writeReadElement(EngravingItem* element)
{
    //
    // write element
    //
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);
    rw400::WriteContext wctx;
    xml.startDocument();
    rw400::TWrite::writeItem(element, xml, wctx);
    xml.flush();
    buffer.close();

    //
    // read element
    //

    rw400::ReadContext rctx;
    XmlReader e(buffer.data());
    e.readNextStartElement();
    element = Factory::createItemByName(e.name(), element->score()->dummy());
    rw400::TRead::readItem(element, e, rctx);
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
