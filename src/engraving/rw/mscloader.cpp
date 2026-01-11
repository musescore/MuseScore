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
#include "../dom/part.h"

#include "engraving/automation/iautomation.h"

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

//---------------------------------------------------------
//   isLightweightExcerpt
///   Check if excerpt data represents a lightweight excerpt
///   by looking for the <lightweight>true</lightweight> element
//---------------------------------------------------------

static bool isLightweightExcerpt(const ByteArray& excerptData)
{
    XmlReader xml(excerptData);
    while (xml.readNextStartElement()) {
        if (xml.name() == "museScore") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "Score") {
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "lightweight") {
                            return xml.readText().toInt() != 0;
                        } else {
                            xml.skipCurrentElement();
                        }
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }
    return false;
}

//---------------------------------------------------------
//   readLightweightExcerpt
///   Read a lightweight excerpt that has no full excerptScore
///   Restores name, parts references, and tracks mapping
//---------------------------------------------------------

static Excerpt* readLightweightExcerpt(MasterScore* masterScore, const ByteArray& excerptData, const String& fileName)
{
    Excerpt* excerpt = new Excerpt(masterScore);
    excerpt->setFileName(fileName);
    TracksMap tracksMap;

    XmlReader xml(excerptData);
    while (xml.readNextStartElement()) {
        if (xml.name() == "museScore") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "Score") {
                    while (xml.readNextStartElement()) {
                        const AsciiStringView tag = xml.name();
                        if (tag == "lightweight") {
                            xml.readText();  // consume the value
                        } else if (tag == "name") {
                            excerpt->setName(xml.readText(), false);
                        } else if (tag == "Tracklist") {
                            track_idx_t sTrack = static_cast<track_idx_t>(xml.intAttribute("sTrack", -1));
                            track_idx_t dstTrack = static_cast<track_idx_t>(xml.intAttribute("dstTrack", -1));
                            if (sTrack != muse::nidx && dstTrack != muse::nidx) {
                                tracksMap.insert({ sTrack, dstTrack });
                            }
                            xml.readNext();
                        } else if (tag == "initialPartId") {
                            excerpt->setInitialPartId(ID(xml.readText().toStdString()));
                        } else if (tag == "Part") {
                            ID partId(static_cast<uint64_t>(xml.intAttribute("id", 0)));
                            // Find the part in master score by ID
                            for (Part* part : masterScore->parts()) {
                                if (part->id() == partId) {
                                    excerpt->parts().push_back(part);
                                    break;
                                }
                            }
                            xml.readNext();
                        } else {
                            xml.unknown();
                        }
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    if (!tracksMap.empty()) {
        excerpt->setTracksMapping(tracksMap);
    }

    return excerpt;
}

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

    if (mscReader.isContainer()) {
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

                bool custom = style.styleV(Sid::chordStyle).value<ChordStylePreset>() == ChordStylePreset::CUSTOM;
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
                    imageStore.add(name.toStdString(), mscReader.readImageFile(name));
                }
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
    if (ret && masterScore->mscVersion() >= 400 && mscReader.isContainer()) {
        std::vector<String> excerptFileNames = mscReader.excerptFileNames();
        for (const String& excerptFileName : excerptFileNames) {
            ByteArray excerptData = mscReader.readExcerptFile(excerptFileName);

            // Check if this is a lightweight excerpt (potential excerpt without full score)
            if (isLightweightExcerpt(excerptData)) {
                Excerpt* ex = readLightweightExcerpt(masterScore, excerptData, excerptFileName);
                masterScore->addLightweightExcerpt(ex);
                continue;
            }

            // Regular excerpt with full score
            Score* partScore = masterScore->createScore();

            compat::ReadStyleHook::setupDefaultStyle(partScore);

            Excerpt* ex = new Excerpt(masterScore);
            ex->setExcerptScore(partScore);
            ex->setFileName(excerptFileName);

            ByteArray excerptStyleData = mscReader.readExcerptStyleFile(excerptFileName);
            Buffer excerptStyleBuf(&excerptStyleData);
            excerptStyleBuf.open(IODevice::ReadOnly);
            partScore->style().read(&excerptStyleBuf);

            XmlReader xml(excerptData);
            xml.setDocName(excerptFileName);

            ReadInOutData partReadInData;
            partReadInData.links = inOut->links;

            RetVal<IReaderPtr> reader = makeReader(masterScore->mscVersion(), ignoreVersionError);
            if (!reader.ret) {
                ret = reader.ret;
                break;
            }

            ret = reader.val->readScoreFile(partScore, xml, &partReadInData);
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

    // Read automation
    {
        if (masterScore->automation()) {
            ByteArray ba = mscReader.readAutomationJsonFile();
            masterScore->automation()->read(ba);
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
            if (styleHook && score->mscVersion() < 400) {
                styleHook->setupDefaultStyle();
            }

            if (score->mscVersion() >= 400) {
                //! NOTE: make sure we have a chord list
                //! Load the default chord list otherwise
                score->checkChordList();
            }

            Ret ret = reader.val->readScoreFile(score, e, out);

            score->setExcerptsChanged(false);

            return ret;
        } else {
            e.unknown();
        }
    }

    return make_ret(Err::FileCriticallyCorrupted, e.errorString());
}
