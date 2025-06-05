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

#include "chordrest.h"

#include "translation.h"

#include "types/typesconv.h"

#include "actionicon.h"
#include "barline.h"
#include "beam.h"
#include "breath.h"
#include "chord.h"
#include "clef.h"
#include "factory.h"
#include "figuredbass.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "hook.h"
#include "instrchange.h"
#include "keysig.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "navigate.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "rehearsalmark.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"
#include "tuplet.h"
#include "utils.h"
#include "volta.h"

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
    m_staffMove    = 0;
    m_beam         = 0;
    m_tabDur       = 0;
    m_beamMode     = BeamMode::AUTO;
    m_isSmall     = false;
    m_melismaEnd   = false;
    m_crossMeasure = CrossMeasure::UNKNOWN;
}

ChordRest::ChordRest(const ChordRest& cr, bool link)
    : DurationElement(cr)
{
    m_durationType = cr.m_durationType;
    m_staffMove    = cr.m_staffMove;
    m_beam         = 0;
    m_tabDur       = 0;    // tab sur. symb. depends upon context: can't be
                           // simply copied from another CR

    m_beamMode     = cr.m_beamMode;
    m_isSmall     = cr.m_isSmall;
    m_melismaEnd   = cr.m_melismaEnd;
    m_crossMeasure = cr.m_crossMeasure;

    for (Lyrics* l : cr.m_lyrics) {          // make deep copy
        Lyrics* nl = Factory::copyLyrics(*l);
        if (link) {
            nl->linkTo(l);
        }
        nl->setParent(this);
        nl->setTrack(track());
        m_lyrics.push_back(nl);
    }
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void ChordRest::undoUnlink()
{
    DurationElement::undoUnlink();
    for (Lyrics* l : m_lyrics) {
        l->undoUnlink();
    }
}

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
{
    muse::DeleteAll(m_lyrics);
    muse::DeleteAll(m_el);
    delete m_tabDur;

    if (m_beam) {
        muse::remove(m_beam->elements(), this);
        m_beam = nullptr;
    }

    if (m_beamlet) {
        m_beamlet = nullptr;
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
    bool fromPalette = (e->track() == muse::nidx);
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
            score()->cmdSplitMeasure(this);
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
    case ElementType::EXPRESSION:
    case ElementType::FRET_DIAGRAM:
    case ElementType::TREMOLOBAR:
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        e->setTrack(track());
        e->setParent(segment());
        score()->undoAddElement(e);
        return e;

    case ElementType::DYNAMIC:
        e->setTrack(track());
        e->checkVoiceAssignmentCompatibleWithTrack();
        e->setParent(segment());
        score()->undoAddElement(e);
        return e;

    case ElementType::NOTE: {
        Note* note = toNote(e);
        // calculate correct transposed tpc
        Interval v = staff()->transpose(tick());
        v.flip();
        note->setTpc2(transposeTpc(note->tpc1(), v, true));

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
        Interval interval = staff()->transpose(tick());
        if (!style().styleB(Sid::concertPitch) && !interval.isZero()) {
            interval.flip();
            int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
            int baseTpc = transposeTpc(harmony->bassTpc(), interval, true);
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
    case ElementType::CAPO:
    case ElementType::STRING_TUNINGS:
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
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN16);
            break;
        case ActionIconType::BEAM_BREAK_INNER_16TH:
            undoChangeProperty(Pid::BEAM_MODE, BeamMode::BEGIN32);
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
            data.pos = canvasPos(); // measure->drop() expects to receive canvas pos
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
    return !(measure() && measure()->stemless(staffIdx())) ? m_beam : nullptr;
}

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
{
    m_beam = b;
}

void ChordRest::setBeamlet(BeamSegment* b)
{
    m_beamlet = b;
}

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(DurationType t)
{
    m_durationType.setType(t);
    m_crossMeasure = CrossMeasure::UNKNOWN;
}

void ChordRest::setDurationType(const Fraction& ticks)
{
    m_durationType.setVal(ticks.ticks());
    m_crossMeasure = CrossMeasure::UNKNOWN;
}

void ChordRest::setDurationType(TDuration v)
{
    m_durationType = v;
    m_crossMeasure = CrossMeasure::UNKNOWN;
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
            tupletType = muse::mtrc("engraving", "Duplet");
            break;
        case 3:
            tupletType = muse::mtrc("engraving", "Triplet");
            break;
        case 4:
            tupletType = muse::mtrc("engraving", "Quadruplet");
            break;
        case 5:
            tupletType = muse::mtrc("engraving", "Quintuplet");
            break;
        case 6:
            tupletType = muse::mtrc("engraving", "Sextuplet");
            break;
        case 7:
            tupletType = muse::mtrc("engraving", "Septuplet");
            break;
        case 8:
            tupletType = muse::mtrc("engraving", "Octuplet");
            break;
        case 9:
            tupletType = muse::mtrc("engraving", "Nonuplet");
            break;
        default:
            //: %1 is tuplet ratio numerator (i.e. the number of notes in the tuplet)
            tupletType = muse::mtrc("engraving", "%1 note tuplet").arg(tuplet()->ratio().numerator());
        }
    }
    String dotString;
    if (!tupletType.isEmpty()) {
        dotString += u' ';
    }

    switch (dots()) {
    case 1:
        dotString += muse::mtrc("engraving", "Dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 2:
        dotString += muse::mtrc("engraving", "Double dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 3:
        dotString += muse::mtrc("engraving", "Triple dotted %1").arg(TConv::translatedUserName(durationType().type()));
        break;
    case 4:
        dotString += muse::mtrc("engraving", "Quadruple dotted %1").arg(TConv::translatedUserName(durationType().type()));
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
        m_lyrics.push_back(toLyrics(e));
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
        auto i = std::find(m_lyrics.begin(), m_lyrics.end(), toLyrics(e));
        if (i != m_lyrics.end()) {
            m_lyrics.erase(i);
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
    setBeamlet(nullptr);
    if (m_beam) {
        Beam* b = m_beam;
        m_beam->remove(this);
        if (b->empty()) {
            score()->doUndoRemoveElement(b);
        } else {
            renderer()->layoutBeam1(b);
        }
    }
    if (!beamed && isChord()) {
        Chord* c = toChord(this);
        if (c->shouldHaveHook()) {
            if (!c->hook()) {
                c->createHook();
            }
        } else if (c->hook()) {
            score()->doUndoRemoveElement(c->hook());
        }
        renderer()->layoutStem(c);
    }
}

void ChordRest::computeUp()
{
    UNREACHABLE;
}

//---------------------------------------------------------
//   replaceBeam
//---------------------------------------------------------

void ChordRest::replaceBeam(Beam* newBeam)
{
    if (m_beam == newBeam) {
        return;
    }
    removeDeleteBeam(true);
    newBeam->add(this);
}

Slur* ChordRest::slur(const ChordRest* secondChordRest) const
{
    if (secondChordRest == nullptr) {
        ChordRestNavigateOptions options;
        options.disableOverRepeats = true;
        secondChordRest = nextChordRest(const_cast<ChordRest*>(this), options);
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

void ChordRest::undoChangeProperty(Pid id, const PropertyValue& newValue, PropertyFlags ps)
{
    if (id == Pid::BEAM_MODE) {
        if (m_durationType.hooks() == 0) {
            return;
        }
        BeamMode newBeamMode = newValue.value<BeamMode>();
        if ((newBeamMode == BeamMode::BEGIN16 && m_durationType.hooks() < 2)
            || (newBeamMode == BeamMode::BEGIN32 && m_durationType.hooks() < 3)) {
            return;
        }
    }

    EngravingItem::undoChangeProperty(id, newValue, ps);
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
        if (s->tick() >= end) {
            return s;
        }
    }
    return nullptr;
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
    if (m_beam) {
        func(m_beam);
    }
    if (m_tabDur) {
        func(m_tabDur);
    }
    for (Lyrics* l : m_lyrics) {
        func(l);
    }
    if (tuplet()) {
        func(tuplet());
    }
    for (EngravingItem* e : m_el) {
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
                if (!m_lyrics.empty()) {
                    return m_lyrics[0];
                } else {
                    return nullptr;
                }
            }
        }
    } else {
        auto i = std::find(m_lyrics.begin(), m_lyrics.end(), e);
        if (i != m_lyrics.end()) {
            if (i != m_lyrics.end() - 1) {
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
    auto i = std::find(m_lyrics.begin(), m_lyrics.end(), e);
    if (i != m_lyrics.end()) {
        if (i != m_lyrics.begin()) {
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
    case ElementType::ORNAMENT:
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
        } else if (!m_lyrics.empty()) {
            return m_lyrics[0];
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
    case ElementType::ORNAMENT:
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

    Tuplet* tuplet = this->tuplet();
    if (tuplet && this == tuplet->elements().front()) {
        return tuplet;
    }

    staff_idx_t staffId = e->staffIdx();
    EngravingItem* prevItem = segment()->prevElement(staffId);
    if (prevItem && prevItem->isNote()) {
        const Chord* prevChord = toNote(prevItem)->chord();
        if (prevChord && !prevChord->articulations().empty()) {
            return prevChord->articulations().back();
        }
    }
    return prevItem;
}

//---------------------------------------------------------
//   lastElementBeforeSegment
//---------------------------------------------------------

EngravingItem* ChordRest::lastElementBeforeSegment()
{
    if (!m_lyrics.empty()) {
        return m_lyrics.back();
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
    if (m_beam && (m_beam->elements().front() == this)
        && !measure()->stemless(staffIdx())) {
        m_beam->scanElements(data, func, all);
    }
    for (Lyrics* l : m_lyrics) {
        l->scanElements(data, func, all);
    }
    DurationElement* de = this;
    while (de->tuplet() && de->tuplet()->elements().front() == de) {
        de->tuplet()->scanElements(data, func, all);
        de = de->tuplet();
    }
    if (m_tabDur) {
        func(data, m_tabDur);
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
                    rez += u" " + muse::mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
                }
                if (s->tick2() == tick() && s->track2() == track()) {
                    rez += u" " + muse::mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
                }
            } else if (s->staffIdx() == staffIdx()) {
                bool start = s->tick() == tick();
                bool end   = s->tick2() == tick() + ticks();
                if (start && end) {
                    rez += u" " + muse::mtrc("engraving", "Start and end of %1").arg(s->screenReaderInfo());
                } else if (start) {
                    rez += u" " + muse::mtrc("engraving", "Start of %1").arg(s->screenReaderInfo());
                } else if (end) {
                    rez += u" " + muse::mtrc("engraving", "End of %1").arg(s->screenReaderInfo());
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
    return m_melismaEnd;
}

//---------------------------------------------------------
//   setMelismaEnd
//---------------------------------------------------------

void ChordRest::setMelismaEnd(bool v)
{
    m_melismaEnd = v;
    // TODO: don't take "false" at face value
    // check to see if some other melisma ends here,
    // in which case we can leave this set to true
    // for now, rely on the fact that we'll generate the value correctly on layout
}

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

Lyrics* ChordRest::lyrics(int no) const
{
    for (Lyrics* l : m_lyrics) {
        if (l->no() == no) {
            return l;
        }
    }
    return 0;
}

Lyrics* ChordRest::lyrics(int no, PlacementV p) const
{
    for (Lyrics* l : m_lyrics) {
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

    for (Lyrics* l : m_lyrics) {
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
    muse::DeleteAll(el());
    clearEls();
    muse::DeleteAll(lyrics());
    lyrics().clear();
}

//---------------------------------------------------------
//   isBefore
//---------------------------------------------------------

bool ChordRest::isBefore(const EngravingItem* o) const
{
    if (!o || this == o) {
        return false;
    }

    const ChordRest* otherCr = nullptr;
    if (o->isChordRest()) {
        otherCr = toChordRest(o);
    } else if (o->isNote()) {
        otherCr = toNote(o)->chord();
    }

    if (!otherCr) {
        return EngravingItem::isBefore(o);
    }

    int otick = o->tick().ticks();
    int t     = tick().ticks();
    if (t == otick) {   // At least one of the chord is a grace, order the grace notes
        bool oGraceAfter = otherCr->isGraceAfter();
        bool graceAfter  = isGraceAfter();
        bool oGrace      = otherCr->isGrace();
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

bool ChordRest::isBelowCrossBeam(const BeamBase* beamBase) const
{
    return staffMove() >= beamBase->crossStaffIdx();
}

void ChordRest::checkStaffMoveValidity()
{
    if (!staff() || !staff()->visible()) {
        return;
    }
    staff_idx_t idx = m_staffMove ? vStaffIdx() : staffIdx() + m_storedStaffMove;
    const Staff* baseStaff = staff();
    const StaffType* baseStaffType = baseStaff->staffTypeForElement(this);
    const Staff* targetStaff  = score()->staff(idx);
    const StaffType* targetStaffType = targetStaff ? targetStaff->staffTypeForElement(this) : nullptr;
    // check that destination staff makes sense
    staff_idx_t minStaff = part()->startTrack() / VOICES;
    staff_idx_t maxStaff = part()->endTrack() / VOICES;
    bool isDestinationValid = targetStaff && targetStaff->visible() && idx >= minStaff && idx < maxStaff
                              && targetStaffType->group() == baseStaffType->group() && targetStaff->isLinked() == baseStaff->isLinked();
    if (!isDestinationValid) {
        LOGD("staffMove out of scope %zu + %d min %zu max %zu",
             staffIdx(), m_staffMove, minStaff, maxStaff);
        // If destination staff is invalid, reset to base staff
        if (m_staffMove) {
            // Remember the intended staff move, so it can be re-applied if
            // destination staff becomes valid (e.g. unihidden)
            m_storedStaffMove = m_staffMove;
        }
        setStaffMove(0);
    } else if (!m_staffMove && m_storedStaffMove) {
        setStaffMove(m_storedStaffMove);
        m_storedStaffMove = 0;
    }

    if (isDestinationValid) {
        // Move valid, clear stored move
        m_storedStaffMove = 0;
    }
}

bool ChordRest::hasFollowingJumpItem() const
{
    const Segment* seg = segment();
    const Measure* measure = seg ? seg->measure() : nullptr;
    if (!measure) {
        return false;
    }

    if (endTick() != measure->endTick()) {
        return false;
    }

    std::vector<Measure*> followingRepeatMeasures = findFollowingRepeatMeasures(measure);

    return !followingRepeatMeasures.empty();
}

bool ChordRest::hasPrecedingJumpItem() const
{
    TRACEFUNC;
    const Segment* seg = segment();
    const Measure* measure = seg->measure();

    if (tick() != measure->tick()) {
        return false;
    }

    std::vector<Measure*> precedingRepeatMeasures = findPreviousRepeatMeasures(measure);

    return !precedingRepeatMeasures.empty();
}
}
