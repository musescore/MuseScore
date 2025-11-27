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

#include "iex_lyricsexport.h"

#include <QBuffer>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/lyrics.h"

using namespace mu::engraving;

namespace mu::iex::lrcexport {
// Interface implementation
std::vector<project::INotationWriter::UnitType> LRCWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool LRCWriter::supportsUnitType(UnitType ut) const { return ut == UnitType::PER_PART; }

muse::Ret LRCWriter::write(notation::INotationPtr notation, muse::io::IODevice& device, const Options&)
{
    Score* score = notation->elements()->msScore();
    QByteArray data;
    QBuffer buffer(&data);

    /***********
    *
    * PENDING....
    Is there any advantage to writing to a buffer first, and then writing that buffer to the device? It would seem more efficient to me to write to the device directly.

    ******/

    buffer.open(QIODevice::WriteOnly);

    writeMetadata(buffer, score);

    const auto lyrics = collectLyrics(score);

    // Write lyrics
    for (auto it = lyrics.constBegin(); it != lyrics.constEnd(); ++it) {
        buffer.write(QString("[%1]%2\n").arg(formatTimestamp(it.key()), it.value()).toUtf8());
    }

    device.write(data);
    return muse::Ret(muse::Ret::Code::Ok);
}

muse::Ret LRCWriter::writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&)
{
    return muse::Ret(muse::Ret::Code::NotSupported);
}

void LRCWriter::writeMetadata(QIODevice& device, const engraving::Score* score) const
{
    QString metadata;

    // Title
    QString title = QString::fromStdString(score->metaTag(muse::String("workTitle")).toStdString());
    if (!title.isEmpty()) {
        metadata += QString("[ti:%1]\n").arg(title);
    }

    // Composer/Artist
    QString artist = QString::fromStdString(score->metaTag(muse::String("composer")).toStdString());
    if (!artist.isEmpty()) {
        metadata += QString("[ar:%1]\n").arg(artist);
    }

    if (!metadata.isEmpty()) {
        device.write(metadata.toUtf8());
    }
}

// Core lyric collection (simplified)
QMap<qreal, QString> LRCWriter::collectLyrics(const Score* score) const
{
    QMap<qreal, QString> lyrics;
    const RepeatList& repeats = score->repeatList();

    for (const RepeatSegment* rs : repeats) {
        const int tickOffset = rs->utick - rs->tick;

        for (const MeasureBase* mb = rs->firstMeasure(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }

            for (Segment* seg = toMeasure(mb)->first(); seg; seg = seg->next()) {
                if (!seg->isChordRestType()) {
                    continue;
                }

                for (EngravingItem* e : seg->elist()) {
                    if (!e || !e->isChordRest()) {
                        continue;
                    }

                    for (Lyrics* l : toChordRest(e)->lyrics()) {
                        // if (l->text().empty())
                        if (l->plainText().isEmpty()) {
                            continue;
                        }

                        const qreal time = score->utick2utime(l->tick().ticks() + tickOffset) * 1000;
                        lyrics.insert(time, l->plainText());
                    }
                }
            }
        }
    }
    return lyrics;
}

QString LRCWriter::formatTimestamp(qreal ms) const
{
    const int totalSec = static_cast<int>(ms / 1000);
    return QString("%1:%2.%3")
           .arg(totalSec / 60, 2, 10, QLatin1Char('0'))
           .arg(totalSec % 60, 2, 10, QLatin1Char('0'))
           .arg(static_cast<int>(ms) % 1000 / 10, 2, 10, QLatin1Char('0'));
}
} // namespace mu::iex::lrcexport
