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
#include "scorereader.h"

#include <QBuffer>

#include "compat/readstyle.h"
#include "compat/read114.h"
#include "compat/read206.h"
#include "compat/read302.h"
#include "read400.h"

#include "../libmscore/excerpt.h"
#include "../libmscore/imageStore.h"
#include "../libmscore/audio.h"
#include "../libmscore/revisions.h"

#include "log.h"

using namespace mu::engraving;
using namespace Ms;

Err ScoreReader::loadMscz(Ms::MasterScore* score, const mu::engraving::MscReader& mscReader, bool ignoreVersionError)
{
    using namespace mu::engraving;

    IF_ASSERT_FAILED(mscReader.isOpened()) {
        return Err::FileOpenError;
    }

    ScoreLoad sl;
    score->fileInfo()->setFile(mscReader.params().filePath);

    Err retval;
    // Read style
    {
        QByteArray styleData = mscReader.readStyleFile();
        QBuffer buf(&styleData);
        buf.open(QIODevice::ReadOnly);
        score->style().read(&buf);
    }

    // Read score
    {
        QByteArray scoreData = mscReader.readScoreFile();
        QString completeBaseName = score->fileInfo()->completeBaseName();

        compat::ReadStyleHook styleHook(score, scoreData, completeBaseName);

        XmlReader xml(scoreData);
        xml.setDocName(completeBaseName);
        ReadContext ctx(score);
        ctx.setIgnoreVersionError(ignoreVersionError);
        retval = read(score, xml, ctx, &styleHook);
    }

    // Read excerpts
    if (score->mscVersion() >= 400) {
        std::vector<QString> excerptNames = mscReader.excerptNames();
        for (const QString& excerptName : excerptNames) {
            Score* partScore = score->createScore();

            compat::ReadStyleHook::setupDefaultStyle(partScore);

            Excerpt* ex = new Excerpt(score);
            ex->setPartScore(partScore);

            QByteArray excerptStyleData = mscReader.readExcerptStyleFile(excerptName);
            QBuffer excerptStyleBuf(&excerptStyleData);
            excerptStyleBuf.open(QIODevice::ReadOnly);
            partScore->style().read(&excerptStyleBuf);

            QByteArray excerptData = mscReader.readExcerptFile(excerptName);
            XmlReader xml(excerptData);
            xml.setDocName(excerptName);
            ReadContext ctx(score);
            Read400::read400(partScore, xml, ctx);

            partScore->linkMeasures(score);
            ex->setTracks(xml.tracks());

            ex->setTitle(excerptName);

            score->addExcerpt(ex);
        }
    }

    // Read ChordList
    {
        QByteArray styleData = mscReader.readChordListFile();
        QBuffer buf(&styleData);
        buf.open(QIODevice::ReadOnly);
        score->chordList()->read(&buf);
    }

    // Read images
    {
        if (!MScore::noImages) {
            std::vector<QString> images = mscReader.imageFileNames();
            for (const QString& name : images) {
                imageStore.add(name, mscReader.readImageFile(name));
            }
        }
    }

    //  Read audio
    {
        if (score->audio()) {
            QByteArray dbuf1 = mscReader.readAudioFile();
            score->audio()->setData(dbuf1);
        }
    }

    return retval;
}

Err ScoreReader::read(MasterScore* score, XmlReader& e, ReadContext& ctx, compat::ReadStyleHook* styleHook)
{
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            const QString& version = e.attribute("version");
            QStringList sl = version.split('.');
            score->setMscVersion(sl[0].toInt() * 100 + sl[1].toInt());

            if (!ctx.ignoreVersionError()) {
                if (score->mscVersion() > MSCVERSION) {
                    return Err::FileTooNew;
                }
                if (score->mscVersion() < 114) {
                    return Err::FileTooOld;
                }
                if (score->mscVersion() == 300) {
                    return Err::FileOld300Format;
                }
            }

            //! NOTE We need to achieve that the default style corresponds to the version in which the score is created.
            //! The values that the user changed will be written on over (only they are stored in the `mscz` file)
            //! For version 4.0 (400), this does not need to be done,
            //! because starting from version 4.0 the entire style is stored in a file,
            //! respectively, the entire style will be loaded, which was when the score was created.
            if (styleHook && score->mscVersion() < 400) {
                styleHook->setupDefaultStyle();
            }

            Err err;
            if (score->mscVersion() <= 114) {
                Score::FileError error = compat::Read114::read114(score, e, ctx);
                err = scoreFileErrorToErr(error);
            } else if (score->mscVersion() <= 207) {
                Score::FileError error = compat::Read206::read206(score, e, ctx);
                err = scoreFileErrorToErr(error);
            } else if (score->mscVersion() < 400 || MScore::testMode) {
                Score::FileError error = compat::Read302::read302(score, e, ctx);
                err = scoreFileErrorToErr(error);
            } else {
                err = doRead(score, e, ctx);
            }

            score->setCreated(false);
            score->setExcerptsChanged(false);
            return err;
        } else {
            e.unknown();
        }
    }
    return Err::FileCorrupted;
}

Err ScoreReader::doRead(MasterScore* score, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "programVersion") {
            score->setMscoreVersion(e.readElementText());
        } else if (tag == "programRevision") {
            score->setMscoreRevision(e.readIntHex());
        } else if (tag == "Score") {
            if (!Read400::readScore400(score, e, ctx)) {
                if (e.error() == QXmlStreamReader::CustomError) {
                    return Err::FileCriticalCorrupted;
                }
                return Err::FileBadFormat;
            }
        } else if (tag == "Revision") {
            Revision* revision = new Revision;
            revision->read(e);
            score->revisions()->add(revision);
        }
    }

    return Err::NoError;
}
