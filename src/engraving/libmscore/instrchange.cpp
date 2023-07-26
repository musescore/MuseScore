/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "factory.h"
#include "keysig.h"
#include "measure.h"
#include "mscore.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftypechange.h"
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
    _instrument = new Instrument();
}

InstrumentChange::InstrumentChange(const Instrument& i, EngravingItem* parent)
    : TextBase(ElementType::INSTRUMENT_CHANGE, parent, TextStyleType::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&instrumentChangeStyle);
    _instrument = new Instrument(i);
}

InstrumentChange::InstrumentChange(const InstrumentChange& is)
    : TextBase(is)
{
    _instrument = new Instrument(*is._instrument);
    _init = is._init;
}

InstrumentChange::~InstrumentChange()
{
    delete _instrument;
}

void InstrumentChange::setInstrument(const Instrument& i)
{
    *_instrument = i;
    //delete _instrument;
    //_instrument = new Instrument(i);
}

void InstrumentChange::setupInstrument(const Instrument* instrument)
{
    if (_init) {
        Fraction tickStart = segment()->tick();
        Fraction tickBeforeIc = tickStart - Fraction::eps();

        Part* part = staff()->part();

        Instrument* oldInstrument = part->instrument(tickStart);

        Interval oldV = oldInstrument->transpose();
        Interval oldKv = staff()->transpose(tickStart);

        bool isPitchedNew = !instrument->useDrumset();
        bool isPitchedBeforeIc = !part->instrument(tickBeforeIc)->useDrumset();

        // change the clef for each staff
        for (size_t i = 0; i < part->nstaves(); i++) {
            ClefTypeList clefType = instrument->clefType(i);
            // If instrument change is at the start of a measure, use the measure as the element, as this will place the instrument change before the barline.
            EngravingItem* element = rtick().isZero() ? toEngravingItem(findMeasure()) : toEngravingItem(this);
            score()->undoChangeClef(part->staff(i), element, clefType._concertClef, clefType._transposingClef, true);
        }

        // change key signature
        for (size_t i = 0; i < part->nstaves(); i++) {
            KeySigEvent oldKeyEvent = isPitchedBeforeIc ? part->staff(i)->keySigEvent(tickStart) : score()->keyList().key(tickStart.ticks());

            bool forInstChange = true;
            if (isPitchedNew) {
                // check, if key signature is already there
                KeySig* ksig = nullptr;
                Segment* seg = segment()->prev(SegmentType::KeySig);
                if (seg) {
                    track_idx_t track = part->staff(i)->idx() * VOICES;
                    ksig = toKeySig(seg->element(track));
                }
                    // key signature is there
                if ((ksig && !ksig->generated() && !ksig->forInstrumentChange())
                    // or on unpitched staff, global key signature is there
                    || (!isPitchedBeforeIc && (score()->keyList().currentKeyTick(tickStart.ticks()) == tickStart.ticks()))) {

                    forInstChange = false;
                }
            }

            KeySigEvent ks;
            if (isPitchedNew) {
                ks.setForInstrumentChange(forInstChange);
                Key cKey = oldKeyEvent.concertKey();
                ks.setConcertKey(cKey);
            } else { // add Atonal Key Signature for unpitched instrument
                ks.setForInstrumentChange(true);
                ks.setConcertKey(Key::C);
                ks.setMode(KeyMode::NONE);
            }
            score()->undoChangeKeySig(part->staff(i), tickStart, ks);
        }

        // change staff type
        for(Staff* staff : part->staves()) {
            track_idx_t track = staff->idx() * VOICES;
            Measure* m = segment()->measure(); //findMeasure();
            StaffType staffType = instrument->defaultStaffType();
            StaffTypeChange* stc = nullptr;
            // if staffTypeChange already there, change it, only if deafaults differ
            if (staff->isStaffTypeStartFrom(tickStart)) {
                StaffType oldStafftype = oldInstrument->defaultStaffType();
                if (!(staffType == oldStafftype)) {
                    for (EngravingItem* e : m->el()) {
                        if (e->isStaffTypeChange() && e->track() == track) {
                            score()->deleteItem(e);
                        }
                    }
                } else {
                    break;
                }
            }
            StaffType* tmpStaffType = new StaffType(staffType);
            stc = Factory::createStaffTypeChange(m);
            stc->setParent(m);
            stc->setTrack(track);
            stc->setForInstrumentChange(true);
            stc->setStaffType(tmpStaffType, true);
            score()->undoAddElement(stc);
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
        const String newInstrChangeText = mtrc("engraving", "To %1").arg(instrument->trackName());
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
//   staffTypeChanges
//---------------------------------------------------------

std::vector<StaffTypeChange*> InstrumentChange::staffTypeChanges() const
{
    std::vector<StaffTypeChange*> staffTypeChanges;
    const MeasureBase* mb = findMeasureBase();
    if (mb) {
        track_idx_t startTrack = part()->staff(0)->idx() * VOICES;
        track_idx_t endTrack = part()->staff(part()->nstaves() - 1)->idx() * VOICES;
        ElementList els = mb->el();
        for(EngravingItem* e : els) {
            track_idx_t eTrack = e->track();
            if (eTrack >= startTrack && eTrack <= endTrack && e->isType(ElementType::STAFFTYPE_CHANGE) && toStaffTypeChange(e)->forInstrumentChange()) {
                staffTypeChanges.push_back(toStaffTypeChange(e));
            }
        }
    }
    return staffTypeChanges;
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
