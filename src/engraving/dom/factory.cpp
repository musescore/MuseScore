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

#include "factory.h"

#include "types/typesconv.h"

#include "accidental.h"
#include "actionicon.h"
#include "ambitus.h"
#include "anchors.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bagpembell.h"
#include "barline.h"
#include "beam.h"
#include "bend.h"
#include "box.h"
#include "bracket.h"
#include "breath.h"
#include "capo.h"
#include "chord.h"
#include "chordbracket.h"
#include "chordline.h"
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
#include "pagelockindicator.h"
#include "palmmute.h"
#include "parenthesis.h"
#include "partialtie.h"
#include "pedal.h"
#include "pickscrape.h"
#include "playcounttext.h"
#include "playtechannotation.h"
#include "rasgueado.h"
#include "rehearsalmark.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "slur.h"
#include "soundflag.h"
#include "spacer.h"
#include "staff.h"
#include "stafflines.h"
#include "staffstate.h"
#include "stafftext.h"
#include "stafftypechange.h"
#include "staffvisibilityindicator.h"
#include "stavesharinglabel.h"
#include "stem.h"
#include "stemslash.h"
#include "sticking.h"
#include "stringtunings.h"
#include "system.h"
#include "systemdivider.h"
#include "systemlockindicator.h"
#include "systemtext.h"
#include "tabdurationsymbol.h"
#include "tapping.h"
#include "tempotext.h"
#include "text.h"
#include "textline.h"
#include "tie.h"
#include "timesig.h"
#include "tremolobar.h"
#include "tremolosinglechord.h"
#include "tremolotwochord.h"
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

//! The parent if it is of the given type, the dummy otherwise: a parent of the wrong
//! type is no parent at all.
template<typename T>
static DummyParentOr<T> parentOfTypeOrDummy(EngravingItem* parent, DummyParent* dummy)
{
    return parentOrDummy(dynamic_cast<T*>(parent), dummy);
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
    case ElementType::CLEF:              return new Clef(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::KEYSIG:            return new KeySig(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::TIMESIG:           return new TimeSig(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::BAR_LINE:          return new BarLine(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::SYSTEM_DIVIDER:    return new SystemDivider(parentOfTypeOrDummy<System>(parent, dummy));
    case ElementType::ARPEGGIO:          return new Arpeggio(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::CHORD_BRACKET:     return new ChordBracket(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::BREATH:            return new Breath(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::GLISSANDO:         return new Glissando(parent);
    case ElementType::BRACKET:           return new Bracket(parent);
    case ElementType::ARTICULATION:      return new Articulation(parentOfTypeOrDummy<ChordRest>(parent, dummy));
    case ElementType::TAPPING:           return new Tapping(parentOfTypeOrDummy<ChordRest>(parent, dummy));
    case ElementType::ORNAMENT:          return new Ornament(parentOfTypeOrDummy<ChordRest>(parent, dummy));
    case ElementType::FERMATA:           return new Fermata(parent);
    case ElementType::CHORDLINE:         return new ChordLine(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::ACCIDENTAL:        return new Accidental(parent);
    case ElementType::DYNAMIC:           return new Dynamic(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::EXPRESSION:        return new Expression(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::TEXT:              return new Text(parent);
    case ElementType::MEASURE_NUMBER:    return new MeasureNumber(parentOfTypeOrDummy<Measure>(parent, dummy));
    case ElementType::MMREST_RANGE:      return new MMRestRange(parentOfTypeOrDummy<Measure>(parent, dummy));
    case ElementType::INSTRUMENT_NAME:   return new InstrumentName(parentOfTypeOrDummy<System>(parent, dummy));
    case ElementType::STAFF_TEXT:        return new StaffText(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::STAVE_SHARING_LABEL: return new StaveSharingLabel(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::PLAY_COUNT_TEXT:   return new PlayCountText(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::PLAYTECH_ANNOTATION: return new PlayTechAnnotation(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::CAPO:              return new Capo(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::SYSTEM_TEXT:       return new SystemText(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::REHEARSAL_MARK:    return new RehearsalMark(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::INSTRUMENT_CHANGE: return new InstrumentChange(parent);
    case ElementType::SOUND_FLAG:        return new SoundFlag(parent);
    case ElementType::STAFFTYPE_CHANGE:  return new StaffTypeChange(parentOfTypeOrDummy<MeasureBase>(parent, dummy));
    case ElementType::STAFF_VISIBILITY_INDICATOR: return new StaffVisibilityIndicator(parentOfTypeOrDummy<System>(parent, dummy));
    case ElementType::NOTEHEAD:          return new NoteHead(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::NOTEDOT: {
        if (parent->isNote()) {
            return new NoteDot(toNote(parent));
        } else if (parent->isRest()) {
            return new NoteDot(toRest(parent));
        } else {
            return new NoteDot(dummy);
        }
    }
    case ElementType::TREMOLO_SINGLECHORD: return new TremoloSingleChord(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::TREMOLO_TWOCHORD:  return new TremoloTwoChord(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::LAYOUT_BREAK:      return new LayoutBreak(parentOfTypeOrDummy<MeasureBase>(parent, dummy));
    case ElementType::MARKER:            return new Marker(parent);
    case ElementType::JUMP:              return new Jump(parentOfTypeOrDummy<Measure>(parent, dummy));
    case ElementType::MEASURE_REPEAT:    return new MeasureRepeat(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::ACTION_ICON:       return new ActionIcon(parent);
    case ElementType::NOTE:              return new Note(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::SYMBOL:            return new Symbol(parent);
    case ElementType::FSYMBOL:           return new FSymbol(parent);
    case ElementType::CHORD:             return new Chord(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::REST:              return new Rest(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::MMREST:            return new MMRest(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::DEAD_SLAPPED:      return new DeadSlapped(toRest(parent));
    case ElementType::SPACER:            return new Spacer(parentOfTypeOrDummy<Measure>(parent, dummy));
    case ElementType::STAFF_STATE:       return new StaffState(parent);
    case ElementType::TEMPO_TEXT:        return new TempoText(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::HARMONY:           return new Harmony(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::FRET_DIAGRAM:      return new FretDiagram(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::HARP_DIAGRAM:      return new HarpPedalDiagram(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::BEND:              return new Bend(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::GUITAR_BEND:       return new GuitarBend(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::TREMOLOBAR:        return new TremoloBar(parent);
    case ElementType::LYRICS:            return new Lyrics(parentOfTypeOrDummy<ChordRest>(parent, dummy));
    case ElementType::LYRICSLINE:        return new LyricsLine(parent);
    case ElementType::FIGURED_BASS:      return new FiguredBass(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::STEM:              return new Stem(parentOfTypeOrDummy<Chord>(parent, dummy));
    case ElementType::SLUR:              return new Slur(parent);
    case ElementType::HAMMER_ON_PULL_OFF: return new HammerOnPullOff(parent);
    case ElementType::TIE:               return new Tie(parent);
    case ElementType::TUPLET:            return new Tuplet(parentOfTypeOrDummy<Measure>(parent, dummy));
    case ElementType::FINGERING:         return new Fingering(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::HBOX:              return new HBox(parent->score());
    case ElementType::VBOX:              return new VBox(parent->score());
    case ElementType::TBOX:              return new TBox(parent->score());
    case ElementType::FBOX:              return new FBox(parent->score());
    case ElementType::MEASURE:           return new Measure(parent->score());
    case ElementType::TAB_DURATION_SYMBOL: return new TabDurationSymbol(parentOfTypeOrDummy<ChordRest>(parent, dummy));
    case ElementType::IMAGE:             return new Image(parent);
    case ElementType::BAGPIPE_EMBELLISHMENT: return new BagpipeEmbellishment(parent);
    case ElementType::AMBITUS:           return new Ambitus(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::STICKING:          return new Sticking(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::TRIPLET_FEEL:      return new TripletFeel(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::STRING_TUNINGS:      return new StringTunings(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::TIME_TICK_ANCHOR:  return new TimeTickAnchor(parentOfTypeOrDummy<Segment>(parent, dummy));
    case ElementType::LAISSEZ_VIB:       return new LaissezVib(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::PARTIAL_TIE:       return new PartialTie(parentOfTypeOrDummy<Note>(parent, dummy));
    case ElementType::PARTIAL_LYRICSLINE: return new PartialLyricsLine(parent);
    case ElementType::PARENTHESIS:       return new Parenthesis(parent);

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
    case ElementType::SHARED_PART:
    case ElementType::STAFF:
    case ElementType::SCORE:
    case ElementType::BRACKET_ITEM:
    case ElementType::GRACE_NOTES_GROUP:
    case ElementType::ROOT_ITEM:
    case ElementType::FIGURED_BASS_ITEM:
    case ElementType::DUMMY:
    case ElementType::SYSTEM_LOCK_INDICATOR:
    case ElementType::PAGE_LOCK_INDICATOR:
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

#define CREATE_ITEM_IMPL(T, P, isAccessibleEnabled) \
    T* Factory::create##T(P parent, bool isAccessibleEnabled) \
    { \
        T* e = new T(parent); \
        e->setAccessibleEnabled(isAccessibleEnabled); \
        return e; \
    } \

#define MAKE_ITEM_IMPL(T, P) \
    std::shared_ptr<T> Factory::make##T(P parent) \
    { \
        return std::shared_ptr<T>(create##T(parent)); \
    } \

#define COPY_ITEM_IMPL(T) \
    T* Factory::copy##T(const T& src) \
    { \
        T* copy = new T(src); \
        return copy; \
    } \

CREATE_ITEM_IMPL(Accidental, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Accidental, EngravingItem*)

CREATE_ITEM_IMPL(Ambitus, DummyParentOr<Segment>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Ambitus, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(Arpeggio, DummyParentOr<Chord>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Arpeggio, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(ChordBracket, DummyParentOr<Chord>, isAccessibleEnabled)
MAKE_ITEM_IMPL(ChordBracket, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(Articulation, DummyParentOr<ChordRest>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Articulation, DummyParentOr<ChordRest>)

CREATE_ITEM_IMPL(Tapping, DummyParentOr<ChordRest>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Tapping, DummyParentOr<ChordRest>)

CREATE_ITEM_IMPL(Ornament, DummyParentOr<ChordRest>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Ornament, DummyParentOr<ChordRest>)

CREATE_ITEM_IMPL(BarLine, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(BarLine)
MAKE_ITEM_IMPL(BarLine, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(Beam, Score*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Beam, Score*)

CREATE_ITEM_IMPL(Bend, DummyParentOr<Note>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Bend, DummyParentOr<Note>)

CREATE_ITEM_IMPL(Bracket, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Bracket, EngravingItem*)

BracketItem* Factory::createBracketItem(EngravingItem * parent)
{
    BracketItem* bi = new BracketItem(parent);
    return bi;
}

BracketItem* Factory::createBracketItem(EngravingItem* parent, BracketType bracketType, size_t span)
{
    BracketItem* bi = new BracketItem(parent, bracketType, span);
    return bi;
}

CREATE_ITEM_IMPL(Breath, DummyParentOr<Segment>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Breath, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(Chord, DummyParentOr<Segment>, isAccessibleEnabled)

Chord* Factory::copyChord(const Chord& src, bool link)
{
    Chord* copy = new Chord(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}
MAKE_ITEM_IMPL(Chord, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(ChordLine, DummyParentOr<Chord>, isAccessibleEnabled)
COPY_ITEM_IMPL(ChordLine)
MAKE_ITEM_IMPL(ChordLine, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(Clef, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(Clef)
MAKE_ITEM_IMPL(Clef, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(DeadSlapped, Rest*, isAccessibleEnabled)
COPY_ITEM_IMPL(DeadSlapped)

CREATE_ITEM_IMPL(Fermata, DummyParentOr<Segment>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Fermata, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(FiguredBass, DummyParentOr<Segment>, isAccessibleEnabled)
MAKE_ITEM_IMPL(FiguredBass, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(FretDiagram, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(FretDiagram)
MAKE_ITEM_IMPL(FretDiagram, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(HarpPedalDiagram, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(HarpPedalDiagram)
MAKE_ITEM_IMPL(HarpPedalDiagram, DummyParentOr<Segment>);

CREATE_ITEM_IMPL(KeySig, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(KeySig)
MAKE_ITEM_IMPL(KeySig, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(LaissezVib, DummyParentOr<Note>, isAccessibleEnabled)
COPY_ITEM_IMPL(LaissezVib);

CREATE_ITEM_IMPL(LayoutBreak, DummyParentOr<MeasureBase>, isAccessibleEnabled)
COPY_ITEM_IMPL(LayoutBreak)
MAKE_ITEM_IMPL(LayoutBreak, DummyParentOr<MeasureBase>)

CREATE_ITEM_IMPL(Lyrics, DummyParentOr<ChordRest>, isAccessibleEnabled)
COPY_ITEM_IMPL(Lyrics)

CREATE_ITEM_IMPL(LyricsLine, EngravingItem*, isAccessibleEnabled)
COPY_ITEM_IMPL(LyricsLine)

CREATE_ITEM_IMPL(Measure, Score*, isAccessibleEnabled)
COPY_ITEM_IMPL(Measure)

CREATE_ITEM_IMPL(MeasureRepeat, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(MeasureRepeat)

CREATE_ITEM_IMPL(StringTunings, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(StringTunings)
MAKE_ITEM_IMPL(StringTunings, DummyParentOr<Segment>);

CREATE_ITEM_IMPL(Note, DummyParentOr<Chord>, isAccessibleEnabled)
Note* Factory::copyNote(const Note& src, bool link)
{
    Note* copy = new Note(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}
MAKE_ITEM_IMPL(Note, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(NoteDot, DummyParentOr<Note>, isAccessibleEnabled)
CREATE_ITEM_IMPL(NoteDot, Rest*, isAccessibleEnabled)
COPY_ITEM_IMPL(NoteDot)

CREATE_ITEM_IMPL(NoteLine, DummyParentOr<Note>, isAccessibleEnabled)
MAKE_ITEM_IMPL(NoteLine, DummyParentOr<Note>);

CREATE_ITEM_IMPL(Page, Score*, isAccessibleEnabled)

PageLockIndicator* Factory::createPageLockIndicator(DummyParentOr<System> parent, const RangeLock * lock, bool isAccessibleEnabled)
{
    PageLockIndicator* pli = new PageLockIndicator(parent, lock);
    pli->setAccessibleEnabled(isAccessibleEnabled);
    return pli;
}

COPY_ITEM_IMPL(PageLockIndicator)

CREATE_ITEM_IMPL(PartialTie, DummyParentOr<Note>, isAccessibleEnabled)
COPY_ITEM_IMPL(PartialTie)

CREATE_ITEM_IMPL(PartialLyricsLine, EngravingItem*, isAccessibleEnabled)
COPY_ITEM_IMPL(PartialLyricsLine)

CREATE_ITEM_IMPL(Rest, DummyParentOr<Segment>, isAccessibleEnabled)

Rest* Factory::createRest(DummyParentOr<Segment> parent, const TDuration& t, bool isAccessibleEnabled)
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

CREATE_ITEM_IMPL(Segment, DummyParentOr<Measure>, isAccessibleEnabled)

Segment* Factory::createSegment(DummyParentOr<Measure> parent, SegmentType type, const Fraction& t, bool isAccessibleEnabled)
{
    Segment* s = new Segment(parent, type, t);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

CREATE_ITEM_IMPL(Slur, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Slur, EngravingItem*)

CREATE_ITEM_IMPL(Spacer, DummyParentOr<Measure>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Spacer, DummyParentOr<Measure>)

Staff* Factory::createStaff(Part * parent)
{
    Staff* staff = new Staff(parent);
    staff->setPart(parent);
    return staff;
}

CREATE_ITEM_IMPL(StaffLines, DummyParentOr<Measure>, isAccessibleEnabled)
COPY_ITEM_IMPL(StaffLines)

CREATE_ITEM_IMPL(StaffState, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(StaffTypeChange, DummyParentOr<MeasureBase>, isAccessibleEnabled)
MAKE_ITEM_IMPL(StaffTypeChange, DummyParentOr<MeasureBase>)

StaffText* Factory::createStaffText(DummyParentOr<Segment> parent, TextStyleType textStyleType, bool isAccessibleEnabled)
{
    StaffText* staffText = new StaffText(parent, textStyleType);
    staffText->setAccessibleEnabled(isAccessibleEnabled);

    return staffText;
}

StaveSharingLabel* Factory::createStaveSharingLabel(DummyParentOr<Segment> parent, TextStyleType textStyleType, bool isAccessibleEnabled)
{
    StaveSharingLabel* staveSharingLabel = new StaveSharingLabel(parent, textStyleType);
    staveSharingLabel->setAccessibleEnabled(isAccessibleEnabled);
    return staveSharingLabel;
}

CREATE_ITEM_IMPL(SoundFlag, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(Expression, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(RehearsalMark, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(Stem, DummyParentOr<Chord>, isAccessibleEnabled)
COPY_ITEM_IMPL(Stem)

CREATE_ITEM_IMPL(StemSlash, DummyParentOr<Chord>, isAccessibleEnabled)
COPY_ITEM_IMPL(StemSlash)

CREATE_ITEM_IMPL(System, Score*, isAccessibleEnabled)

SystemText* Factory::createSystemText(DummyParentOr<Segment> parent, TextStyleType textStyleType, ElementType type,
                                      bool isAccessibleEnabled)
{
    SystemText* systemText = new SystemText(parent, textStyleType, type);
    systemText->setAccessibleEnabled(isAccessibleEnabled);

    return systemText;
}

CREATE_ITEM_IMPL(InstrumentChange, DummyParentOr<Segment>, isAccessibleEnabled)

InstrumentChange* Factory::createInstrumentChange(DummyParentOr<Segment> parent, const Instrument& instrument,
                                                  bool isAccessibleEnabled)
{
    InstrumentChange* instrumentChange = new InstrumentChange(instrument, parent);
    instrumentChange->setAccessibleEnabled(isAccessibleEnabled);

    return instrumentChange;
}

CREATE_ITEM_IMPL(Sticking, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(Fingering, DummyParentOr<Note>, isAccessibleEnabled)

Fingering* Factory::createFingering(DummyParentOr<Note> parent, TextStyleType textStyleType,
                                    bool isAccessibleEnabled)
{
    Fingering* fingering = new Fingering(parent, textStyleType);
    fingering->setAccessibleEnabled(isAccessibleEnabled);

    return fingering;
}

CREATE_ITEM_IMPL(Harmony, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(TempoText, DummyParentOr<Segment>, isAccessibleEnabled)

Text* Factory::createText(EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Text* t = new Text(parent, tid);
    t->setAccessibleEnabled(isAccessibleEnabled);

    return t;
}

COPY_ITEM_IMPL(Text)

CREATE_ITEM_IMPL(Tie, EngravingItem*, isAccessibleEnabled)
Tie* Factory::copyTie(const Tie& src)
{
    Tie* copy = src.isLaissezVib() ? new LaissezVib(*toLaissezVib(&src)) : new Tie(src);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}

CREATE_ITEM_IMPL(TimeSig, DummyParentOr<Segment>, isAccessibleEnabled)
COPY_ITEM_IMPL(TimeSig)
MAKE_ITEM_IMPL(TimeSig, DummyParentOr<Segment>)

CREATE_ITEM_IMPL(TremoloTwoChord, DummyParentOr<Chord>, isAccessibleEnabled)
COPY_ITEM_IMPL(TremoloTwoChord)
MAKE_ITEM_IMPL(TremoloTwoChord, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(TremoloSingleChord, DummyParentOr<Chord>, isAccessibleEnabled)
COPY_ITEM_IMPL(TremoloSingleChord)
MAKE_ITEM_IMPL(TremoloSingleChord, DummyParentOr<Chord>)

CREATE_ITEM_IMPL(TremoloBar, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(TremoloBar, EngravingItem*)

CREATE_ITEM_IMPL(Tuplet, DummyParentOr<Measure>, isAccessibleEnabled)
COPY_ITEM_IMPL(Tuplet)

CREATE_ITEM_IMPL(Hairpin, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Hairpin, EngravingItem*)

CREATE_ITEM_IMPL(HammerOnPullOff, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(HammerOnPullOff, EngravingItem*)

CREATE_ITEM_IMPL(Glissando, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(Glissando, EngravingItem*)

CREATE_ITEM_IMPL(GuitarBend, DummyParentOr<Note>, isAccessibleEnabled)
MAKE_ITEM_IMPL(GuitarBend, DummyParentOr<Note>)

CREATE_ITEM_IMPL(Jump, DummyParentOr<Measure>, isAccessibleEnabled)

CREATE_ITEM_IMPL(Trill, EngravingItem*, isAccessibleEnabled)

TripletFeel* Factory::createTripletFeel(DummyParentOr<Segment> parent, TripletFeelType type, bool isAccessibleEnabled)
{
    TripletFeel* t = new TripletFeel(parent, type);
    t->setAccessibleEnabled(isAccessibleEnabled);

    return t;
}

CREATE_ITEM_IMPL(Vibrato, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(TextLine, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(TextLine, EngravingItem*);

CREATE_ITEM_IMPL(Ottava, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(LetRing, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(Marker, EngravingItem*, isAccessibleEnabled)

Marker* Factory::createMarker(EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Marker* m = new Marker(parent, tid);
    m->setAccessibleEnabled(isAccessibleEnabled);

    return m;
}

MAKE_ITEM_IMPL(Marker, EngravingItem*)

CREATE_ITEM_IMPL(GradualTempoChange, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(PalmMute, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(WhammyBar, EngravingItem*, isAccessibleEnabled)
MAKE_ITEM_IMPL(WhammyBar, EngravingItem*)

CREATE_ITEM_IMPL(Rasgueado, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(HarmonicMark, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(PickScrape, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(Volta, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(Pedal, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(Dynamic, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(MMRest, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(VBox, Score*, isAccessibleEnabled)

VBox* Factory::createTitleVBox(Score * parent, bool isAccessibleEnabled)
{
    VBox* b = new VBox(parent);
    b->setAccessibleEnabled(isAccessibleEnabled);
    b->setSizeIsSpatiumDependent(false);
    b->setTick(Fraction(0, 1));

    return b;
}

CREATE_ITEM_IMPL(HBox, Score*, isAccessibleEnabled)

CREATE_ITEM_IMPL(TBox, Score*, isAccessibleEnabled)

CREATE_ITEM_IMPL(FBox, Score*, isAccessibleEnabled)

Image* Factory::createImage(EngravingItem * parent)
{
    Image* image = new Image(parent);
    image->setOwnershipParent(parent);

    return image;
}

CREATE_ITEM_IMPL(Symbol, EngravingItem*, isAccessibleEnabled)
CREATE_ITEM_IMPL(FSymbol, EngravingItem*, isAccessibleEnabled)

CREATE_ITEM_IMPL(PlayCountText, DummyParentOr<Segment>, isAccessibleEnabled)

PlayTechAnnotation* Factory::createPlayTechAnnotation(DummyParentOr<Segment> parent, PlayingTechniqueType techniqueType,
                                                      TextStyleType styleType,
                                                      bool isAccessibleEnabled)
{
    PlayTechAnnotation* annotation = new PlayTechAnnotation(parent, techniqueType, styleType);
    annotation->setAccessibleEnabled(isAccessibleEnabled);

    return annotation;
}

CREATE_ITEM_IMPL(Capo, DummyParentOr<Segment>, isAccessibleEnabled)
MAKE_ITEM_IMPL(Capo, DummyParentOr<Segment>);

CREATE_ITEM_IMPL(TimeTickAnchor, DummyParentOr<Segment>, isAccessibleEnabled)

CREATE_ITEM_IMPL(StaffVisibilityIndicator, DummyParentOr<System>, isAccessibleEnabled)

SystemLockIndicator* Factory::createSystemLockIndicator(DummyParentOr<System> parent, const RangeLock * lock, bool isAccessibleEnabled)
{
    SystemLockIndicator* sli = new SystemLockIndicator(parent, lock);
    sli->setAccessibleEnabled(isAccessibleEnabled);
    return sli;
}

COPY_ITEM_IMPL(SystemLockIndicator)

CREATE_ITEM_IMPL(Parenthesis, EngravingItem*, isAccessibleEnabled);
COPY_ITEM_IMPL(Parenthesis)
