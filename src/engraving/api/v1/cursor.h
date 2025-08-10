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

#include "apistructs.h"

#include "engraving/dom/input.h"

Q_MOC_INCLUDE("engraving/api/v1/elements.h")
Q_MOC_INCLUDE("engraving/api/v1/score.h")

namespace mu::engraving {
class EngravingItem;
class InputState;
class Score;
class Chord;
class Rest;
class Note;
class Segment;
class ChordRest;
class StaffText;
class Measure;

enum class SegmentType;
}

namespace mu::engraving::apiv1 {
class EngravingItem;
class Measure;
class Segment;
class Score;
class Staff;

//---------------------------------------------------------
//   @@ Cursor
///   Cursor can be used by plugins to manipulate the score.
///   Cursor object for a score can be obtained with
///   \ref Score.newCursor method. After creating a cursor
///   it does not point to any location in a score. To define
///   its initial location use \ref rewind or \ref rewindToTick
///   methods. Alternatively, you can set its
///   \ref inputStateMode to \ref INPUT_STATE_SYNC_WITH_SCORE "Cursor.INPUT_STATE_SYNC_WITH_SCORE"
///   to make cursor location be synchronized with
///   user-visible note input state.
//---------------------------------------------------------

class Cursor : public QObject
{
    Q_OBJECT
    /// Current track
    Q_PROPERTY(int track READ track WRITE setTrack)
    /// Current staff number (#track / 4)
    Q_PROPERTY(int staffIdx READ staffIdx WRITE setStaffIdx)
    /// Current \ref Staff the cursor is on.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Staff * staff READ staff WRITE setStaff)
    /// Current voice (#track % 4)
    Q_PROPERTY(int voice READ voice WRITE setVoice)
    /// Segment type filter, a bitmask from
    /// PluginAPI::PluginAPI::Segment values.
    /// Determines which segments this cursor will move to
    /// on next() and nextMeasure() operations. The default
    /// value is mu::engraving::SegmentType::ChordRest so only segments
    /// containing chords and rests are handled by default.
    Q_PROPERTY(int filter READ filter WRITE setFilter)

    /// MIDI tick position, read only
    /// As of MuseScore 4.6, this property is deprecated and should
    /// not be used to read the cursor position. Use \ref cursor.fraction
    /// instead. However, this property can still sometimes be useful to
    /// compare position values using operators.
    Q_PROPERTY(int tick READ tick)
    /// MIDI tick position, accounting for repeats.
    /// \since MuseScore 4.6
    Q_PROPERTY(int utick READ utick)
    /// Time position in this score, measured as a fraction of a
    /// whole note (the same units MuseScore's code uses internally).
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::FractionWrapper * fraction READ qmlFraction)

    /// Tempo at current tick, read only
    Q_PROPERTY(qreal tempo READ tempo)

    /// Key signature of current staff at tick pos. (read only)
    Q_PROPERTY(int keySignature READ qmlKeySignature)
    /// Associated score
    Q_PROPERTY(apiv1::Score * score READ score WRITE setScore)

    /// Current element at track, read only
    Q_PROPERTY(apiv1::EngravingItem * element READ element) //todo set
    /// Current segment, read only
    Q_PROPERTY(apiv1::Segment * segment READ qmlSegment)
    /// Current measure, read only
    Q_PROPERTY(apiv1::Measure * measure READ measure)
    /// A physical string number where this cursor currently at. This is useful
    /// in conjunction with \ref InputStateMode.INPUT_STATE_SYNC_WITH_SCORE
    /// cursor mode.
    /// \since MuseScore 3.5
    Q_PROPERTY(int stringNumber READ inputStateString WRITE setInputStateString)

public:
    enum RewindMode {
        SCORE_START = 0,     ///< Rewind to the start of a score
        SELECTION_START = 1,     ///< Rewind to the start of a selection
        SELECTION_END = 2     ///< Rewind to the end of a selection
    };
    Q_ENUM(RewindMode);

    /// \since MuseScore 3.5
    enum InputStateMode {
        INPUT_STATE_INDEPENDENT,     ///< Input state of cursor is independent of score input state (default)
        INPUT_STATE_SYNC_WITH_SCORE     ///< Input state of cursor is synchronized with score input state
    };
    Q_ENUM(InputStateMode);

private:
    /// Behavior of input state (position, notes duration etc.) of this cursor
    /// with respect to input state of the score. By default any changes in
    /// score and in this Cursor are not synchronized.
    /// \since MuseScore 3.5
    Q_PROPERTY(InputStateMode inputStateMode READ inputStateMode WRITE setInputStateMode)

    mu::engraving::Score* m_score = nullptr;
//       bool m_expandRepeats; // used?
    engraving::SegmentType m_filter;
    std::unique_ptr<engraving::InputState> is;
    InputStateMode m_inputStateMode = INPUT_STATE_INDEPENDENT;

    // utility methods
    void prevInTrack();
    void nextInTrack();
    void setScore(mu::engraving::Score* s);
    mu::engraving::EngravingItem* currentElement() const;

    engraving::InputState& inputState();
    const engraving::InputState& inputState() const { return const_cast<Cursor*>(this)->inputState(); }

    mu::engraving::Segment* segment() const;
    void setSegment(mu::engraving::Segment* seg);

    mu::engraving::Fraction fraction() const;

    int inputStateString() const;
    void setInputStateString(int);

public:
    /// \cond MS_INTERNAL
    Cursor(mu::engraving::Score* s = nullptr);
//       Cursor(Score*, bool); // not implemented? what is bool?

    Score* score() const;
    void setScore(Score* s);

    int track() const;
    void setTrack(int v);

    int staffIdx() const;
    void setStaffIdx(int v);

    Staff* staff() const;
    void setStaff(Staff* s);

    int voice() const;
    void setVoice(int v);

    int filter() const { return int(m_filter); }
    void setFilter(int f) { m_filter = engraving::SegmentType(f); }

    InputStateMode inputStateMode() const { return m_inputStateMode; }
    void setInputStateMode(InputStateMode val);

    EngravingItem* element() const;
    Segment* qmlSegment() const;
    Measure* measure() const;

    int tick();
    int utick();
    FractionWrapper* qmlFraction() const;
    qreal tempo();

    int qmlKeySignature();
    /// \endcond

    Q_INVOKABLE void rewind(RewindMode mode);
    Q_INVOKABLE void rewindToTick(int tick);
    Q_INVOKABLE void rewindToFraction(apiv1::FractionWrapper* f);
    Q_INVOKABLE double time(bool includeRepeats = false);

    Q_INVOKABLE bool next();
    Q_INVOKABLE bool nextMeasure();
    Q_INVOKABLE bool prev();
    Q_INVOKABLE void add(apiv1::EngravingItem*);

    Q_INVOKABLE void addNote(int pitch, bool addToChord = false);
    Q_INVOKABLE void addRest();
    Q_INVOKABLE void addTuplet(apiv1::FractionWrapper* ratio, apiv1::FractionWrapper* duration);

    //@ set duration
    //@   z: numerator
    //@   n: denominator
    //@   Quarter, if n == 0
    Q_INVOKABLE void setDuration(int z, int n);
};
}
