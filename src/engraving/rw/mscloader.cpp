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
#include "mscloader.h"

#include <memory>
#include <map>

#include "global/io/buffer.h"
#include "global/types/retval.h"

#include "../libmscore/masterscore.h"
#include "../libmscore/audio.h"
#include "../libmscore/excerpt.h"
#include "../libmscore/imageStore.h"

#include "114/read114.h"
#include "206/read206.h"
#include "302/read302.h"
#include "400/read400.h"

#include "compat/readstyle.h"

#include "xmlreader.h"
#include "400/readcontext.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

using IScoreReaderPtr = std::shared_ptr<IScoreReader>;

static RetVal<IScoreReaderPtr> makeReader(int version, bool ignoreVersionError)
{
    if (!ignoreVersionError) {
        if (version > MSCVERSION) {
            return RetVal<IScoreReaderPtr>(make_ret(Err::FileTooNew));
        }

        if (version < 114) {
            return RetVal<IScoreReaderPtr>(make_ret(Err::FileTooOld));
        }

        if (version == 300) {
            return RetVal<IScoreReaderPtr>(make_ret(Err::FileOld300Format));
        }
    }

    if (version <= 114) {
        return RetVal<IScoreReaderPtr>::make_ok(std::make_shared<compat::Read114>());
    } else if (version <= 207) {
        return RetVal<IScoreReaderPtr>::make_ok(std::make_shared<compat::Read206>());
    } else if (version < 400 || MScore::testMode) {
        return RetVal<IScoreReaderPtr>::make_ok(std::make_shared<compat::Read302>());
    }

    return RetVal<IScoreReaderPtr>::make_ok(std::make_shared<rw400::Read400>());
}

mu::Ret MscLoader::loadMscz(MasterScore* masterScore, const MscReader& mscReader, SettingsCompat& settingsCompat,
                            bool ignoreVersionError)
{
    TRACEFUNC;

    using namespace mu::engraving;

    IF_ASSERT_FAILED(mscReader.isOpened()) {
        return make_ret(Err::FileOpenError, mscReader.params().filePath);
    }

    ScoreLoad sl;

    // Read style
    {
        ByteArray styleData = mscReader.readStyleFile();
        if (!styleData.empty()) {
            Buffer buf(&styleData);
            buf.open(IODevice::ReadOnly);
            masterScore->style().read(&buf);
        }
    }

    // Read ChordList
    {
        ByteArray chordListData = mscReader.readChordListFile();
        if (!chordListData.empty()) {
            Buffer buf(&chordListData);
            buf.open(IODevice::ReadOnly);
            masterScore->chordList()->read(&buf);
        }
    }

    // Read images
    {
        if (!MScore::noImages) {
            std::vector<String> images = mscReader.imageFileNames();
            for (const String& name : images) {
                imageStore.add(name, mscReader.readImageFile(name));
            }
        }
    }

    ReadInOutData masterReadOutData;

    Ret ret = make_ok();

    // Read score
    {
        ByteArray scoreData = mscReader.readScoreFile();
        String docName = masterScore->fileInfo()->fileName().toString();

        compat::ReadStyleHook styleHook(masterScore, scoreData, docName);

        XmlReader xml(scoreData);
        xml.setDocName(docName);

        ret = read(masterScore, xml, ignoreVersionError, &masterReadOutData, &styleHook);
    }

    // Read excerpts
    if (masterScore->mscVersion() >= 400) {
        std::vector<String> excerptNames = mscReader.excerptNames();
        for (const String& excerptName : excerptNames) {
            Score* partScore = masterScore->createScore();

            compat::ReadStyleHook::setupDefaultStyle(partScore);

            Excerpt* ex = new Excerpt(masterScore);
            ex->setExcerptScore(partScore);

            ByteArray excerptStyleData = mscReader.readExcerptStyleFile(excerptName);
            Buffer excerptStyleBuf(&excerptStyleData);
            excerptStyleBuf.open(IODevice::ReadOnly);
            partScore->style().read(&excerptStyleBuf);

            ByteArray excerptData = mscReader.readExcerptFile(excerptName);

            XmlReader xml(excerptData);
            xml.setDocName(excerptName);

            ReadInOutData partReadInData;
            partReadInData.links = masterReadOutData.links;

            rw400::Read400().read(partScore, xml, &partReadInData);

            partScore->linkMeasures(masterScore);

            ex->setName(excerptName);

            masterScore->addExcerpt(ex);
        }

        // Needs to be re-done after reading excerpts, since the sigmap gets messed up while reading excerpts
        // (only necessary for >= 400, because for older scores, part scores are read "recursively",
        // so the `setupTempoMap` for the master score is last anyway)
        masterScore->setUpTempoMap();
    }

    //  Read audio
    {
        if (masterScore->audio()) {
            ByteArray dbuf1 = mscReader.readAudioFile();
            masterScore->audio()->setData(dbuf1);
        }
    }

    settingsCompat = std::move(masterReadOutData.settingsCompat);

    return ret;
}

mu::Ret MscLoader::read(MasterScore* score, XmlReader& e, bool ignoreVersionError, ReadInOutData* out, compat::ReadStyleHook* styleHook)
{
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            const String& version = e.attribute("version");
            StringList sl = version.split('.');
            score->setMscVersion(sl[0].toInt() * 100 + sl[1].toInt());

            RetVal<IScoreReaderPtr> reader = makeReader(score->mscVersion(), ignoreVersionError);
            if (!reader.ret) {
                return reader.ret;
            }

            IF_ASSERT_FAILED(reader.val) {
                return make_ret(Err::FileUnknownError);
            }

            //! NOTE We need to achieve that the default style corresponds to the version in which the score is created.
            //! The values that the user changed will be written on over (only they are stored in the `mscz` file)
            //! For version 4.0 (400), this does not need to be done,
            //! because starting from version 4.0 the entire style is stored in a file,
            //! respectively, the entire style will be loaded, which was when the score was created.
            if (styleHook && (score->mscVersion() < 400 || MScore::testMode)) {
                styleHook->setupDefaultStyle();
            }

            if (score->mscVersion() >= 400) {
                //! NOTE: make sure we have a chord list
                //! Load the default chord list otherwise
                score->checkChordList();
            }

            Err err = reader.val->read(score, e, out);

            score->setExcerptsChanged(false);

            // don't autosave (as long as there's no change to the score)
            score->setAutosaveDirty(false);

            return make_ret(err);
        } else {
            e.unknown();
        }
    }

    return Ret(static_cast<int>(Err::FileCorrupted), e.errorString().toStdString());
}
