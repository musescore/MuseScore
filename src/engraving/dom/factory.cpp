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

#include "factory.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "actionicon.h"
#include "ambitus.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bagpembell.h"
#include "barline.h"
#include "beam.h"
#include "bend.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "chord.h"
#include "chordline.h"
#include "capo.h"
#include "deadslapped.h"
#include "dynamic.h"
#include "expression.h"
#include "fermata.h"
#include "figuredbass.h"
#include "fingering.h"
#include "fret.h"
#include "glissando.h"
#include "gradualtempochange.h"
#include "guitarbend.h"
#include "hairpin.h"
#include "hammeronpulloff.h"
#include "harmonicmark.h"
#include "harmony.h"
#include "harppedaldiagram.h"
#include "image.h"
#include "instrchange.h"
#include "instrumentname.h"
#include "jump.h"
#include "keysig.h"
#include "laissezvib.h"
#include "layoutbreak.h"
#include "letring.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "measurenumber.h"
#include "measurerepeat.h"
#include "mmrest.h"
#include "mmrestrange.h"
#include "note.h"
#include "noteline.h"
#include "ornament.h"
#include "ottava.h"
#include "page.h"
#include "palmmute.h"
#include "parenthesis.h"
#include "partialtie.h"
#include "pedal.h"
#include "pickscrape.h"
#include "playtechannotation.h"
#include "rasgueado.h"
#include "rehearsalmark.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "spacer.h"
#include "staff.h"
#include "stafflines.h"
#include "staffstate.h"
#include "stafftext.h"
#include "stafftypechange.h"
#include "stem.h"
#include "stemslash.h"
#include "sticking.h"
#include "stringtunings.h"
#include "system.h"
#include "systemdivider.h"
#include "systemlock.h"
#include "systemtext.h"
#include "soundflag.h"
#include "tapping.h"
#include "tempotext.h"
#include "text.h"
#include "textline.h"
#include "tie.h"
#include "timesig.h"
#include "anchors.h"

#include "tremolotwochord.h"
#include "tremolosinglechord.h"
#include "tremolobar.h"
#include "trill.h"
#include "tripletfeel.h"
#include "tuplet.h"
#include "vibrato.h"
#include "volta.h"
#include "whammybar.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

EngravingItem* Factory::createItem(ElementType type, EngravingItem* parent, bool isAccessibleEnabled)
{
    EngravingItem* item = doCreateItem(type, parent);

    if (item) {
        item->setAccessibleEnabled(isAccessibleEnabled);
    }

    return item;
}

EngravingItem* Factory::doCreateItem(ElementType type, EngravingItem* parent)
{
    auto dummy = parent->score()->dummy();
    switch (type) {
    case ElementType::VOLTA:             return new Volta(parent);
    case ElementType::OTTAVA:            return new Ottava(parent);
    case ElementType::TEXTLINE:          return new TextLine(parent);
    case ElementType::NOTELINE:          return new NoteLine(parent);
    case ElementType::TRILL:             return new Trill(parent);
    case ElementType::LET_RING:          return new LetRing(parent);
    case ElementType::GRADUAL_TEMPO_CHANGE: return new GradualTempoChange(parent);
    case ElementType::VIBRATO:           return new Vibrato(parent);
    case ElementType::PALM_MUTE:         return new PalmMute(parent);
    case ElementType::WHAMMY_BAR:        return new WhammyBar(parent);
    case ElementType::RASGUEADO:         return new Rasgueado(parent);
    case ElementType::HARMONIC_MARK:     return new HarmonicMark(parent);
    case ElementType::PICK_SCRAPE:       return new PickScrape(parent);
    case ElementType::PEDAL:             return new Pedal(parent);
    case ElementType::HAIRPIN:           return new Hairpin(parent);
    case ElementType::CLEF:              return new Clef(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::KEYSIG:            return new KeySig(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TIMESIG:           return new TimeSig(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::BAR_LINE:          return new BarLine(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::SYSTEM_DIVIDER:    return new SystemDivider(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::ARPEGGIO:          return new Arpeggio(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::BREATH:            return new Breath(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::GLISSANDO:         return new Glissando(parent);
    case ElementType::BRACKET:           return new Bracket(parent);
    case ElementType::ARTICULATION:      return new Articulation(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::TAPPING:           return new Tapping(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::ORNAMENT:          return new Ornament(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::FERMATA:           return new Fermata(parent);
    case ElementType::CHORDLINE:         return new ChordLine(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::ACCIDENTAL:        return new Accidental(parent);
    case ElementType::DYNAMIC:           return new Dynamic(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::EXPRESSION:        return new Expression(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TEXT:              return new Text(parent);
    case ElementType::MEASURE_NUMBER:    return new MeasureNumber(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::MMREST_RANGE:      return new MMRestRange(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::INSTRUMENT_NAME:   return new InstrumentName(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::STAFF_TEXT:        return new StaffText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::PLAYTECH_ANNOTATION: return new PlayTechAnnotation(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::CAPO:              return new Capo(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::SYSTEM_TEXT:       return new SystemText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::REHEARSAL_MARK:    return new RehearsalMark(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::INSTRUMENT_CHANGE: return new InstrumentChange(parent);
    case ElementType::SOUND_FLAG:        return new SoundFlag(parent);
    case ElementType::STAFFTYPE_CHANGE:  return new StaffTypeChange(parent->isMeasureBase() ? toMeasureBase(parent) : dummy->measure());
    case ElementType::NOTEHEAD:          return new NoteHead(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::NOTEDOT: {
        if (parent->isNote()) {
            return new NoteDot(toNote(parent));
        } else if (parent->isRest()) {
            return new NoteDot(toRest(parent));
        } else {
            return new NoteDot(dummy->note());
        }
    }
    case ElementType::TREMOLO_SINGLECHORD: return new TremoloSingleChord(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::TREMOLO_TWOCHORD:  return new TremoloTwoChord(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::LAYOUT_BREAK:      return new LayoutBreak(parent->isMeasureBase() ? toMeasureBase(parent) : dummy->measure());
    case ElementType::MARKER:            return new Marker(parent);
    case ElementType::JUMP:              return new Jump(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::MEASURE_REPEAT:    return new MeasureRepeat(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::ACTION_ICON:       return new ActionIcon(parent);
    case ElementType::NOTE:              return new Note(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::SYMBOL:            return new Symbol(parent);
    case ElementType::FSYMBOL:           return new FSymbol(parent);
    case ElementType::CHORD:             return new Chord(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::REST:              return new Rest(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::MMREST:            return new MMRest(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::DEAD_SLAPPED:      return new DeadSlapped(toRest(parent));
    case ElementType::SPACER:            return new Spacer(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::STAFF_STATE:       return new StaffState(parent);
    case ElementType::TEMPO_TEXT:        return new TempoText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::HARMONY:           return new Harmony(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::FRET_DIAGRAM:      return new FretDiagram(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::HARP_DIAGRAM:      return new HarpPedalDiagram(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::BEND:              return new Bend(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::GUITAR_BEND:       return new GuitarBend(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::TREMOLOBAR:        return new TremoloBar(parent);
    case ElementType::LYRICS:            return new Lyrics(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::FIGURED_BASS:      return new FiguredBass(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::STEM:              return new Stem(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::SLUR:              return new Slur(parent);
    case ElementType::HAMMER_ON_PULL_OFF: return new HammerOnPullOff(parent);
    case ElementType::TIE:               return new Tie(parent);
    case ElementType::TUPLET:            return new Tuplet(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::FINGERING:         return new Fingering(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::HBOX:              return new HBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::VBOX:              return new VBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::TBOX:              return new TBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::FBOX:              return new FBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::MEASURE:           return new Measure(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::TAB_DURATION_SYMBOL: return new TabDurationSymbol(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::IMAGE:             return new Image(parent);
    case ElementType::BAGPIPE_EMBELLISHMENT: return new BagpipeEmbellishment(parent);
    case ElementType::AMBITUS:           return new Ambitus(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::STICKING:          return new Sticking(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TRIPLET_FEEL:      return new TripletFeel(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::STRING_TUNINGS:      return new StringTunings(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TIME_TICK_ANCHOR:  return new TimeTickAnchor(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::LAISSEZ_VIB:       return new LaissezVib(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::PARTIAL_TIE:       return new PartialTie(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::PARTIAL_LYRICSLINE: return new PartialLyricsLine(parent);
    case ElementType::PARENTHESIS:       return new Parenthesis(parent->isSegment() ? toSegment(parent) : dummy->segment());

    case ElementType::LYRICSLINE:
    case ElementType::TEXTLINE_BASE:
    case ElementType::TEXTLINE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE_SEGMENT:
    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GUITAR_BEND_HOLD:
    case ElementType::GUITAR_BEND_HOLD_SEGMENT:
    case ElementType::GUITAR_BEND_TEXT:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE_SEGMENT:
    case ElementType::STEM_SLASH:
    case ElementType::PAGE:
    case ElementType::BEAM:
    case ElementType::HOOK:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::OTTAVA_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
    case ElementType::WHAMMY_BAR_SEGMENT:
    case ElementType::RASGUEADO_SEGMENT:
    case ElementType::HARMONIC_MARK_SEGMENT:
    case ElementType::PICK_SCRAPE_SEGMENT:
    case ElementType::VOLTA_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
    case ElementType::LEDGER_LINE:
    case ElementType::STAFF_LINES:
    case ElementType::SELECTION:
    case ElementType::LASSO:
    case ElementType::SHADOW_NOTE:
    case ElementType::SEGMENT:
    case ElementType::SYSTEM:
    case ElementType::MAXTYPE:
    case ElementType::INVALID:
    case ElementType::PART:
    case ElementType::STAFF:
    case ElementType::SCORE:
    case ElementType::BRACKET_ITEM:
    case ElementType::GRACE_NOTES_GROUP:
    case ElementType::ROOT_ITEM:
    case ElementType::FIGURED_BASS_ITEM:
    case ElementType::DUMMY:
    case ElementType::SYSTEM_LOCK_INDICATOR:
    case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
    case ElementType::HAMMER_ON_PULL_OFF_TEXT:
    case ElementType::TAPPING_HALF_SLUR:
    case ElementType::TAPPING_HALF_SLUR_SEGMENT:
    case ElementType::TAPPING_TEXT:
        break;
    }

    LOGD() << "Cannot create element of type " << static_cast<int>(type) << " (" << TConv::toXml(type) << ")";

    return nullptr;
}

EngravingItem* Factory::createItemByName(const AsciiStringView& name, EngravingItem* parent, bool isAccessibleEnabled)
{
    ElementType type = TConv::fromXml(name, ElementType::INVALID, isAccessibleEnabled);
    if (type == ElementType::INVALID) {
        LOGE() << "Invalid type: " << name;
        return nullptr;
    }
    return createItem(type, parent, isAccessibleEnabled);
}

#define CREATE_ITEM_IMPL(T, type, P, isAccessibleEnabled) \
    T* Factory::create##T(P * parent, bool isAccessibleEnabled) \
    { \
        EngravingItem* e = createItem(type, parent, isAccessibleEnabled); \
        return item_cast<T*>(e); \
    } \

#define MAKE_ITEM_IMPL(T, P) \
    std::shared_ptr<T> Factory::make##T(P * parent) \
    { \
        return std::shared_ptr<T>(create##T(parent)); \
    } \

#define COPY_ITEM_IMPL(T) \
    T* Factory::copy##T(const T& src) \
    { \
        T* copy = new T(src); \
        return copy; \
    } \

CREATE_ITEM_IMPL(Accidental, ElementType::ACCIDENTAL, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Accidental, EngravingItem)

CREATE_ITEM_IMPL(Ambitus, ElementType::AMBITUS, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(Ambitus, Segment)

CREATE_ITEM_IMPL(Arpeggio, ElementType::ARPEGGIO, Chord, isAccessibleEnabled)
MAKE_ITEM_IMPL(Arpeggio, Chord)

CREATE_ITEM_IMPL(Articulation, ElementType::ARTICULATION, ChordRest, isAccessibleEnabled)
MAKE_ITEM_IMPL(Articulation, ChordRest)

CREATE_ITEM_IMPL(Tapping, ElementType::TAPPING, ChordRest, isAccessibleEnabled)
MAKE_ITEM_IMPL(Tapping, ChordRest)

CREATE_ITEM_IMPL(Ornament, ElementType::ORNAMENT, ChordRest, isAccessibleEnabled)
MAKE_ITEM_IMPL(Ornament, ChordRest)

CREATE_ITEM_IMPL(BarLine, ElementType::BAR_LINE, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(BarLine)
MAKE_ITEM_IMPL(BarLine, Segment)

Beam* Factory::createBeam(System * parent, bool isAccessibleEnabled)
{
    Beam* b = new Beam(parent);
    b->setAccessibleEnabled(isAccessibleEnabled);

    return b;
}

std::shared_ptr<Beam> Factory::makeBeam(System* parent)
{
    return std::shared_ptr<Beam>(createBeam(parent));
}

CREATE_ITEM_IMPL(Bend, ElementType::BEND, Note, isAccessibleEnabled)
MAKE_ITEM_IMPL(Bend, Note)

CREATE_ITEM_IMPL(Bracket, ElementType::BRACKET, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Bracket, EngravingItem)

BracketItem* Factory::createBracketItem(EngravingItem * parent)
{
    BracketItem* bi = new BracketItem(parent);
    return bi;
}

BracketItem* Factory::createBracketItem(EngravingItem* parent, BracketType a, int b)
{
    BracketItem* bi = new BracketItem(parent, a, b);
    return bi;
}

CREATE_ITEM_IMPL(Breath, ElementType::BREATH, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(Breath, Segment)

CREATE_ITEM_IMPL(Chord, ElementType::CHORD, Segment, isAccessibleEnabled)

Chord* Factory::copyChord(const Chord& src, bool link)
{
    Chord* copy = new Chord(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}
MAKE_ITEM_IMPL(Chord, Segment)

CREATE_ITEM_IMPL(ChordLine, ElementType::CHORDLINE, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(ChordLine)
MAKE_ITEM_IMPL(ChordLine, Chord)

CREATE_ITEM_IMPL(Clef, ElementType::CLEF, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(Clef)
MAKE_ITEM_IMPL(Clef, Segment)

CREATE_ITEM_IMPL(DeadSlapped, ElementType::DEAD_SLAPPED, Rest, isAccessibleEnabled)
COPY_ITEM_IMPL(DeadSlapped)

CREATE_ITEM_IMPL(Fermata, ElementType::FERMATA, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(Fermata, Segment)

CREATE_ITEM_IMPL(FiguredBass, ElementType::FIGURED_BASS, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(FiguredBass, Segment)

CREATE_ITEM_IMPL(FretDiagram, ElementType::FRET_DIAGRAM, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(FretDiagram)
MAKE_ITEM_IMPL(FretDiagram, Segment)

CREATE_ITEM_IMPL(HarpPedalDiagram, ElementType::HARP_DIAGRAM, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(HarpPedalDiagram)
MAKE_ITEM_IMPL(HarpPedalDiagram, Segment);

CREATE_ITEM_IMPL(KeySig, ElementType::KEYSIG, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(KeySig)
MAKE_ITEM_IMPL(KeySig, Segment)

CREATE_ITEM_IMPL(LaissezVib, ElementType::LAISSEZ_VIB, Note, isAccessibleEnabled)
COPY_ITEM_IMPL(LaissezVib);

CREATE_ITEM_IMPL(LayoutBreak, ElementType::LAYOUT_BREAK, MeasureBase, isAccessibleEnabled)
COPY_ITEM_IMPL(LayoutBreak)
MAKE_ITEM_IMPL(LayoutBreak, MeasureBase)

CREATE_ITEM_IMPL(Lyrics, ElementType::LYRICS, ChordRest, isAccessibleEnabled)
COPY_ITEM_IMPL(Lyrics)

CREATE_ITEM_IMPL(Measure, ElementType::MEASURE, System, isAccessibleEnabled)
COPY_ITEM_IMPL(Measure)

CREATE_ITEM_IMPL(MeasureRepeat, ElementType::MEASURE_REPEAT, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(MeasureRepeat)

CREATE_ITEM_IMPL(StringTunings, ElementType::STRING_TUNINGS, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(StringTunings)
MAKE_ITEM_IMPL(StringTunings, Segment);

CREATE_ITEM_IMPL(Note, ElementType::NOTE, Chord, isAccessibleEnabled)
Note* Factory::copyNote(const Note& src, bool link)
{
    Note* copy = new Note(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}
MAKE_ITEM_IMPL(Note, Chord)

CREATE_ITEM_IMPL(NoteDot, ElementType::NOTEDOT, Note, isAccessibleEnabled)
CREATE_ITEM_IMPL(NoteDot, ElementType::NOTEDOT, Rest, isAccessibleEnabled)
COPY_ITEM_IMPL(NoteDot)

CREATE_ITEM_IMPL(NoteLine, ElementType::NOTELINE, Note, isAccessibleEnabled)
MAKE_ITEM_IMPL(NoteLine, Note);

Page* Factory::createPage(RootItem* parent, bool isAccessibleEnabled)
{
    Page* page = new Page(parent);
    page->setAccessibleEnabled(isAccessibleEnabled);

    return page;
}

CREATE_ITEM_IMPL(PartialTie, ElementType::PARTIAL_TIE, Note, isAccessibleEnabled)
COPY_ITEM_IMPL(PartialTie)

CREATE_ITEM_IMPL(PartialLyricsLine, ElementType::PARTIAL_LYRICSLINE, EngravingItem, isAccessibleEnabled)
COPY_ITEM_IMPL(PartialLyricsLine)

Rest* Factory::createRest(Segment * parent, bool isAccessibleEnabled)
{
    Rest* r = new Rest(parent);
    r->setAccessibleEnabled(isAccessibleEnabled);

    return r;
}

Rest* Factory::createRest(Segment* parent, const TDuration& t, bool isAccessibleEnabled)
{
    Rest* r = new Rest(parent, t);
    r->setAccessibleEnabled(isAccessibleEnabled);

    return r;
}

Rest* Factory::copyRest(const Rest& src, bool link)
{
    Rest* copy = new Rest(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}

Segment* Factory::createSegment(Measure* parent, bool isAccessibleEnabled)
{
    Segment* s = new Segment(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

Segment* Factory::createSegment(Measure* parent, SegmentType type, const Fraction& t, bool isAccessibleEnabled)
{
    Segment* s = new Segment(parent, type, t);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

CREATE_ITEM_IMPL(Slur, ElementType::SLUR, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Slur, EngravingItem)

CREATE_ITEM_IMPL(Spacer, ElementType::SPACER, Measure, isAccessibleEnabled)
MAKE_ITEM_IMPL(Spacer, Measure)

Staff* Factory::createStaff(Part * parent)
{
    Staff* staff = new Staff(parent);
    staff->setPart(parent);
    return staff;
}

StaffLines* Factory::createStaffLines(Measure* parent, bool isAccessibleEnabled)
{
    StaffLines* sl = new StaffLines(parent);
    sl->setAccessibleEnabled(isAccessibleEnabled);

    return sl;
}

COPY_ITEM_IMPL(StaffLines)

CREATE_ITEM_IMPL(StaffState, ElementType::STAFF_STATE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(StaffTypeChange, ElementType::STAFFTYPE_CHANGE, MeasureBase, isAccessibleEnabled)
MAKE_ITEM_IMPL(StaffTypeChange, MeasureBase)

StaffText* Factory::createStaffText(Segment * parent, TextStyleType textStyleType, bool isAccessibleEnabled)
{
    StaffText* staffText = new StaffText(parent, textStyleType);
    staffText->setAccessibleEnabled(isAccessibleEnabled);

    return staffText;
}

CREATE_ITEM_IMPL(SoundFlag, ElementType::SOUND_FLAG, EngravingItem, isAccessibleEnabled)

Expression* Factory::createExpression(Segment * parent, bool isAccessibleEnabled)
{
    Expression* expression = new Expression(parent);
    expression->setAccessibleEnabled(isAccessibleEnabled);
    return expression;
}

CREATE_ITEM_IMPL(RehearsalMark, ElementType::REHEARSAL_MARK, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Stem, ElementType::STEM, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(Stem)

StemSlash* Factory::createStemSlash(Chord * parent, bool isAccessibleEnabled)
{
    StemSlash* s = new StemSlash(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

COPY_ITEM_IMPL(StemSlash)

System* Factory::createSystem(Page * parent, bool isAccessibleEnabled)
{
    System* s = new System(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

SystemText* Factory::createSystemText(Segment* parent, TextStyleType textStyleType, ElementType type, bool isAccessibleEnabled)
{
    SystemText* systemText = new SystemText(parent, textStyleType, type);
    systemText->setAccessibleEnabled(isAccessibleEnabled);

    return systemText;
}

CREATE_ITEM_IMPL(InstrumentChange, ElementType::INSTRUMENT_CHANGE, Segment, isAccessibleEnabled)

InstrumentChange* Factory::createInstrumentChange(Segment * parent, const Instrument& instrument,
                                                  bool isAccessibleEnabled)
{
    InstrumentChange* instrumentChange = new InstrumentChange(instrument, parent);
    instrumentChange->setAccessibleEnabled(isAccessibleEnabled);

    return instrumentChange;
}

CREATE_ITEM_IMPL(Sticking, ElementType::STICKING, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Fingering, ElementType::FINGERING, Note, isAccessibleEnabled)

Fingering* Factory::createFingering(Note * parent, TextStyleType textStyleType,
                                    bool isAccessibleEnabled)
{
    Fingering* fingering = new Fingering(parent, textStyleType);
    fingering->setAccessibleEnabled(isAccessibleEnabled);

    return fingering;
}

CREATE_ITEM_IMPL(Harmony, ElementType::HARMONY, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(TempoText, ElementType::TEMPO_TEXT, Segment, isAccessibleEnabled)

Text* Factory::createText(EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Text* t = new Text(parent, tid);
    t->setAccessibleEnabled(isAccessibleEnabled);

    return t;
}

COPY_ITEM_IMPL(Text)

CREATE_ITEM_IMPL(Tie, ElementType::TIE, EngravingItem, isAccessibleEnabled)
Tie* Factory::copyTie(const Tie& src)
{
    Tie* copy = src.isLaissezVib() ? new LaissezVib(*toLaissezVib(&src)) : new Tie(src);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}

CREATE_ITEM_IMPL(TimeSig, ElementType::TIMESIG, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(TimeSig)
MAKE_ITEM_IMPL(TimeSig, Segment)

CREATE_ITEM_IMPL(TremoloTwoChord, ElementType::TREMOLO_TWOCHORD, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(TremoloTwoChord)
MAKE_ITEM_IMPL(TremoloTwoChord, Chord)

CREATE_ITEM_IMPL(TremoloSingleChord, ElementType::TREMOLO_SINGLECHORD, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(TremoloSingleChord)
MAKE_ITEM_IMPL(TremoloSingleChord, Chord)

CREATE_ITEM_IMPL(TremoloBar, ElementType::TREMOLOBAR, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(TremoloBar, EngravingItem)

CREATE_ITEM_IMPL(Tuplet, ElementType::TUPLET, Measure, isAccessibleEnabled)
COPY_ITEM_IMPL(Tuplet)

CREATE_ITEM_IMPL(Hairpin, ElementType::HAIRPIN, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Hairpin, EngravingItem)

CREATE_ITEM_IMPL(HammerOnPullOff, ElementType::HAMMER_ON_PULL_OFF, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(HammerOnPullOff, EngravingItem)

CREATE_ITEM_IMPL(Glissando, ElementType::GLISSANDO, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Glissando, EngravingItem)

CREATE_ITEM_IMPL(GuitarBend, ElementType::GUITAR_BEND, Note, isAccessibleEnabled)
MAKE_ITEM_IMPL(GuitarBend, Note)

CREATE_ITEM_IMPL(Jump, ElementType::JUMP, Measure, isAccessibleEnabled)

CREATE_ITEM_IMPL(Trill, ElementType::TRILL, EngravingItem, isAccessibleEnabled)

TripletFeel* Factory::createTripletFeel(Segment * parent, TripletFeelType type, bool isAccessibleEnabled)
{
    TripletFeel* t = new TripletFeel(parent, type);
    t->setAccessibleEnabled(isAccessibleEnabled);

    return t;
}

CREATE_ITEM_IMPL(Vibrato, ElementType::VIBRATO, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(TextLine, ElementType::TEXTLINE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Ottava, ElementType::OTTAVA, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(LetRing, ElementType::LET_RING, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Marker, ElementType::MARKER, EngravingItem, isAccessibleEnabled)

Marker* Factory::createMarker(EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Marker* m = new Marker(parent, tid);
    m->setAccessibleEnabled(isAccessibleEnabled);

    return m;
}

MAKE_ITEM_IMPL(Marker, EngravingItem)

CREATE_ITEM_IMPL(GradualTempoChange, ElementType::GRADUAL_TEMPO_CHANGE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(PalmMute, ElementType::PALM_MUTE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(WhammyBar, ElementType::WHAMMY_BAR, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Rasgueado, ElementType::RASGUEADO, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(HarmonicMark, ElementType::HARMONIC_MARK, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(PickScrape, ElementType::PICK_SCRAPE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Volta, ElementType::VOLTA, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Pedal, ElementType::PEDAL, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Dynamic, ElementType::DYNAMIC, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Harmony, ElementType::HARMONY, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(MMRest, ElementType::MMREST, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(VBox, ElementType::VBOX, System, isAccessibleEnabled)

VBox* Factory::createVBox(const ElementType& type, System * parent, bool isAccessibleEnabled)
{
    VBox* b = new VBox(type, parent);
    b->setAccessibleEnabled(isAccessibleEnabled);

    return b;
}

VBox* Factory::createTitleVBox(System* parent, bool isAccessibleEnabled)
{
    VBox* b = new VBox(ElementType::VBOX, parent);
    b->setAccessibleEnabled(isAccessibleEnabled);
    b->setSizeIsSpatiumDependent(false);
    b->setTick(Fraction(0, 1));

    return b;
}

CREATE_ITEM_IMPL(HBox, ElementType::HBOX, System, isAccessibleEnabled)

CREATE_ITEM_IMPL(TBox, ElementType::TBOX, System, isAccessibleEnabled)

CREATE_ITEM_IMPL(FBox, ElementType::FBOX, System, isAccessibleEnabled)

Image* Factory::createImage(EngravingItem * parent)
{
    Image* image = new Image(parent);
    image->setParent(parent);

    return image;
}

CREATE_ITEM_IMPL(Symbol, ElementType::SYMBOL, EngravingItem, isAccessibleEnabled)
CREATE_ITEM_IMPL(FSymbol, ElementType::FSYMBOL, EngravingItem, isAccessibleEnabled)

PlayTechAnnotation* Factory::createPlayTechAnnotation(Segment * parent, PlayingTechniqueType techniqueType, TextStyleType styleType,
                                                      bool isAccessibleEnabled)
{
    PlayTechAnnotation* annotation = new PlayTechAnnotation(parent, techniqueType, styleType);
    annotation->setAccessibleEnabled(isAccessibleEnabled);

    return annotation;
}

CREATE_ITEM_IMPL(Capo, ElementType::CAPO, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(TimeTickAnchor, ElementType::TIME_TICK_ANCHOR, Segment, isAccessibleEnabled)

SystemLockIndicator* Factory::createSystemLockIndicator(System * parent, const SystemLock * lock, bool isAccessibleEnabled)
{
    SystemLockIndicator* sli = new SystemLockIndicator(parent, lock);
    sli->setAccessibleEnabled(isAccessibleEnabled);
    return sli;
}

COPY_ITEM_IMPL(SystemLockIndicator)

CREATE_ITEM_IMPL(Parenthesis, ElementType::PARENTHESIS, Segment, isAccessibleEnabled);
COPY_ITEM_IMPL(Parenthesis)
