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
#include "mscloader.h"

#include <memory>

#include "global/io/buffer.h"
#include "global/types/retval.h"

#include "../engravingerrors.h"

#include "../types/types.h"

#include "../dom/masterscore.h"
#include "../dom/audio.h"
#include "../dom/excerpt.h"
#include "../dom/imageStore.h"

#include "compat/compatutils.h"
#include "compat/readstyle.h"

#include "rwregister.h"
#include "xmlreader.h"
#include "inoutdata.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::engraving::rw;

static RetVal<IReaderPtr> makeReader(int version, bool ignoreVersionError)
{
    if (!ignoreVersionError) {
        if (version > Constants::MSC_VERSION) {
            return RetVal<IReaderPtr>(make_ret(Err::FileTooNew));
        }

        if (version < 114) {
            return RetVal<IReaderPtr>(make_ret(Err::FileTooOld));
        }

        if (version == 300) {
            return RetVal<IReaderPtr>(make_ret(Err::FileOld300Format));
        }
    }

    if (version > 302 && MScore::testMode && MScore::useRead302InTestMode) {
        version = 302;
    }

    return RetVal<IReaderPtr>::make_ok(RWRegister::reader(version));
}

Ret MscLoader::loadMscz(MasterScore* masterScore, const MscReader& mscReader, SettingsCompat& settingsCompat,
                        bool ignoreVersionError, rw::ReadInOutData* inOut)
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
            if (inOut) {
                inOut->originalSpatium = masterScore->style().spatium();
            }
        }
    }

    // Read ChordList
    {
        bool chordListOk = false;
        ByteArray chordListData = mscReader.readChordListFile();
        if (!chordListData.empty()) {
            Buffer buf(&chordListData);
            buf.open(IODevice::ReadOnly);

            chordListOk = masterScore->chordList()->read(&buf);
        }

        masterScore->chordList()->setCustomChordList(chordListOk);

        if (!chordListOk) {
            // See also ReadChordListHook::validate()
            MStyle& style = masterScore->style();
            ChordList* chordList = masterScore->chordList();

            bool custom = style.styleSt(Sid::chordStyle) == "custom";
            chordList->setCustomChordList(custom);

            // Ensure that `checkChordList` loads the default chord list
            chordList->unload();
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
    if (!inOut) {
        inOut = &masterReadOutData;
    }

    Ret ret = muse::make_ok();

    // Read score
    {
        ByteArray scoreData = mscReader.readScoreFile();
        String docName = masterScore->fileInfo()->fileName().toString();

        compat::ReadStyleHook styleHook(masterScore, scoreData, docName);

        XmlReader xml(scoreData);
        xml.setDocName(docName);

        ret = readMasterScore(masterScore, xml, ignoreVersionError, inOut, &styleHook);
    }

    // Read excerpts
    if (ret && masterScore->mscVersion() >= 400) {
        std::vector<String> excerptFileNames = mscReader.excerptFileNames();
        for (const String& excerptFileName : excerptFileNames) {
            Score* partScore = masterScore->createScore();

            compat::ReadStyleHook::setupDefaultStyle(partScore);

            Excerpt* ex = new Excerpt(masterScore);
            ex->setExcerptScore(partScore);
            ex->setFileName(excerptFileName);

            ByteArray excerptStyleData = mscReader.readExcerptStyleFile(excerptFileName);
            Buffer excerptStyleBuf(&excerptStyleData);
            excerptStyleBuf.open(IODevice::ReadOnly);
            partScore->style().read(&excerptStyleBuf);

            ByteArray excerptData = mscReader.readExcerptFile(excerptFileName);

            XmlReader xml(excerptData);
            xml.setDocName(excerptFileName);

            ReadInOutData partReadInData;
            partReadInData.links = inOut->links;

            RetVal<IReaderPtr> reader = makeReader(masterScore->mscVersion(), ignoreVersionError);
            if (!reader.ret) {
                ret = reader.ret;
                break;
            }

            ret = reader.val->readScore(partScore, xml, &partReadInData);
            if (!ret) {
                break;
            }

            partScore->linkMeasures(masterScore);

            if (ex->name().empty()) {
                // If no excerpt name tag was found while reading, try the "partName" meta tag
                const String nameFromMeta = partScore->metaTag(u"partName");

                if (nameFromMeta.empty()) {
                    // If that's also empty, fall back to the filename
                    ex->setName(excerptFileName, /*saveAndNotify=*/ false);
                } else {
                    ex->setName(nameFromMeta, /*saveAndNotify=*/ false);
                }
            }

            masterScore->addExcerpt(ex);
        }
    }

    // Compatibility conversions
    // NOTE: must be done after all score and parts have been read
    compat::CompatUtils::doCompatibilityConversions(masterScore);

    //  Read audio
    {
        if (masterScore->audio()) {
            ByteArray dbuf1 = mscReader.readAudioFile();
            masterScore->audio()->setData(dbuf1);
        }
    }

    settingsCompat = std::move(inOut->settingsCompat);

    return ret;
}

Ret MscLoader::readMasterScore(MasterScore* score, XmlReader& e, bool ignoreVersionError, ReadInOutData* out,
                               compat::ReadStyleHook* styleHook)
{
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            const String version = e.attribute("version");
            const StringList sl = version.split(u'.');
            score->setMscVersion(sl[0].toInt() * 100 + sl[1].toInt());

            RetVal<IReaderPtr> reader = makeReader(score->mscVersion(), ignoreVersionError);
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

            Ret ret = reader.val->readScore(score, e, out);

            score->setExcerptsChanged(false);

            return ret;
        } else {
            e.unknown();
        }
    }

    return make_ret(Err::FileCriticallyCorrupted, e.errorString());
}
