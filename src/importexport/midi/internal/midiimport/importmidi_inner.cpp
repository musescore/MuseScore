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
#include "importmidi_inner.h"

#include <QTextCodec>

#include "importmidi_operations.h"
#include "importmidi_chord.h"

#include "engraving/dom/durationtype.h"
#include "engraving/dom/sig.h"

namespace mu::iex::midi {
MTrack::MTrack(const MTrack& other)
    : program(other.program)
    , staff(other.staff)
    , mtrack(other.mtrack)
    , name(other.name)
    , hasKey(other.hasKey)
    , indexOfOperation(other.indexOfOperation)
    , division(other.division)
    , isDivisionInTps(other.isDivisionInTps)
    , hadInitialNotes(other.hadInitialNotes)
    , volumes(other.volumes)
    , chords(other.chords)
{
    updateTupletsFromChords();
}

MTrack& MTrack::operator=(MTrack other)
{
    std::swap(program, other.program);
    std::swap(staff, other.staff);
    std::swap(mtrack, other.mtrack);
    std::swap(name, other.name);
    std::swap(hasKey, other.hasKey);
    std::swap(indexOfOperation, other.indexOfOperation);
    std::swap(division, other.division);
    std::swap(isDivisionInTps, other.isDivisionInTps);
    std::swap(hadInitialNotes, other.hadInitialNotes);
    std::swap(volumes, other.volumes);
    std::swap(chords, other.chords);
    updateTupletsFromChords();

    return *this;
}

// chords of the MTrack are considered to be created/copied already;
// tuplets here are extracted and inserted to the tuplet map of the MTrack
// from tuplet iterators in chords/notes
void MTrack::updateTupletsFromChords()
{
    for (auto& chord: chords) {
        if (chord.second.isInTuplet) {
            updateTuplet(chord.second.tuplet);
        }
        for (auto& note: chord.second.notes) {
            if (note.isInTuplet) {
                updateTuplet(note.tuplet);
            }
        }
    }
}

void MTrack::updateTuplet(
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator& tupletIt)
{
    auto foundTuplet = tuplets.end();
    const auto foundTupletsRange = tuplets.equal_range(tupletIt->first);
    for (auto it = foundTupletsRange.first; it != foundTupletsRange.second; ++it) {
        if (it->second.voice == tupletIt->second.voice) {
            foundTuplet = it;
            break;
        }
    }
    if (foundTuplet != tuplets.end()) {
        tupletIt = foundTuplet;
    } else {
        tupletIt = tuplets.insert({ tupletIt->first, tupletIt->second });
    }
}

namespace Meter {
ReducedFraction userTimeSigToFraction(
    MidiOperations::TimeSigNumerator timeSigNumerator,
    MidiOperations::TimeSigDenominator timeSigDenominator)
{
    int numerator = 4;
    int denominator = 4;

    switch (timeSigNumerator) {
    case MidiOperations::TimeSigNumerator::_2:
        numerator = 2;
        break;
    case MidiOperations::TimeSigNumerator::_3:
        numerator = 3;
        break;
    case MidiOperations::TimeSigNumerator::_4:
        numerator = 4;
        break;
    case MidiOperations::TimeSigNumerator::_5:
        numerator = 5;
        break;
    case MidiOperations::TimeSigNumerator::_6:
        numerator = 6;
        break;
    case MidiOperations::TimeSigNumerator::_7:
        numerator = 7;
        break;
    case MidiOperations::TimeSigNumerator::_9:
        numerator = 9;
        break;
    case MidiOperations::TimeSigNumerator::_12:
        numerator = 12;
        break;
    case MidiOperations::TimeSigNumerator::_15:
        numerator = 15;
        break;
    case MidiOperations::TimeSigNumerator::_21:
        numerator = 21;
        break;
    default:
        break;
    }

    switch (timeSigDenominator) {
    case MidiOperations::TimeSigDenominator::_2:
        denominator = 2;
        break;
    case MidiOperations::TimeSigDenominator::_4:
        denominator = 4;
        break;
    case MidiOperations::TimeSigDenominator::_8:
        denominator = 8;
        break;
    case MidiOperations::TimeSigDenominator::_16:
        denominator = 16;
        break;
    case MidiOperations::TimeSigDenominator::_32:
        denominator = 32;
        break;
    default:
        break;
    }

    return ReducedFraction(numerator, denominator);
}

MidiOperations::TimeSigNumerator fractionNumeratorToUserValue(int n)
{
    MidiOperations::TimeSigNumerator numerator = MidiOperations::TimeSigNumerator::_4;

    if (n == 2) {
        numerator = MidiOperations::TimeSigNumerator::_2;
    } else if (n == 3) {
        numerator = MidiOperations::TimeSigNumerator::_3;
    } else if (n == 4) {
        numerator = MidiOperations::TimeSigNumerator::_4;
    } else if (n == 5) {
        numerator = MidiOperations::TimeSigNumerator::_5;
    } else if (n == 6) {
        numerator = MidiOperations::TimeSigNumerator::_6;
    } else if (n == 7) {
        numerator = MidiOperations::TimeSigNumerator::_7;
    } else if (n == 9) {
        numerator = MidiOperations::TimeSigNumerator::_9;
    } else if (n == 12) {
        numerator = MidiOperations::TimeSigNumerator::_12;
    } else if (n == 15) {
        numerator = MidiOperations::TimeSigNumerator::_15;
    } else if (n == 21) {
        numerator = MidiOperations::TimeSigNumerator::_21;
    } else {
        Q_ASSERT_X(false, "Meter::fractionNumeratorToUserValue", "Unknown numerator");
    }

    return numerator;
}

MidiOperations::TimeSigDenominator fractionDenominatorToUserValue(int z)
{
    MidiOperations::TimeSigDenominator denominator = MidiOperations::TimeSigDenominator::_4;

    if (z == 2) {
        denominator = MidiOperations::TimeSigDenominator::_2;
    } else if (z == 4) {
        denominator = MidiOperations::TimeSigDenominator::_4;
    } else if (z == 8) {
        denominator = MidiOperations::TimeSigDenominator::_8;
    } else if (z == 16) {
        denominator = MidiOperations::TimeSigDenominator::_16;
    } else if (z == 32) {
        denominator = MidiOperations::TimeSigDenominator::_32;
    } else {
        Q_ASSERT_X(false, "Meter::fractionDenominatorToUserValue", "Unknown denominator");
    }

    return denominator;
}
} // namespace Meter

namespace MidiTuplet {
bool haveIntersection(const std::pair<ReducedFraction, ReducedFraction>& interval1,
                      const std::pair<ReducedFraction, ReducedFraction>& interval2,
                      bool strictComparison)
{
    if (strictComparison) {
        return interval1.second > interval2.first && interval1.first < interval2.second;
    }
    return interval1.second >= interval2.first && interval1.first <= interval2.second;
}

bool haveIntersection(
    const std::pair<ReducedFraction, ReducedFraction>& interval,
    const std::vector<std::pair<ReducedFraction, ReducedFraction> >& intervals,
    bool strictComparison)
{
    for (const auto& i: intervals) {
        if (haveIntersection(i, interval, strictComparison)) {
            return true;
        }
    }
    return false;
}
}

namespace MidiCharset {
QString convertToCharset(const std::string& text)
{
    // charset for the current MIDI file
    QString charset = midiImportOperations.data()->charset;
    auto* codec = QTextCodec::codecForName(charset.toLatin1());
    if (codec) {
        return codec->toUnicode(text.c_str());
    } else {
        return QString::fromUtf8(text.data(), int(text.size()));
    }
}

QString defaultCharset()
{
    return "UTF-8";
}

std::string fromUchar(const uchar* text)
{
    return reinterpret_cast<const char*>(text);
}
} // namespace MidiCharset

namespace MidiBar {
ReducedFraction findBarStart(const ReducedFraction& time, const engraving::TimeSigMap* sigmap)
{
    int barIndex, beat, tick;
    sigmap->tickValues(time.ticks(), &barIndex, &beat, &tick);
    return ReducedFraction::fromTicks(sigmap->bar2tick(barIndex, 0));
}
} // namespace MidiBar
namespace MidiDuration {
double durationCount(const QList<std::pair<ReducedFraction, engraving::TDuration> >& durations)
{
    double count = durations.size();
    for (const auto& d: durations) {
        if (d.second.dots()) {
            count += 0.5;
        }
    }
    return count;
}
} // namespace MidiDuration
} // namespace mu::iex::midi
