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

#include "chordrest.h"

#include "translation.h"

#include "style/style.h"
#include "types/typesconv.h"
#include "layout/v0/tlayout.h"
#include "layout/v0/chordlayout.h"

#include "actionicon.h"
#include "articulation.h"
#include "barline.h"
#include "beam.h"
#include "breath.h"

#include "chord.h"
#include "clef.h"
#include "connector.h"

#include "factory.h"
#include "figuredbass.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "instrchange.h"
#include "keysig.h"
#include "lyrics.h"
#include "measure.h"
#include "navigate.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "rehearsalmark.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"
#include "tuplet.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(const ElementType& type, Segment* parent)
    : DurationElement(type, parent)
{
    _staffMove    = 0;
    _beam         = 0;
    _tabDur       = 0;
    _up           = true;
    _beamMode     = BeamMode::AUTO;
    m_isSmall     = false;
    _melismaEnd   = false;
    _crossMeasure = CrossMeasure::UNKNOWN;
}

ChordRest::ChordRest(const ChordRest& cr, bool link)
    : DurationElement(cr)
{
    _durationType = cr._durationType;
    _staffMove    = cr._staffMove;
    _beam         = 0;
    _tabDur       = 0;    // tab sur. symb. depends upon context: can't be
                          // simply copied from another CR

    _beamMode     = cr._beamMode;
    _up           = cr._up;
    m_isSmall     = cr.m_isSmall;
    _melismaEnd   = cr._melismaEnd;
    _crossMeasure = cr._crossMeasure;

    for (Lyrics* l : cr._lyrics) {          // make deep copy
        Lyrics* nl = Factory::copyLyrics(*l);
        if (link) {
            nl->linkTo(l);
        }
        nl->setParent(this);
        nl->setTrack(track());
        _lyrics.push_back(nl);
    }
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void ChordRest::undoUnlink()
{
    DurationElement::undoUnlink();
    for (Lyrics* l : _lyrics) {
        l->undoUnlink();
    }
}

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
{
    DeleteAll(_lyrics);
    DeleteAll(_el);
    delete _tabDur;
    if (_beam && _beam->contains(this)) {
        delete _beam;     // Beam destructor removes references to the deleted object
    }
}

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void ChordRest::undoSetSmall(bool val)
{
    undoChangeProperty(Pid::SMALL, val);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* ChordRest::drop(EditData& data)
{
    EngravingItem* e       = data.dropElement;
    Measure* m       = measure();
    bool fromPalette = (e->track() == mu::nidx);
    switch (e->type()) {
    case ElementType::BREATH:
    {
        Breath* b = toBreath(e);
        b->setPos(PointF());
        // allow breath marks in voice > 1
        b->setTrack(this->track());
        b->setPlacement(b->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
        Fraction bt = tick() + actualTicks();
        bt = tick() + actualTicks();

        // TODO: insert automatically in all staves?

        Segment* seg = m->undoGetSegment(SegmentType::Breath, bt);
        b->setParent(seg);
        score()->undoAddElement(b);
    }
        return e;

    case ElementType::BAR_LINE:
        if (data.control()) {
            score()->splitMeasure(segment());
        } else {
            BarLine* bl = toBarLine(e);
            bl->setPos(PointF());
            bl->setTrack(staffIdx() * VOICES);
            bl->setGenerated(false);
            Fraction blt = bl->barLineType() == BarLineType::START_REPEAT ? tick() : tick() + actualTicks();

            if (blt == m->tick() || blt == m->endTick()) {
                return m->drop(data);
            }

            BarLine* obl = 0;
            for (Staff* st  : staff()->staffList()) {
                Score* score = st->score();
                Measure* measure = score->tick2measure(m->tick());
                Segment* seg = measure->undoGetSegment(SegmentType::BarLine, blt);
                BarLine* l;
                if (obl == 0) {
                    obl = l = bl->clone();
                } else {
                    l = toBarLine(obl->linkedClone());
                }
                l->setTrack(st->idx() * VOICES);
                l->setParent(seg);
                score->undoAddElement(l);

                layout()->layoutOnChordRestDrop(l);
            }
        }
        delete e;
        return 0;

    case ElementType::CLEF:
        score()->cmdInsertClef(toClef(e), this);
        break;

    case ElementType::TIMESIG:
        if (measure()->system()) {
            EditData ndd = data;
            // adding from palette sets pos, but normal paste does not
            if (!fromPalette) {
                ndd.pos = pagePos();
            }
            // convert page-relative pos to score-relative
            ndd.pos += measure()->system()->page()->pos();
            return measure()->drop(ndd);
        } else {
            delete e;
            return 0;
        }

    case ElementType::FERMATA:
        e->setPlacement(track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
        for (EngravingItem* el: segment()->annotations()) {
            if (el->isFermata() && (el->track() == track())) {
                if (el->subtype() == e->subtype()) {
                    delete e;
                    return el;
                } else {
                    e->setPlacement(el->placement());
                    e->setTrack(track());
                    e->setParent(segment());
                    score()->undoChangeElement(el, e);
                    return e;
                }
            }
        }
    // fall through
    case ElementType::TEMPO_TEXT:
    case ElementType::DYNAMIC:
    case ElementType::EXPRESSION:
    case ElementType::FRET_DIAGRAM:
    case ElementType::TREMOLOBAR:
    case ElementType::SYMBOL:
        e->setTrack(track());
        e->setParent(segment());
        score()->undoAddElement(e);
        return e;

    case ElementType::NOTE: {
        Note* note = toNote(e);
        Segment* seg = segment();
        score()->undoRemoveElement(this);
        Chord* chord = Factory::createChord(score()->dummy()->segment());
        chord->setTrack(track());
        chord->setDurationType(durationType());
        chord->setTicks(ticks());
        chord->setTuplet(tuplet());
        chord->add(note);
        score()->undoAddCR(chord, seg->measure(), seg->tick());
        return note;
    }

    case ElementType::HARMONY:
    {
        // transpose
        Harmony* harmony = toHarmony(e);
        Interval interval = staff()->part()->instrument(tick())->transpose();
        if (!score()->styleB(Sid::concertPitch) && !interval.isZero()) {
            interval.flip();
            int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
            int baseTpc = transposeTpc(harmony->baseTpc(), interval, true);
            score()->undoTransposeHarmony(harmony, rootTpc, baseTpc);
        }
        // render
        harmony->render();
    }
    // fall through
    case ElementType::TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::SYSTEM_TEXT:
    case ElementType::TRIPLET_FEEL:
    case ElementType::PLAYTECH_ANNOTATION:
    case ElementType::STICKING:
    case ElementType::STAFF_STATE:
    case ElementType::HARP_DIAGRAM:
    // fall through
    case ElementType::REHEARSAL_MARK:
    {
        e->setParent(segment());
        e->setTrack(trackZeroVoice(track()));
        if (e->isRehearsalMark()) {
            RehearsalMark* r = toRehearsalMark(e);
            if (fromPalette) {
                r->setXmlText(score()->createRehearsalMarkText(r));
            }
        }
        // Match pedal config with previous diagram's
        if (e->isHarpPedalDiagram()) {
            HarpPedalDiagram* h = toHarpPedalDiagram(e);
            if (fromPalette && part()) {
                HarpPedalDiagram* prevDiagram = part()->prevHarpDiagram(segment()->tick());
                if (prevDiagram) {
                    h->setPedalState(prevDiagram->getPedalState());
                }
            }
        }
        score()->undoAddElement(e);
        return e;
    }
    case ElementType::INSTRUMENT_CHANGE:
        if (part()->instruments().find(tick().ticks()) != part()->instruments().end()) {
            LOGD() << "InstrumentChange already exists at tick = " << tick().ticks();
            delete e;
            return nullptr;
        } else {
            InstrumentChange* ic = toInstrumentChange(e);
            ic->setParent(segment());
            ic->setTrack(trackZeroVoice(track()));

            const Instrument* prevInstr = part()->instrument(tick());
            const Instrument instr = *ic->instrument();

            IF_ASSERT_FAILED(prevInstr) {
                delete e;
                return nullptr;
            }

            // temporarily set previous instrument, for correct transposition calculation
            ic->setInstrument(*prevInstr);
            score()->undoAddElement(ic);

            if (!fromPalette) {
                ic->setupInstrument(&instr);
            }
            return e;
        }
    case ElementType::FIGURED_BASS:
    {
        bool bNew;
        FiguredBass* fb = toFiguredBass(e);
        fb->setParent(segment());
        fb->setTrack(trackZeroVoice(track()));
        fb->setTicks(ticks());
        fb->setOnNote(true);
        FiguredBass::addFiguredBassToSegment(segment(), fb->track(), fb->ticks(), &bNew);
        if (bNew) {
            score()->undoAddElement(e);
        }
        return e;
    }

    case ElementType::IMAGE:
        e->setParent(segment());
        score()->undoAddElement(e);
        return e;

    case ElementType::ACTION_ICON:
    {
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::BEAM_AUTO:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::AUTO);
            break;
        case ActionIconType::BEAM_NONE:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::NONE);
            break;
        case ActionIconType::BEAM_BREAK_LEFT:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN);
            break;
        case ActionIconType::BEAM_BREAK_INNER_8TH:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN32);
            break;
        case ActionIconType::BEAM_BREAK_INNER_16TH:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN64);
            break;
        case ActionIconType::BEAM_JOIN:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::MID);
            break;
        default:
            break;
        }
    }
        delete e;
        break;

    case ElementType::KEYSIG:
    {
        KeySig* ks    = toKeySig(e);
        KeySigEvent k = ks->keySigEvent();

        if (data.modifiers & ControlModifier) {
            // apply only to this stave, before the selected chordRest
            score()->undoChangeKeySig(staff(), tick(), k);
            delete ks;
        } else {
            // apply to all staves, at the beginning of the measure
            return m->drop(data);
        }
    }
    break;

    default:
        if (e->isSpanner()) {
            Spanner* spanner = toSpanner(e);
            spanner->setTick(tick());
            if (spanner->systemFlag()) {
                spanner->setTrack(0);
                spanner->setTrack2(0);
            } else {
                spanner->setTrack(track());
                spanner->setTrack2(track());
            }
            score()->undoAddElement(spanner);
            return e;
        }
        LOGD("cannot drop %s", e->typeName());
        delete e;
        return 0;
    }
    return 0;
}

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

Beam* ChordRest::beam() const
{
    return !(measure() && measure()->stemless(staffIdx())) ? _beam : nullptr;
}

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
{
    _beam = b;
}

void ChordRest::setBeamlet(BeamSegment* b)
{
    _beamlet = b;
    segment()->createShape(vStaffIdx());
}

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(DurationType t)
{
    _durationType.setType(t);
    _crossMeasure = CrossMeasure::UNKNOWN;
}

void ChordRest::setDurationType(const Fraction& ticks)
{
    _durationType.setVal(ticks.ticks());
    _crossMeasure = CrossMeasure::UNKNOWN;
}

void ChordRest::setDurationType(TDuration v)
{
    _durationType = v;
    _crossMeasure = CrossMeasure::UNKNOWN;
}

//---------------------------------------------------------
//   durationUserName
//---------------------------------------------------------

String ChordRest::durationUserName() const
{
    String tupletType;
    if (tuplet()) {
        switch (tuplet()->ratio().numerator()) {
        case 2:
            tupletType = mtrc("engraving", "Duplet");
            break;
        case 3:
            tupletType = mtrc("engraving", "Triplet");
            break;
        case 4:
            tupletType = mtrc("engraving", "Quadruplet");
            break;
        case 5:
            tupletType = mtrc("engraving", "Quintuplet");
            break;
        case 6:
            tupletType = mtrc("engraving", "Sextuplet");
            break;
        case 7:
            tupletType = mtrc("engraving", "Septuplet");
            break;
        case 8:
            tupletType = mtrc("engraving", "Octuplet");
            break;
        case 9:
            tupletType = mtrc("engraving", "Nonuplet");
            break;
        default:
            //: %1 is tuplet ratio numerator (i.e. the number of notes in the tuplet)
            tupletType = mtrc("engraving", "%1 note tuplet").arg(tuplet()->ratio().numerator());
        }
    }
    String dotString;
    if (!tupletType.isEmpty()) {
        dotString += u' ';
    }

    switch (dots()) {
    case 1:
        dotString += mtrc("engraving", "Dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 2:
        dotString += mtrc("engraving", "Double dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 3:
        dotString += mtrc("engraving", "Triple dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 4:
        dotString += mtrc("engraving", "Quadruple dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    default:
        dotString += TConv::translatedUserName(durationType().type());
    }
    return String(u"%1%2").arg(tupletType, dotString);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRest::add(EngravingItem* e)
{
    e->setParent(this);
    e->setTrack(track());
    switch (e->type()) {
    case ElementType::ARTICULATION:             // for backward compatibility
        LOGD("ChordRest::add: unknown element %s", e->typeName());
        break;
    case ElementType::LYRICS:
        if (e->isStyled(Pid::OFFSET)) {
            e->setOffset(e->propertyDefault(Pid::OFFSET).value<PointF>());
        }
        _lyrics.push_back(toLyrics(e));
        e->added();
        break;
    default:
        ASSERT_X(u"ChordRest::add: unknown element " + String::fromAscii(e->typeName()));
        break;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ChordRest::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::LYRICS: {
        toLyrics(e)->removeFromScore();
        auto i = std::find(_lyrics.begin(), _lyrics.end(), toLyrics(e));
        if (i != _lyrics.end()) {
            _lyrics.erase(i);
            e->removed();
        } else {
            LOGD("ChordRest::remove: %s %p not found", e->typeName(), e);
        }
    }
    break;
    default:
        ASSERT_X(u"ChordRest::remove: unknown element " + String::fromAscii(e->typeName()));
    }
}

//---------------------------------------------------------
//   removeDeleteBeam
///   Remove ChordRest from beam, delete beam if empty.
///   \param beamed - if the chordrest is beamed (will get
///                   a (new) beam)
//---------------------------------------------------------

void ChordRest::removeDeleteBeam(bool beamed)
{
    if (_beam) {
        Beam* b = _beam;
        _beam->remove(this);
        if (b->empty()) {
            score()->undoRemoveElement(b);
        } else {
            layout::v0::LayoutContext lctx(score());
            layout::v0::TLayout::layout1(b, lctx);
        }
    }
    if (!beamed && isChord()) {
        layout::v0::LayoutContext lctx(score());
        layout::v0::ChordLayout::layoutStem(toChord(this), lctx);
    }
}

void ChordRest::computeUp()
{
    UNREACHABLE;
    _usesAutoUp = false;
    _up = true;
}

//---------------------------------------------------------
//   replaceBeam
//---------------------------------------------------------

void ChordRest::replaceBeam(Beam* newBeam)
{
    if (_beam == newBeam) {
        return;
    }
    removeDeleteBeam(true);
    newBeam->add(this);
}

Slur* ChordRest::slur(const ChordRest* secondChordRest) const
{
    if (secondChordRest == nullptr) {
        secondChordRest = nextChordRest(const_cast<ChordRest*>(this));
    }
    int currentTick = tick().ticks();
    Slur* result = nullptr;
    for (auto it : score()->spannerMap().findOverlapping(currentTick, currentTick + 1)) {
        Spanner* spanner = it.value;
        if (!spanner->isSlur()) {
            continue;
        }
        Slur* slur = toSlur(spanner);
        if (slur->startElement() == this && slur->endElement() == secondChordRest) {
            if (slur->slurDirection() == DirectionV::AUTO) {
                return slur;
            }
            result = slur;
        }
    }
    return result;
}

//---------------------------------------------------------
//   undoSetBeamMode
//---------------------------------------------------------

void ChordRest::undoSetBeamMode(BeamMode mode)
{
    undoChangeProperty(Pid::BEAM_MODE, mode);
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void ChordRest::localSpatiumChanged(double oldValue, double newValue)
{
    DurationElement::localSpatiumChanged(oldValue, newValue);
    for (EngravingItem* e : lyrics()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (EngravingItem* e : el()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue ChordRest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SMALL:      return PropertyValue::fromValue(isSmall());
    case Pid::BEAM_MODE:  return int(beamMode());
    case Pid::STAFF_MOVE: return staffMove();
    case Pid::DURATION_TYPE_WITH_DOTS: return actualDurationType().typeWithDots();
    default:              return DurationElement::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool ChordRest::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::BEAM_MODE:
        setBeamMode(v.value<BeamMode>());
        break;
    case Pid::STAFF_MOVE:
        setStaffMove(v.toInt());
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        measure()->checkMultiVoices(staffIdx());
        break;
    case Pid::DURATION_TYPE_WITH_DOTS:
        setDurationType(v.value<DurationTypeWithDots>());
        break;
    default:
        return DurationElement::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue ChordRest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SMALL:
        return false;
    case Pid::BEAM_MODE:
        return BeamMode::AUTO;
    case Pid::STAFF_MOVE:
        return 0;
    default:
        return DurationElement::propertyDefault(propertyId);
    }
    // Prevent unreachable code warning
    // triggerLayout();
}

//---------------------------------------------------------
//   isGrace
//---------------------------------------------------------

bool ChordRest::isGrace() const
{
    return isChord() && toChord(this)->isGrace();
}

//---------------------------------------------------------
//   isGraceBefore
//---------------------------------------------------------

bool ChordRest::isGraceBefore() const
{
    return isChord()
           && (toChord(this)->noteType() & (
                   NoteType::ACCIACCATURA | NoteType::APPOGGIATURA | NoteType::GRACE4 | NoteType::GRACE16 | NoteType::GRACE32
                   ));
}

//---------------------------------------------------------
//   isGraceAfter
//---------------------------------------------------------

bool ChordRest::isGraceAfter() const
{
    return isChord()
           && (toChord(this)->noteType() & (NoteType::GRACE8_AFTER | NoteType::GRACE16_AFTER | NoteType::GRACE32_AFTER));
}

//---------------------------------------------------------
//   hasBreathMark - determine if chordrest has breath-mark
//---------------------------------------------------------
Breath* ChordRest::hasBreathMark() const
{
    Fraction end = tick() + actualTicks();
    Segment* s = measure()->findSegment(SegmentType::Breath, end);
    return s ? toBreath(s->element(track())) : 0;
}

//---------------------------------------------------------
//   nextSegmentAfterCR
//    returns first segment at tick CR->tick + CR->actualTicks
//    of given types
//---------------------------------------------------------

Segment* ChordRest::nextSegmentAfterCR(SegmentType types) const
{
    Fraction end = tick() + actualTicks();
    for (Segment* s = segment()->next1MM(types); s; s = s->next1MM(types)) {
        // chordrest ends at afrac+actualFraction
        // we return the segment at or after the end of the chordrest
        // Segment::afrac() is based on ticks; use DurationElement::afrac() if possible
        EngravingItem* e = s;
        if (s->isChordRestType()) {
            // Find the first non-NULL element in the segment
            for (EngravingItem* ee : s->elist()) {
                if (ee) {
                    e = ee;
                    break;
                }
            }
        }
        if (e->tick() >= end) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void ChordRest::setTrack(track_idx_t val)
{
    EngravingItem::setTrack(val);
    processSiblings([val](EngravingItem* e) { e->setTrack(val); });
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ChordRest::setScore(Score* s)
{
    EngravingItem::setScore(s);
    processSiblings([s](EngravingItem* e) { e->setScore(s); });
}

//---------------------------------------------------------
//   processSiblings
//---------------------------------------------------------

void ChordRest::processSiblings(std::function<void(EngravingItem*)> func)
{
    if (_beam) {
        func(_beam);
    }
    if (_tabDur) {
        func(_tabDur);
    }
    for (Lyrics* l : _lyrics) {
        func(l);
    }
    if (tuplet()) {
        func(tuplet());
    }
    for (EngravingItem* e : _el) {
        func(e);
    }
}

//---------------------------------------------------------
//   nextArticulationOrLyric
//---------------------------------------------------------

EngravingItem* ChordRest::nextArticulationOrLyric(EngravingItem* e)
{
    if (isChord() && e->isArticulationFamily()) {
        Chord* c = toChord(this);
        auto i = std::find(c->articulations().begin(), c->articulations().end(), e);
        if (i != c->articulations().end()) {
            if (i != c->articulations().end() - 1) {
                return *(i + 1);
            } else {
                if (!_lyrics.empty()) {
                    return _lyrics[0];
                } else {
                    return nullptr;
                }
            }
        }
    } else {
        auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
        if (i != _lyrics.end()) {
            if (i != _lyrics.end() - 1) {
                return *(i + 1);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   prevArticulationOrLyric
//---------------------------------------------------------

EngravingItem* ChordRest::prevArticulationOrLyric(EngravingItem* e)
{
    auto i = std::find(_lyrics.begin(), _lyrics.end(), e);
    if (i != _lyrics.end()) {
        if (i != _lyrics.begin()) {
            return *(i - 1);
        } else {
            if (isChord() && !toChord(this)->articulations().empty()) {
                return toChord(this)->articulations().back();
            } else {
                return nullptr;
            }
        }
    } else if (isChord() && e->isArticulationFamily()) {
        Chord* c = toChord(this);
        auto j = std::find(c->articulations().begin(), c->articulations().end(), e);
        if (j != c->articulations().end()) {
            if (j != c->articulations().begin()) {
                return *(j - 1);
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* ChordRest::nextElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::LYRICS: {
        EngravingItem* next = nextArticulationOrLyric(e);
        if (next) {
            return next;
        } else {
            break;
        }
    }
    default: {
        if (isChord() && !toChord(this)->articulations().empty()) {
            return toChord(this)->articulations()[0];
        } else if (!_lyrics.empty()) {
            return _lyrics[0];
        } else {
            break;
        }
    }
    }
    staff_idx_t staffId = e->staffIdx();
    return segment()->nextElement(staffId);
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* ChordRest::prevElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().back();
    }
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::LYRICS: {
        EngravingItem* prev = prevArticulationOrLyric(e);
        if (prev) {
            return prev;
        } else {
            if (isChord()) {
                return toChord(this)->lastElementBeforeSegment();
            }
        }
        // fall through
    }
    default: {
        break;
    }
    }
    staff_idx_t staffId = e->staffIdx();
    return segment()->prevElement(staffId);
}

//---------------------------------------------------------
//   lastElementBeforeSegment
//---------------------------------------------------------

EngravingItem* ChordRest::lastElementBeforeSegment()
{
    if (!_lyrics.empty()) {
        return _lyrics.back();
    }

    return nullptr;
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* ChordRest::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (_beam && (_beam->elements().front() == this)
        && !measure()->stemless(staffIdx())) {
        _beam->scanElements(data, func, all);
    }
    for (Lyrics* l : _lyrics) {
        l->scanElements(data, func, all);
    }
    DurationElement* de = this;
    while (de->tuplet() && de->tuplet()->elements().front() == de) {
        de->tuplet()->scanElements(data, func, all);
        de = de->tuplet();
    }
    if (_tabDur) {
        func(data, _tabDur);
    }
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* ChordRest::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

String ChordRest::accessibleExtraInfo() const
{
    String rez;
    for (EngravingItem* l : lyrics()) {
        if (!score()->selectionFilter().canSelect(l)) {
            continue;
        }
        rez = String(u"%1 %2").arg(rez, l->screenReaderInfo());
    }

    if (segment()) {
        for (EngravingItem* e : segment()->annotations()) {
            if (!score()->selectionFilter().canSelect(e)) {
                continue;
            }
            if (e->track() == track()) {
                rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
            }
        }

        SpannerMap& smap = score()->spannerMap();
        auto spanners = smap.findOverlapping(tick().ticks(), tick().ticks());
        for (auto interval : spanners) {
            Spanner* s = interval.value;
            if (!score()->selectionFilter().canSelect(s)) {
                continue;
            }
            if (s->type() == ElementType::VOLTA          //voltas are added for barlines
                || s->type() == ElementType::TIE) {      //ties are added in notes
                continue;
            }

            if (s->type() == ElementType::SLUR) {
                if (s->tick() == tick() && s->track() == track()) {
                    rez += u" " + mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
                }
                if (s->tick2() == tick() && s->track2() == track()) {
                    rez += u" " + mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
                }
            } else if (s->staffIdx() == staffIdx()) {
                bool start = s->tick() == tick();
                bool end   = s->tick2() == tick() + ticks();
                if (start && end) {
                    rez += u" " + mtrc("engraving", "Start and end of %1").arg(s->screenReaderInfo());
                } else if (start) {
                    rez += u" " + mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
                } else if (end) {
                    rez += u" " + mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
                }
            }
        }
    }
    return rez;
}

//---------------------------------------------------------
//   isMelismaEnd
//    returns true if chordrest represents the end of a melisma
//---------------------------------------------------------

bool ChordRest::isMelismaEnd() const
{
    return _melismaEnd;
}

//---------------------------------------------------------
//   setMelismaEnd
//---------------------------------------------------------

void ChordRest::setMelismaEnd(bool v)
{
    _melismaEnd = v;
    // TODO: don't take "false" at face value
    // check to see if some other melisma ends here,
    // in which case we can leave this set to true
    // for now, rely on the fact that we'll generate the value correctly on layout
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape ChordRest::shape() const
{
    Shape shape;
    {
        double x1 = 1000000.0;
        double x2 = -1000000.0;
        for (Lyrics* l : _lyrics) {
            if (!l || !l->addToSkyline()) {
                continue;
            }
            double lmargin = score()->styleS(Sid::lyricsMinDistance).val() * spatium() * 0.5;
            double rmargin = lmargin;
            LyricsSyllabic syl = l->syllabic();
            if ((syl == LyricsSyllabic::BEGIN || syl == LyricsSyllabic::MIDDLE) && score()->styleB(Sid::lyricsDashForce)) {
                rmargin = std::max(rmargin, styleP(Sid::lyricsDashMinLength));
            }
            // for horizontal spacing we only need the lyrics width:
            x1 = std::min(x1, l->bbox().x() - lmargin + l->pos().x());
            x2 = std::max(x2, l->bbox().x() + l->bbox().width() + rmargin + l->pos().x());
            if (l->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS)) {
                x2 += spatium();
            }
            shape.addHorizontalSpacing(l, x1, x2);
        }
    }

    if (isMelismaEnd()) {
        double right = rightEdge();
        shape.addHorizontalSpacing(nullptr, right, right);
    }

    return shape;
}

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

Lyrics* ChordRest::lyrics(int no) const
{
    for (Lyrics* l : _lyrics) {
        if (l->no() == no) {
            return l;
        }
    }
    return 0;
}

Lyrics* ChordRest::lyrics(int no, PlacementV p) const
{
    for (Lyrics* l : _lyrics) {
        if (l->placement() == p && l->no() == no) {
            return l;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   lastVerse
//    return last verse number (starting from 0)
//    return -1 if there are no lyrics;
//---------------------------------------------------------

int ChordRest::lastVerse(PlacementV p) const
{
    int lastVerse = -1;

    for (Lyrics* l : _lyrics) {
        if (l->placement() == p && l->no() > lastVerse) {
            lastVerse = l->no();
        }
    }

    return lastVerse;
}

//---------------------------------------------------------
//   removeMarkings
//    - this is normally called after cloning a chord to tie a note over the barline
//    - there is no special undo handling; the assumption is that undo will simply remove the cloned chord
//    - two note tremolos are converted into simple notes
//    - single note tremolos are optionally retained
//---------------------------------------------------------

void ChordRest::removeMarkings(bool /* keepTremolo */)
{
    DeleteAll(el());
    el().clear();
    DeleteAll(lyrics());
    lyrics().clear();
}

//---------------------------------------------------------
//   isBefore
//---------------------------------------------------------

bool ChordRest::isBefore(const ChordRest* o) const
{
    if (!o || this == o) {
        return false;
    }
    int otick = o->tick().ticks();
    int t     = tick().ticks();
    if (t == otick) {   // At least one of the chord is a grace, order the grace notes
        bool oGraceAfter = o->isGraceAfter();
        bool graceAfter  = isGraceAfter();
        bool oGrace      = o->isGrace();
        bool grace       = isGrace();
        // normal note are initialized at graceIndex 0 and graceIndex is 0 based
        size_t oGraceIndex  = oGrace ? toChord(o)->graceIndex() + 1 : 0;
        size_t graceIndex   = grace ? toChord(this)->graceIndex() + 1 : 0;
        if (oGrace) {
            oGraceIndex = toChord(o->explicitParent())->graceNotes().size() - oGraceIndex;
        }
        if (grace) {
            graceIndex = toChord(explicitParent())->graceNotes().size() - graceIndex;
        }
        otick = otick + (oGraceAfter ? 1 : -1) * static_cast<int>(oGraceIndex);
        t     = t + (graceAfter ? 1 : -1) * static_cast<int>(graceIndex);
    }
    return t < otick;
}

//---------------------------------------------------------
//   undoAddAnnotation
//---------------------------------------------------------

void ChordRest::undoAddAnnotation(EngravingItem* a)
{
    Segment* seg = segment();
    Measure* m = measure();
    if (m && m->isMMRest()) {
        seg = m->mmRestFirst()->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    }

    a->setTrack(/*a->systemFlag() ? 0 : */ track());
    a->setParent(seg);
    score()->undoAddElement(a);
}

void ChordRest::checkStaffMoveValidity()
{
    if (!staff()) {
        return;
    }
    staff_idx_t idx = _staffMove ? vStaffIdx() : staffIdx() + _storedStaffMove;
    const Staff* baseStaff = staff();
    const StaffType* baseStaffType = baseStaff->staffTypeForElement(this);
    const Staff* targetStaff  = score()->staff(idx);
    const StaffType* targetStaffType = targetStaff ? targetStaff->staffTypeForElement(this) : nullptr;
    // check that destination staff makes sense
    staff_idx_t minStaff = part()->startTrack() / VOICES;
    staff_idx_t maxStaff = part()->endTrack() / VOICES;
    bool isDestinationValid = targetStaff && targetStaff->visible() && idx >= minStaff && idx < maxStaff
                              && targetStaffType->group() == baseStaffType->group();
    if (!isDestinationValid) {
        LOGD("staffMove out of scope %zu + %d min %zu max %zu",
             staffIdx(), _staffMove, minStaff, maxStaff);
        // If destination staff is invalid, reset to base staff
        if (_staffMove) {
            // Remember the intended staff move, so it can be re-applied if
            // destination staff becomes valid (e.g. unihidden)
            _storedStaffMove = _staffMove;
        }
        undoChangeProperty(Pid::STAFF_MOVE, 0);
    } else if (!_staffMove && _storedStaffMove) {
        undoChangeProperty(Pid::STAFF_MOVE, _storedStaffMove);
    }
}
}
