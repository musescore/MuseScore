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

#include "instrchange.h"

#include "translation.h"

#include "keysig.h"
#include "measure.h"
#include "mscore.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "undo.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   instrumentChangeStyle
//---------------------------------------------------------

static const ElementStyle instrumentChangeStyle {
    { Sid::instrumentChangePlacement,          Pid::PLACEMENT },
    { Sid::instrumentChangeMinDistance,        Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(EngravingItem* parent)
    : TextBase(ElementType::INSTRUMENT_CHANGE, parent, TextStyleType::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&instrumentChangeStyle);
    m_instrument = new Instrument();
}

InstrumentChange::InstrumentChange(const Instrument& i, EngravingItem* parent)
    : TextBase(ElementType::INSTRUMENT_CHANGE, parent, TextStyleType::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&instrumentChangeStyle);
    m_instrument = new Instrument(i);
}

InstrumentChange::InstrumentChange(const InstrumentChange& is)
    : TextBase(is)
{
    m_instrument = new Instrument(*is.m_instrument);
    m_init = is.m_init;
}

InstrumentChange::~InstrumentChange()
{
    delete m_instrument;
}

void InstrumentChange::setInstrument(const Instrument& i)
{
    *m_instrument = i;
    //delete _instrument;
    //_instrument = new Instrument(i);
}

void InstrumentChange::setupInstrument(const Instrument* instrument)
{
    if (m_init) {
        Fraction tickStart = segment()->tick();
        Part* part = staff()->part();
        Interval oldV = part->instrument(tickStart)->transpose();
        Interval oldKv = staff()->transpose(tickStart);
        Interval v = instrument->transpose();
        bool concPitch = style().styleB(Sid::concertPitch);

        // change the clef for each staff
        for (size_t i = 0; i < part->nstaves(); i++) {
            ClefType oldClefType = concPitch ? part->instrument(tickStart)->clefType(i).concertClef
                                   : part->instrument(tickStart)->clefType(i).transposingClef;
            ClefType newClefType = concPitch ? instrument->clefType(i).concertClef
                                   : instrument->clefType(i).transposingClef;
            // Introduce cleff change only if the new clef *symbol* is different from the old one
            if (ClefInfo::symId(oldClefType) != ClefInfo::symId(newClefType)) {
                // If instrument change is at the start of a measure, use the measure as the element, as this will place the instrument change before the barline.
                EngravingItem* element = rtick().isZero() ? toEngravingItem(findMeasure()) : toEngravingItem(this);
                score()->undoChangeClef(part->staff(i), element, newClefType, true);
            }
        }

        // Change key signature if necessary. CAUTION: not necessary in case of octave-transposing!
        if ((v.chromatic - oldV.chromatic) % 12) {
            for (size_t i = 0; i < part->nstaves(); i++) {
                if (!part->staff(i)->keySigEvent(tickStart).isAtonal()) {
                    KeySigEvent ks;
                    // Check, if some key signature is already there, if no, mark new one "for instrument change"
                    Segment* seg = segment()->prev1(SegmentType::KeySig);
                    voice_idx_t voice = part->staff(i)->idx() * VOICES;
                    KeySig* ksig = seg ? toKeySig(seg->element(voice)) : nullptr;
                    bool forInstChange = !(ksig && ksig->tick() == tickStart && !ksig->generated());
                    ks.setForInstrumentChange(forInstChange);
                    Key cKey = part->staff(i)->concertKey(tickStart);
                    ks.setConcertKey(cKey);
                    score()->undoChangeKeySig(part->staff(i), tickStart, ks);
                }
            }
        }

        // change instrument in all linked scores
        for (EngravingObject* se : linkList()) {
            InstrumentChange* lic = static_cast<InstrumentChange*>(se);
            Instrument* newInstrument = new Instrument(*instrument);
            lic->score()->undo(new ChangeInstrument(lic, newInstrument));
        }

        // transpose for current score only
        // this automatically propagates to linked scores
        if (part->instrument(tickStart)->transpose() != oldV) {
            auto i = part->instruments().upper_bound(tickStart.ticks());          // find(), ++i
            Fraction tickEnd;
            if (i == part->instruments().end()) {
                tickEnd = Fraction(-1, 1);
            } else {
                tickEnd = Fraction::fromTicks(i->first);
            }
            score()->transpositionChanged(part, oldKv, tickStart, tickEnd);
        }

        //: The text of an "instrument change" marking. It is an instruction to the player to switch to another instrument.
        const String newInstrChangeText = muse::mtrc("engraving", "To %1").arg(instrument->trackName());
        undoChangeProperty(Pid::TEXT, TextBase::plainToXmlText(newInstrChangeText));
    }
}

//---------------------------------------------------------
//   keySigs
//---------------------------------------------------------

std::vector<KeySig*> InstrumentChange::keySigs(bool all) const
{
    std::vector<KeySig*> keysigs;
    Segment* seg = segment()->prev1(SegmentType::KeySig);
    if (seg) {
        voice_idx_t startVoice = part()->staff(0)->idx() * VOICES;
        voice_idx_t endVoice = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
        Fraction t = tick();
        for (voice_idx_t i = startVoice; i <= endVoice; i += VOICES) {
            KeySig* ks = toKeySig(seg->element(i));
            if (ks && (all || ks->forInstrumentChange()) && ks->tick() == t) {
                keysigs.push_back(ks);
            }
        }
    }
    return keysigs;
}

//---------------------------------------------------------
//   clefs
//---------------------------------------------------------

std::vector<Clef*> InstrumentChange::clefs() const
{
    std::vector<Clef*> clefs;
    Segment* seg = segment()->prev1(SegmentType::Clef);
    if (seg) {
        voice_idx_t startVoice = part()->staff(0)->idx() * VOICES;
        voice_idx_t endVoice = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
        Fraction t = tick();
        for (voice_idx_t i = startVoice; i <= endVoice; i += VOICES) {
            Clef* clef = toClef(seg->element(i));
            if (clef && clef->forInstrumentChange() && clef->tick() == t) {
                clefs.push_back(clef);
            }
        }
    }
    return clefs;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue InstrumentChange::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TEXT_STYLE:
        return TextStyleType::INSTRUMENT_CHANGE;
    default:
        return TextBase::propertyDefault(propertyId);
    }
}
}
