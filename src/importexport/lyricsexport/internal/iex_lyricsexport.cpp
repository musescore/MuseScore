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

#include "io/file.h"

#include <QBuffer>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/lyrics.h"

#include "iex_lyricsexport.h"

using namespace mu::engraving;
using namespace muse::io;

namespace mu::iex::lrcexport {
// Interface implementation
std::vector<project::INotationWriter::UnitType> LRCWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

//
// LRCWriter::supportsUnitType
//

bool LRCWriter::supportsUnitType(UnitType ut) const { return ut == UnitType::PER_PART; }

//
// LRCWriter::write
//

muse::Ret LRCWriter::write(notation::INotationPtr notation, muse::io::IODevice& device, const Options&)
{
    Score* score = notation->elements()->msScore();
    bool enhancedLrc = configuration()->lrcUseEnhancedFormat();

    return write(score, &device, enhancedLrc);
}

//
// LRCWriter::exportLrc
//

bool LRCWriter::exportLrc(mu::engraving::Score* score, muse::io::IODevice* device, bool enhancedLrc)
{
    write(score, device, enhancedLrc);
    return true;
}

//
// LRCWriter::writeScore
//

bool LRCWriter::writeScore(mu::engraving::Score* score, const muse::io::path_t& path, bool enhancedLrc)
{
    File f(path);
    if (!f.open(IODevice::WriteOnly)) {
        return false;
    }

    bool res = exportLrc(score, &f, enhancedLrc) && !f.hasError();
    f.close();

    return res;
}

//
// LRCWriter::writeList
//

muse::Ret LRCWriter::writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&)
{
    return muse::Ret(muse::Ret::Code::NotSupported);
}

//
// LRCWriter::writeMetadata
//

void LRCWriter::writeMetadata(muse::io::IODevice* device, const engraving::Score* score) const
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
        device->write(metadata.toUtf8());
    }
}

//
// LRCWriter::write
//

muse::Ret LRCWriter::write(mu::engraving::Score* score, muse::io::IODevice* device, bool enhancedLrc)
{
    writeMetadata(device, score);

    const auto lyrics = collectLyrics(score);

    // Write lyrics
    for (auto it = lyrics.constBegin(); it != lyrics.constEnd(); ++it) {
        if (enhancedLrc) {
            // As there should only be words we replace spaces by "-"
            QString lyricsText = it.value();
            lyricsText.replace(QRegularExpression("\\s"), "-");
            lyricsText.replace(QChar(0x00A0), QChar('-'));

            device->write(QString("[%1] <%1> %2\n").arg(formatTimestamp(it.key()), lyricsText).toUtf8());
        } else {
            device->write(QString("[%1]%2\n").arg(formatTimestamp(it.key()), it.value()).toUtf8());
        }
    }

    return muse::Ret(muse::Ret::Code::Ok);
}

//
// LRCWriter::collectLyrics
//

QMap<qreal, QString> LRCWriter::collectLyrics(const mu::engraving::Score* score)
{
    QMap<qreal, QString> lyrics;
    const RepeatList& repeats = score->repeatList();

    staff_idx_t lyricsStaff;
    voice_idx_t lyricsVoice;
    int lyricNumber;

    findStaffVoiceAndLyricToExport(score, lyricsStaff, lyricsVoice, lyricNumber);

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

                        if ((lyricsStaff == e->staffIdx()) && (lyricsVoice == e->voice()) && (lyricNumber == l->subtype())) {
                            const qreal time = score->utick2utime(l->tick().ticks() + tickOffset) * 1000;
                            lyrics.insert(time, l->plainText());
                        }
                    }
                }
            }
        }
    }
    return lyrics;
}

//
// LRCWriter::formatTimestamp
//

QString LRCWriter::formatTimestamp(qreal ms) const
{
    const int totalSec = static_cast<int>(ms / 1000);
    return QString("%1:%2.%3")
           .arg(totalSec / 60, 2, 10, QLatin1Char('0'))
           .arg(totalSec % 60, 2, 10, QLatin1Char('0'))
           .arg(static_cast<int>(ms) % 1000 / 10, 2, 10, QLatin1Char('0'));
}

//
// LRCWriter::findStaffVoiceAndLyricToExport
//

void LRCWriter::findStaffVoiceAndLyricToExport(const mu::engraving::Score* score, mu::engraving::staff_idx_t& staff,
                                               mu::engraving::voice_idx_t& voice, int& lyricNumber)
{
    bool lyricsFound = false;
    staff = 0;
    voice = 0;
    lyricNumber = 0;

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
                        } else {
                            if (!lyricsFound) {
                                lyricsFound = true;
                                staff = e->staffIdx();
                                voice = e->voice();
                                lyricNumber = l->subtype();
                            } else {
                                if (staff > e->staffIdx()) {
                                    staff = e->staffIdx();
                                    voice = e->voice();
                                    lyricNumber = l->subtype();
                                } else if (staff == e->staffIdx()) {
                                    if (voice > e->voice()) {
                                        voice = e->voice();
                                        lyricNumber = l->subtype();
                                    } else if (voice == e->voice()) {
                                        lyricNumber = min(lyricNumber, l->subtype());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
} // namespace mu::iex::lrcexport
