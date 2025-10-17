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

#pragma once

#include <list>
#include <vector>

#include "global/allocator.h"

#include "../types/fraction.h"
#include "../types/types.h"

namespace mu::engraving {
class EngravingItem;
class Measure;
class Tuplet;
class Segment;
class Spanner;
class ScoreRange;
class ChordRest;
class Chord;
class Note;
class Score;
class Tie;
class BarLine;
class Spacer;
class Marker;
class Volta;

//---------------------------------------------------------
//   TrackList
//---------------------------------------------------------

class TrackList : public std::vector<EngravingItem*>
{
    OBJECT_ALLOCATOR(engraving, TrackList)

public:
    typedef std::map<Chord*, Chord*> ClonedChordMap;

    TrackList(ScoreRange* r) { m_range = r; }
    ~TrackList();

    Fraction ticks() const { return m_duration; }
    ScoreRange* range() const { return m_range; }

    track_idx_t track() const { return m_track; }
    void setTrack(track_idx_t val) { m_track = val; }

    void read(const Segment* fs, const Segment* ls);
    bool write(Score*, const Fraction&) const;

    void appendGap(const Fraction&, Score* score);
    bool truncate(const Fraction&);
    void dump() const;

private:

    Tuplet* writeTuplet(Tuplet* parent, Tuplet* tuplet, Measure*& measure, Fraction& rest) const;
    void append(EngravingItem*);
    void appendTuplet(Tuplet* srcTuplet, Tuplet* dstTuplet);
    void combineTuplet(Tuplet* dst, Tuplet* src);

    Fraction m_duration;
    ScoreRange* m_range = nullptr;
    track_idx_t m_track = 0;
    ClonedChordMap m_clonedChord;
};

//---------------------------------------------------------
//   Annotation
//---------------------------------------------------------

struct Annotation {
    Fraction tick;
    EngravingItem* e = nullptr;
};

//---------------------------------------------------------
//   ScoreRange
//---------------------------------------------------------

class ScoreRange
{
public:
    ScoreRange() {}
    ~ScoreRange();
    void read(Segment* first, Segment* last, bool readSpanner = true);
    bool write(Score*, const Fraction&) const;
    Fraction ticks() const;
    Segment* first() const { return m_first; }
    Segment* last() const { return m_last; }
    void fill(const Fraction&);
    bool truncate(const Fraction&);

protected:
    std::vector<Spanner*> m_spanner;
    std::vector<Annotation> m_annotations;

private:
    struct BarLinesBackup
    {
        Fraction sPosition;
        bool formerMeasureStartOrEnd;
        BarLine* bl = nullptr;
    };
    std::vector<BarLinesBackup> m_barLines;

    void backupBarLines(Segment* first, Segment* last);
    bool insertBarLine(Measure* m, const BarLinesBackup& barLine) const;
    void restoreBarLines(Score* score, const Fraction& tick) const;
    void deleteBarLines();

    struct BreaksBackup
    {
        Fraction sPosition;
        LayoutBreakType lBreakType;
    };
    std::vector<BreaksBackup> m_breaks;

    void backupBreaks(Segment* first, Segment* last);
    void restoreBreaks(Score* score, const Fraction& tick) const;

    struct StartEndRepeatBackup
    {
        Fraction sPosition;
        bool isStartRepeat;
    };
    std::vector<StartEndRepeatBackup> m_startEndRepeats;

    void backupRepeats(Segment* first, Segment* last);
    void restoreRepeats(Score* score, const Fraction& tick) const;

    struct SpacerBackup
    {
        Fraction sPosition;
        staff_idx_t staffIdx;
        Spacer* s = nullptr;
    };

    void backupSpacers(Segment* first, Segment* last);
    void restoreSpacers(Score* score, const Fraction& tick) const;
    void deleteSpacers();

    struct JumpsMarkersBackup
    {
        Fraction sPosition;
        EngravingItem* e = nullptr;
    };

    bool endOfMeasureElement(EngravingItem* e) const;
    void backupJumpsAndMarkers(Segment* first, Segment* last);
    void restoreJumpsAndMarkers(Score* score, const Fraction& tick) const;
    void deleteJumpsAndMarkers();

    void restoreVolta(Score* score, const Fraction& tick, Volta* v) const;

    friend class TrackList;

    std::vector<TrackList*> m_tracks;
    std::vector<Tie*> m_startTies;
    std::vector<SpacerBackup> m_spacers;
    std::vector<JumpsMarkersBackup> m_jumpsMarkers;
    Segment* m_first = nullptr;
    Segment* m_last = nullptr;
};
}
