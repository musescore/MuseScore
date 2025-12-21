/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "lrcwriter.h"

#include <QBuffer>

#include "io/file.h"
#include "types/ret.h"

#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"

using namespace Qt::Literals;

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::project;
using namespace mu::iex::lrcexport;

std::vector<INotationWriter::UnitType> LRCWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool LRCWriter::supportsUnitType(UnitType ut) const { return ut == UnitType::PER_PART; }

muse::Ret LRCWriter::write(notation::INotationPtr notation, muse::io::IODevice& device, const Options&)
{
    Score* score = notation->elements()->msScore();
    bool enhancedLrc = configuration()->lrcUseEnhancedFormat();

    return doWrite(score, &device, enhancedLrc);
}

bool LRCWriter::writeScore(mu::engraving::Score* score, const muse::io::path_t& path, bool enhancedLrc)
{
    File f(path);
    if (!f.open(IODevice::WriteOnly)) {
        return false;
    }

    bool res = doWrite(score, &f, enhancedLrc) && !f.hasError();
    f.close();

    return res;
}

muse::Ret LRCWriter::writeList(const notation::INotationPtrList&, muse::io::IODevice&, const Options&)
{
    return make_ret(Ret::Code::NotSupported);
}

void LRCWriter::writeMetadata(muse::io::IODevice* device, const engraving::Score* score) const
{
    QString metadata;

    // Title
    const QString title = score->metaTag(u"workTitle").toQString();
    if (!title.isEmpty()) {
        metadata += u"[ti:%1]\n"_s.arg(title);
    }

    // Composer/Artist
    const QString artist = score->metaTag(u"composer").toQString();
    if (!artist.isEmpty()) {
        metadata += u"[ar:%1]\n"_s.arg(artist);
    }

    if (!metadata.isEmpty()) {
        device->write(metadata.toUtf8());
    }
}

muse::Ret LRCWriter::doWrite(mu::engraving::Score* score, muse::io::IODevice* device, bool enhancedLrc)
{
    writeMetadata(device, score);

    const auto lyrics = collectLyrics(score);

    // Write lyrics
    for (const auto& [timestamp, text] : lyrics) {
        if (enhancedLrc) {
            // As there should only be words we replace spaces by "-"
            QString lyricsText = text;
            lyricsText.replace(QRegularExpression("\\s"), "-");
            lyricsText.replace(u'\u00A0', u'-');

            device->write(QString("[%1] <%1> %2\n").arg(formatTimestamp(timestamp), lyricsText).toUtf8());
        } else {
            device->write(QString("[%1]%2\n").arg(formatTimestamp(timestamp), text).toUtf8());
        }
    }

    return make_ok();
}

std::map<double, QString> LRCWriter::collectLyrics(const mu::engraving::Score* score)
{
    std::map<double, QString> lyrics;
    const RepeatList& repeats = score->repeatList();

    track_idx_t trackNumber;
    int lyricNumber;

    findTrackAndLyricToExport(score, trackNumber, lyricNumber);

    for (const RepeatSegment* rs : repeats) {
        const int tickOffset = rs->utick - rs->tick;

        for (const MeasureBase* mb : rs->measureList()) {
            if (!mb->isMeasure()) {
                continue;
            }

            for (const Segment* seg = toMeasure(mb)->first(); seg; seg = seg->next()) {
                if (!seg->isChordRestType()) {
                    continue;
                }

                EngravingItem* e = seg->element(trackNumber);
                if (!e || !e->isChordRest()) {
                    continue;
                }

                for (const Lyrics* l : toChordRest(e)->lyrics()) {
                    if (l->plainText().isEmpty()) {
                        continue;
                    }

                    if (lyricNumber == l->subtype()) {
                        const double time = score->utick2utime(l->tick().ticks() + tickOffset) * 1000;
                        lyrics.insert_or_assign(time, l->plainText());
                    }
                }
            }
        }
    }
    return lyrics;
}

QString LRCWriter::formatTimestamp(double ms) const
{
    const int totalSec = static_cast<int>(ms / 1000);
    return u"%1:%2.%3"_s
           .arg(totalSec / 60, 2, 10, QLatin1Char('0'))
           .arg(totalSec % 60, 2, 10, QLatin1Char('0'))
           .arg(static_cast<int>(ms) % 1000 / 10, 2, 10, QLatin1Char('0'));
}

void LRCWriter::findTrackAndLyricToExport(const engraving::Score* score, mu::engraving::track_idx_t& trackNumber, int& lyricNumber)
{
    bool lyricsFound = false;
    trackNumber = 0;
    lyricNumber = 0;

    const RepeatList& repeats = score->repeatList();

    for (const RepeatSegment* rs : repeats) {
        for (const MeasureBase* mb : rs->measureList()) {
            if (!mb->isMeasure()) {
                continue;
            }

            for (const Segment* seg = toMeasure(mb)->first(); seg; seg = seg->next()) {
                if (!seg->isChordRestType()) {
                    continue;
                }

                for (const EngravingItem* e : seg->elist()) {
                    if (!e || !e->isChordRest()) {
                        continue;
                    }

                    for (const Lyrics* l : toChordRest(e)->lyrics()) {
                        // if (l->text().empty())
                        if (l->plainText().isEmpty()) {
                            continue;
                        }

                        if (!lyricsFound) {
                            lyricsFound = true;
                            trackNumber = e->track();
                            lyricNumber = l->subtype();
                            continue;
                        }

                        // We check if we have a better option
                        if (trackNumber > e->track()) {
                            trackNumber = e->track();
                            lyricNumber = l->subtype();
                        } else if (trackNumber == e->track()) {
                            lyricNumber = std::min(lyricNumber, l->subtype());
                        }
                    }
                    // If we have already chosen the lowest/prioritized option we can return (no better option available)
                    if (lyricsFound && (trackNumber == 0) && (lyricNumber == 0)) {
                        return;
                    }
                }
            }
        }
    }
}
