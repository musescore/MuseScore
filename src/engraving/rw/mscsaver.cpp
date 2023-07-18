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
#include "mscsaver.h"

#include "global/io/buffer.h"

#include "libmscore/masterscore.h"
#include "libmscore/excerpt.h"
#include "libmscore/imageStore.h"
#include "libmscore/audio.h"

#include "rwregister.h"
#include "inoutdata.h"

#include "log.h"

using namespace mu::io;
using namespace mu::engraving;
using namespace mu::engraving::rw;

bool MscSaver::writeMscz(MasterScore* score, MscWriter& mscWriter, bool onlySelection, bool doCreateThumbnail)
{
    IF_ASSERT_FAILED(mscWriter.isOpened()) {
        return false;
    }

    // Write style of MasterScore
    {
        //! NOTE The style is writing to a separate file only for the master score.
        //! At the moment, the style for the parts is still writing to the score file.
        ByteArray styleData;
        Buffer styleBuf(&styleData);
        styleBuf.open(IODevice::WriteOnly);
        score->style().write(&styleBuf);
        mscWriter.writeStyleFile(styleData);
    }

    WriteInOutData masterWriteOutData;

    // Write MasterScore
    {
        ByteArray scoreData;
        Buffer scoreBuf(&scoreData);
        scoreBuf.open(IODevice::ReadWrite);

        RWRegister::writer()->writeScore(score, &scoreBuf, onlySelection, &masterWriteOutData);

        mscWriter.writeScoreFile(scoreData);
    }

    // Write Excerpts
    {
        if (!onlySelection) {
            for (const Excerpt* excerpt : score->excerpts()) {
                Score* partScore = excerpt->excerptScore();
                if (partScore != score) {
                    // Write excerpt style
                    {
                        ByteArray excerptStyleData;
                        Buffer styleStyleBuf(&excerptStyleData);
                        styleStyleBuf.open(IODevice::WriteOnly);
                        partScore->style().write(&styleStyleBuf);

                        mscWriter.addExcerptStyleFile(excerpt->name(), excerptStyleData);
                    }

                    // Write excerpt
                    {
                        ByteArray excerptData;
                        Buffer excerptBuf(&excerptData);
                        excerptBuf.open(IODevice::ReadWrite);

                        RWRegister::writer()->writeScore(excerpt->excerptScore(), &excerptBuf, onlySelection, &masterWriteOutData);

                        mscWriter.addExcerptFile(excerpt->name(), excerptData);
                    }
                }
            }
        }
    }

    // Write ChordList
    {
        ChordList* chordList = score->chordList();
        if (chordList->customChordList() && !chordList->empty()) {
            ByteArray chlData;
            Buffer chlBuf(&chlData);
            chlBuf.open(IODevice::WriteOnly);
            chordList->write(&chlBuf);
            mscWriter.writeChordListFile(chlData);
        }
    }

    // Write images
    {
        for (ImageStoreItem* ip : imageStore) {
            if (!ip->isUsed(score)) {
                continue;
            }
            ByteArray data = ip->buffer();
            mscWriter.addImageFile(ip->hashName(), data);
        }
    }

    // Write thumbnail
    {
        if (doCreateThumbnail && !score->pages().empty()) {
            auto pixmap = score->createThumbnail();

            ByteArray ba;
            Buffer b(&ba);
            b.open(IODevice::WriteOnly);
            imageProvider()->saveAsPng(pixmap, &b);
            mscWriter.writeThumbnailFile(ba);
        }
    }

    // Write audio
    {
        if (score->audio()) {
            mscWriter.writeAudioFile(score->audio()->data());
        }
    }

    return true;
}

bool MscSaver::exportPart(Score* partScore, MscWriter& mscWriter)
{
    // Write excerpt style as main
    {
        ByteArray excerptStyleData;
        Buffer styleStyleBuf(&excerptStyleData);
        styleStyleBuf.open(IODevice::WriteOnly);
        partScore->style().write(&styleStyleBuf);

        mscWriter.writeStyleFile(excerptStyleData);
    }

    // Write excerpt as main score
    {
        ByteArray excerptData;
        Buffer excerptBuf(&excerptData);
        excerptBuf.open(IODevice::WriteOnly);

        rw::RWRegister::writer()->writeScore(partScore, &excerptBuf, false);

        mscWriter.writeScoreFile(excerptData);
    }

    // Write thumbnail
    {
        if (!partScore->pages().empty()) {
            auto pixmap = partScore->createThumbnail();

            ByteArray ba;
            Buffer b(&ba);
            b.open(IODevice::WriteOnly);
            imageProvider()->saveAsPng(pixmap, &b);
            mscWriter.writeThumbnailFile(ba);
        }
    }

    return true;
}
