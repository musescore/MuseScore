/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Map Encore tempo text and BPM values to MuseScore tempo semantics.

#include "mappers.h"

using namespace mu::engraving;

namespace mu::iex::enc {
double encTextToTempoBps(const QString& text)
{
    const QString t = text.trimmed().toLower();
    if (t.isEmpty()) {
        return -1.0;
    }
    // Italian tempo terms; BPM values match MuseScore's tempo palette (palettecreator.cpp) for consistent playback.
    struct Entry {
        const char* word;
        double bps;
    };
    static const Entry kAbsolute[] = {
        { "grave",        35.0 / 60.0 },
        { "largo",        50.0 / 60.0 },
        { "lento",        52.5 / 60.0 },
        { "larghetto",    63.0 / 60.0 },
        { "adagio",       71.0 / 60.0 },
        { "andante",      92.0 / 60.0 },
        { "andantino",    94.0 / 60.0 },
        { "moderato",    114.0 / 60.0 },
        { "allegretto",  116.0 / 60.0 },
        { "allegro",     144.0 / 60.0 },
        { "vivace",      172.0 / 60.0 },
        { "presto",      187.0 / 60.0 },
        { "prestissimo", 200.0 / 60.0 },
    };
    for (const Entry& e : kAbsolute) {
        if (t == QString::fromLatin1(e.word)) {
            return e.bps;
        }
    }
    // Relative markings: become TempoText with BPS=0 (falls back to previous tempo).
    static const char* kRelative[] = {
        "a tempo",
        "tempo i",
        "tempo 1",
        "tempo primo",
    };
    for (const char* r : kRelative) {
        if (t == QString::fromLatin1(r)) {
            return 0.0;
        }
    }
    return -1.0;
}
} // namespace mu::iex::enc
