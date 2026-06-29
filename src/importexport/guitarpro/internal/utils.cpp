/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "utils.h"

#include "realfn.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/note.h"
#include "engraving/dom/parenthesis.h"
#include "engraving/dom/playcounttext.h"
#include "engraving/dom/score.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/style/styledef.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro::utils {
int harmonicOvertone(Note* note, float harmonicValue, int harmonicType)
{
    int result{ 0 };

    if (muse::RealIsEqual(harmonicValue, 12.0f)) {
        result = 12;
    } else if (muse::RealIsEqual(harmonicValue, 7.0f) || muse::RealIsEqual(harmonicValue, 19.0f)) {
        result = 19;
    } else if (muse::RealIsEqual(harmonicValue, 5.0f) || muse::RealIsEqual(harmonicValue, 24.0f)) {
        result = 24;
    } else if (muse::RealIsEqual(harmonicValue, 3.9f)
               || muse::RealIsEqual(harmonicValue, 4.0f)
               || muse::RealIsEqual(harmonicValue, 9.0f)
               || muse::RealIsEqual(harmonicValue, 16.0f)) {
        result = 28;
    } else if (muse::RealIsEqual(harmonicValue, 3.2f)) {
        result = 31;
    } else if (muse::RealIsEqual(harmonicValue, 2.7f)
               || muse::RealIsEqual(harmonicValue, 5.8f)
               || muse::RealIsEqual(harmonicValue, 9.6f)
               || muse::RealIsEqual(harmonicValue, 14.7f)
               || muse::RealIsEqual(harmonicValue, 21.7f)) {
        result = 34;
    } else if (muse::RealIsEqual(harmonicValue, 2.3f)
               || muse::RealIsEqual(harmonicValue, 2.4f)
               || muse::RealIsEqual(harmonicValue, 8.2f)
               || muse::RealIsEqual(harmonicValue, 17.0f)) {
        result = 36;
    } else if (muse::RealIsEqual(harmonicValue, 2.0f)) {
        result = 38;
    } else if (muse::RealIsEqual(harmonicValue, 1.8f)) {
        result = 40;
    }

    return harmonicType == 1 ? result : (result + note->fret());
}

std::vector<int> standardTuningFor(int midiProgram, int stringsCount)
{
    static const std::map<std::pair<int, /* program */ int /* strings count */>, std::vector<int> > tunings {
        // Guitars 6 strings
        { { 25, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 26, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 27, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 28, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 29, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 30, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 31, 6 }, { 40, 45, 50, 55, 59, 64 } },
        { { 32, 6 }, { 40, 45, 50, 55, 59, 64 } },
        // Guitars 7 strings
        { { 25, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 26, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 27, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 28, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 29, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 30, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 31, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        { { 32, 7 }, { 35, 40, 45, 50, 55, 59, 64 } },
        // Basses 4 strings
        { { 33, 4 }, { 28, 33, 38, 43 } },
        { { 34, 4 }, { 28, 33, 38, 43 } },
        { { 35, 4 }, { 28, 33, 38, 43 } },
        { { 36, 4 }, { 28, 33, 38, 43 } },
        { { 37, 4 }, { 28, 33, 38, 43 } },
        { { 38, 4 }, { 28, 33, 38, 43 } },
        // Basses 5 strings
        { { 33, 5 }, { 23, 28, 33, 38, 43 } },
        { { 34, 5 }, { 23, 28, 33, 38, 43 } },
        { { 35, 5 }, { 23, 28, 33, 38, 43 } },
        { { 36, 5 }, { 23, 28, 33, 38, 43 } },
        { { 37, 5 }, { 23, 28, 33, 38, 43 } },
        { { 38, 5 }, { 23, 28, 33, 38, 43 } },
    };
    std::pair<int, int> key{ midiProgram, stringsCount };

    if (tunings.find(key) == tunings.end()) {
        return tunings.at({ 25, 6 });
    }
    return tunings.at(key);
}

bool isStandardTuning(int midiProgram, const std::vector<int>& tuning)
{
    const auto& standardTuning = standardTuningFor(midiProgram, (int)tuning.size());

    for (size_t i = 0; i < standardTuning.size(); ++i) {
        if (tuning[i] != standardTuning[i]) {
            return false;
        }
    }

    return true;
}

Chord* getLocatedChord(mu::engraving::Score* score, Fraction tickFr, track_idx_t track)
{
    const Measure* measure = score->tick2measure(tickFr);
    if (!measure) {
        LOGE() << "bend import error: no valid measure for track " << track << ", tick " << tickFr.ticks();
        return nullptr;
    }

    Chord* chord = measure->findChord(tickFr, track);
    if (!chord) {
        LOGE() << "bend import error: no valid chord for track " << track << ", tick " << tickFr.ticks();
        return nullptr;
    }

    return chord;
}

static void addParenthesesToNotes(Chord* ch, const std::vector<Note*>& notes)
{
    auto createParen = [ch](DirectionH dir) {
        Parenthesis* p = Factory::createParenthesis(ch);
        p->setParent(ch);
        p->setTrack(ch->track());
        p->setDirection(dir);
        return p;
    };

    NoteParenthesisInfo* parenInfo = new NoteParenthesisInfo(createParen(DirectionH::LEFT), createParen(DirectionH::RIGHT), notes);
    ch->addNoteParenthesisInfo(parenInfo);

    for (Note* n : notes) {
        n->setProperty(Pid::HAS_PARENTHESES, PropertyValue::fromValue(ParenthesesMode::BOTH));
        n->setHideGeneratedParens(true);
    }
}

void createGhostNoteParenGroups(Chord* ch)
{
    if (!ch->noteParentheses().empty()) {
        return;
    }

    const std::vector<Note*>& notes = ch->notes();
    const size_t count = notes.size();
    std::vector<Note*> currentGroup;

    for (size_t i = 0; i <= count; ++i) {
        if (i < count && notes[i]->ghost()) {
            currentGroup.push_back(notes[i]);
        } else if (!currentGroup.empty()) {
            addParenthesesToNotes(ch, currentGroup);
            currentGroup.clear();
        }
    }
}

void addPlayCountTexts(Score* score)
{
    const bool showText = score->style().styleB(Sid::repeatPlayCountShow);
    const bool singleRepeats = score->style().styleB(Sid::repeatPlayCountShowSingleRepeats);
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        if (!m->repeatEnd()) {
            continue;
        }
        const int playCount = m->repeatCount();
        if (!showText || (playCount == 2 && !singleRepeats)) {
            continue;
        }
        Segment* endBarSeg = m->last(SegmentType::BarLineTypes);
        if (!endBarSeg || endBarSeg->findAnnotation(ElementType::PLAY_COUNT_TEXT, 0, 0)) {
            continue;
        }
        PlayCountText* pct = Factory::createPlayCountText(endBarSeg);
        pct->setTrack(0);
        pct->setParent(endBarSeg);
        score->addElement(pct);
    }
}
} // namespace mu::iex::guitarpro
