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
#include "mscsaver.h"

#include "global/io/buffer.h"
#include "global/serialization/xmlstreamwriter.h"

#include "dom/masterscore.h"
#include "dom/excerpt.h"
#include "dom/part.h"
#include "dom/imageStore.h"
#include "dom/audio.h"

#include "engraving/automation/iautomation.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/types/constants.h"

#include "rwregister.h"
#include "inoutdata.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::engraving::rw;

//---------------------------------------------------------
//   writeLightweightExcerpt
///   Write an excerpt that has no excerptScore (potential excerpt)
///   in a lightweight XML format that preserves name, parts, and tracks
//---------------------------------------------------------

static void writeLightweightExcerpt(Excerpt* excerpt, MscWriter& mscWriter)
{
    ByteArray excerptData;
    Buffer excerptBuf(&excerptData);
    excerptBuf.open(IODevice::WriteOnly);

    {
        XmlStreamWriter xml(&excerptBuf);
        xml.startDocument();

        xml.startElement("museScore", { { "version", Constants::MSC_VERSION_STR } });
        xml.startElement("Score");

        // Mark as lightweight excerpt
        xml.element("lightweight", 1);  // Use int, not bool

        // Write name if not empty
        if (!excerpt->name().empty()) {
            xml.element("name", excerpt->name());
        }

        // Write tracks mapping
        const TracksMap& tracks = excerpt->tracksMapping();
        if (!tracks.empty()) {
            for (auto it = tracks.cbegin(); it != tracks.cend(); ++it) {
                xml.element("Tracklist", { { "sTrack", String::number(it->first) },
                                { "dstTrack", String::number(it->second) } });
            }
        }

        // Write initialPartId if set
        if (excerpt->initialPartId().isValid()) {
            xml.element("initialPartId", excerpt->initialPartId().toUint64());
        }

        // Write parts (as Part element with id attribute)
        for (const Part* part : excerpt->parts()) {
            xml.element("Part", { { "id", part->id().toUint64() } });
        }

        xml.endElement();  // Score
        xml.endElement();  // museScore
        xml.flush();
    }  // xml destructor ensures all data is flushed

    excerptBuf.close();

    mscWriter.addExcerptFile(excerpt->fileName(), excerptData);
}

bool MscSaver::writeMscz(MasterScore* score, MscWriter& mscWriter, bool createThumbnail,
                         const write::WriteContext* ctx)
{
    TRACEFUNC;

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

    WriteInOutData masterWriteOutData(score);

    if (ctx) {
        masterWriteOutData.ctx = *ctx;
    }

    // Write MasterScore
    {
        ByteArray scoreData;
        Buffer scoreBuf(&scoreData);
        scoreBuf.open(IODevice::ReadWrite);

        RWRegister::writer(score->iocContext())->writeScore(score, &scoreBuf, &masterWriteOutData);

        mscWriter.writeScoreFile(scoreData);
    }

    // Write Excerpts
    {
        if (!ctx || !ctx->shouldWriteRange()) {
            const std::vector<Excerpt*>& excerpts = score->excerpts();

            for (size_t excerptIndex = 0; excerptIndex < excerpts.size(); ++excerptIndex) {
                Excerpt* excerpt = excerpts.at(excerptIndex);

                excerpt->updateFileName(excerptIndex);

                Score* partScore = excerpt->excerptScore();

                // Lightweight excerpts have no excerptScore - write minimal XML
                if (!partScore) {
                    writeLightweightExcerpt(excerpt, mscWriter);
                    continue;
                }

                IF_ASSERT_FAILED(partScore != score) {
                    continue;
                }

                // Write excerpt style
                {
                    ByteArray excerptStyleData;
                    Buffer styleStyleBuf(&excerptStyleData);
                    styleStyleBuf.open(IODevice::WriteOnly);
                    partScore->style().write(&styleStyleBuf);

                    mscWriter.addExcerptStyleFile(excerpt->fileName(), excerptStyleData);
                }

                // Write excerpt
                {
                    ByteArray excerptData;
                    Buffer excerptBuf(&excerptData);
                    excerptBuf.open(IODevice::ReadWrite);

                    RWRegister::writer(partScore->iocContext())->writeScore(
                        excerpt->excerptScore(), &excerptBuf, &masterWriteOutData);

                    mscWriter.addExcerptFile(excerpt->fileName(), excerptData);
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
            mscWriter.addImageFile(String::fromStdString(ip->hashName()), data);
        }
    }

    // Write thumbnail
    {
        if (createThumbnail && !score->pages().empty()) {
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

    // Write automation
    {
        if (score->automation()) {
            mscWriter.writeAutomationJsonFile(score->automation()->toJson());
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

        rw::RWRegister::writer(partScore->iocContext())->writeScore(partScore, &excerptBuf);

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
