/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "global/io/buffer.h"
#include "global/io/file.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/mscxcompat.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/read400/tread.h"
#include "engraving/rw/write/twrite.h"
#include "engraving/rw/rwregister.h"
#include "engraving/dom/factory.h"
#include "engraving/rw/rwregister.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
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
    muse::io::path_t path = isAbsolutePath ? name : (rootPath() + u"/" + name);
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path));
    std::string suffix = muse::io::suffix(path);

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
    File::remove(name);

    auto output = Buffer::opened(IODevice::WriteOnly);
    if (!rw::RWRegister::writer()->writeScore(score, &output)) {
        return false;
    }
    output.close();

    return File::writeFile(name, output.data());
}

bool ScoreRW::saveScore(Score* score, const String& name, ExportFunc exportFunc)
{
    File::remove(name);

    muse::io::path_t path =  name;
    Err rv = exportFunc(score, path);

    if (rv != Err::NoError) {
        LOGE() << "can't load score, path: " << path;
        return false;
    } else {
        return true;
    }
}

EngravingItem* ScoreRW::writeReadElement(EngravingItem* element)
{
    //
    // write element
    //
    auto buffer = Buffer::opened(IODevice::WriteOnly);
    XmlWriter xml(&buffer);
    xml.startDocument();
    rw::RWRegister::writer()->writeItem(element, xml);
    xml.flush();
    buffer.close();

    //
    // read element
    //

    XmlReader e(buffer.data());
    e.readNextStartElement();
    element = Factory::createItemByName(e.name(), element->score()->dummy());
    rw::RWRegister::reader()->readItem(element, e);
    return element;
}

bool ScoreRW::saveMimeData(muse::ByteArray mimeData, const String& saveName)
{
    return File::writeFile(saveName, mimeData);
}
