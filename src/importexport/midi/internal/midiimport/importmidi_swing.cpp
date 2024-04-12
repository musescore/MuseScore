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
#include "importmidi_swing.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tuplet.h"
#include "importmidi_fraction.h"

using namespace mu::engraving;

namespace mu::iex::midi {
namespace Swing {
class SwingDetector
{
public:
    SwingDetector(MidiOperations::Swing st);

    void add(ChordRest* cr);
    bool wasSwingApplied() const { return swingApplied; }

private:
    std::vector<ChordRest*> elements;
    ReducedFraction sumLen;
    const ReducedFraction FULL_LEN = { 1, 4 };
    MidiOperations::Swing swingType;
    bool swingApplied = false;

    void reset();
    void append(ChordRest* cr);
    void checkNormalSwing();
    void checkShuffle();
    void applySwing();
    bool areAllTuplets() const;
    bool areAllNonTuplets() const;
};

SwingDetector::SwingDetector(MidiOperations::Swing st)
    : swingType(st)
{
}

void SwingDetector::add(ChordRest* cr)
{
    if (elements.empty()) {
        if (ReducedFraction(cr->globalTicks()) >= FULL_LEN) {
            return;
        }
        const int tickInBar = (cr->tick() - cr->measure()->tick()).ticks();
        if (tickInBar % Constants::DIVISION == 0) {
            append(cr);
        }
    } else {
        if (sumLen + ReducedFraction(cr->globalTicks()) > FULL_LEN) {
            reset();
            return;
        }
        append(cr);
        if (sumLen == FULL_LEN) {
            // check for swing patterns
            switch (swingType) {
            case MidiOperations::Swing::SWING:
                checkNormalSwing();
                break;
            case MidiOperations::Swing::SHUFFLE:
                checkShuffle();
                break;
            default:
                break;
            }
            reset();
        }
    }
}

void SwingDetector::reset()
{
    elements.clear();
    sumLen = ReducedFraction(Fraction(0, 1));
}

void SwingDetector::append(ChordRest* cr)
{
    if (cr->isChord() || cr->isRest()) {
        elements.push_back(cr);
        sumLen += ReducedFraction(cr->globalTicks());
    }
}

void SwingDetector::checkNormalSwing()
{
    if (elements.size() == 2
        && areAllTuplets()
        && (elements[0]->isChord() || elements[1]->type() == ElementType::CHORD)
        && elements[0]->ticks().reduced() == Fraction(1, 4)
        && elements[1]->ticks().reduced() == Fraction(1, 8)) {
        // swing with two 8th notes
        // or 8th note + 8th rest
        // or 8th rest + 8th note
        applySwing();
    } else if (elements.size() == 3
               && elements[0]->isChord()
               && elements[1]->isRest()
               && elements[2]->isChord()
               && elements[0]->ticks().reduced() == Fraction(1, 8)
               && elements[1]->ticks().reduced() == Fraction(1, 8)
               && elements[2]->ticks().reduced() == Fraction(1, 8)) {
        // swing with two 8th notes
        applySwing();
    }
}

void SwingDetector::checkShuffle()
{
    if (elements.size() == 2
        && areAllNonTuplets()
        && elements[0]->isChord()
        && (elements[1]->isChord()
            || elements[1]->isRest())
        && elements[0]->ticks().reduced() == Fraction(3, 16)            // dotted 8th
        && elements[1]->ticks().reduced() == Fraction(1, 16)) {
        // swing with two 8th notes
        // or 8th note + 8th rest
        applySwing();
    }
}

void SwingDetector::applySwing()
{
    if (elements.size() != 2 && elements.size() != 3) {
        return;
    }

    Tuplet* tuplet = nullptr;
    for (ChordRest* el: elements) {
        el->setDurationType(DurationType::V_EIGHTH);
        el->setTicks(Fraction(1, 8));
        el->setDots(0);
        if (el->tuplet()) {
            if (!tuplet) {
                tuplet = el->tuplet();
            }
            tuplet->remove(el);
            el->setTuplet(nullptr);
        }
    }

    const ChordRest* first = elements.front();
    const int startTick = first->segment()->tick().ticks();
    ChordRest* last = elements.back();
    last->segment()->remove(last);
    Segment* s = last->measure()->getSegment(SegmentType::ChordRest, Fraction::fromTicks(startTick + Constants::DIVISION / 2));
    s->add(last);

    if (elements.size() == 3) {
        // remove central rest
        ChordRest* cr = elements[1];
        cr->score()->removeElement(cr);
        delete cr;
    }

    if (tuplet) {
        // delete tuplet
        delete tuplet;
        tuplet = nullptr;
    }
    if (!swingApplied) {
        swingApplied = true;
    }
}

bool SwingDetector::areAllTuplets() const
{
    for (const auto& el: elements) {
        if (!el->tuplet()) {
            return false;
        }
    }
    return true;
}

bool SwingDetector::areAllNonTuplets() const
{
    for (const auto& el: elements) {
        if (el->tuplet()) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------

QString swingCaption(MidiOperations::Swing swingType)
{
    QString caption;
    switch (swingType) {
    case MidiOperations::Swing::SWING:
        caption = "Swing";
        break;
    case MidiOperations::Swing::SHUFFLE:
        caption = "Shuffle";
        break;
    case MidiOperations::Swing::NONE:
        break;
    }
    return caption;
}

void detectSwing(Staff* staff, MidiOperations::Swing swingType)
{
    Score* score = staff->score();
    const track_idx_t strack = staff->idx() * VOICES;
    SwingDetector swingDetector(swingType);

    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg;
         seg = seg->next1(SegmentType::ChordRest)) {
        for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
            ChordRest* cr = static_cast<ChordRest*>(seg->element(strack + voice));
            if (!cr) {
                continue;
            }
            swingDetector.add(cr);
        }
    }
    if (swingDetector.wasSwingApplied()) {
        // add swing label to the score
        Segment* seg = score->firstSegment(SegmentType::ChordRest);
        StaffText* st = new StaffText(seg, TextStyleType::STAFF);
        st->setPlainText(swingCaption(swingType));
        st->setParent(seg);
        st->setTrack(strack);       // voice == 0
        score->addElement(st);
    }
}
} // namespace Swing
} // namespace mu::iex::midi
