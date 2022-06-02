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

#include "factory.h"

#include "score.h"

#include "page.h"
#include "rest.h"
#include "segment.h"
#include "staff.h"
#include "stafflines.h"
#include "stemslash.h"
#include "system.h"
#include "volta.h"
#include "ottava.h"
#include "textline.h"
#include "noteline.h"
#include "trill.h"
#include "letring.h"
#include "tempochangeranged.h"
#include "vibrato.h"
#include "palmmute.h"
#include "whammybar.h"
#include "rasgueado.h"
#include "harmonicmark.h"
#include "pedal.h"
#include "hairpin.h"
#include "keysig.h"
#include "timesig.h"
#include "barline.h"
#include "systemdivider.h"
#include "arpeggio.h"
#include "breath.h"
#include "glissando.h"
#include "bracket.h"
#include "articulation.h"
#include "chord.h"
#include "fermata.h"
#include "chordline.h"
#include "slide.h"
#include "accidental.h"
#include "measurenumber.h"
#include "mmrestrange.h"
#include "instrumentname.h"
#include "instrchange.h"
#include "stafftext.h"
#include "systemtext.h"
#include "playtechannotation.h"
#include "rehearsalmark.h"
#include "stafftypechange.h"
#include "tremolo.h"
#include "marker.h"
#include "jump.h"
#include "measurerepeat.h"
#include "actionicon.h"
#include "mmrest.h"
#include "spacer.h"
#include "staffstate.h"
#include "tempotext.h"
#include "harmony.h"
#include "tremolobar.h"
#include "fret.h"
#include "bend.h"
#include "lyrics.h"
#include "figuredbass.h"
#include "slur.h"
#include "tie.h"
#include "fingering.h"
#include "box.h"
#include "image.h"
#include "bagpembell.h"
#include "ambitus.h"
#include "sticking.h"
#include "textframe.h"
#include "tuplet.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

struct ElementName {
    ElementType type;
    AsciiString name;
    const char* userName;
};

//
// list has to be synchronized with ElementType enum
//
/* *INDENT-OFF* */
static const ElementName elementNames[] = {
    { ElementType::INVALID,              "invalid",              QT_TRANSLATE_NOOP("elementName", "Invalid") },
    { ElementType::BRACKET_ITEM,         "BracketItem",          QT_TRANSLATE_NOOP("elementName", "Bracket") },
    { ElementType::PART,                 "Part",                 QT_TRANSLATE_NOOP("elementName", "Part") },
    { ElementType::STAFF,                "Staff",                QT_TRANSLATE_NOOP("elementName", "Staff") },
    { ElementType::SCORE,                "Score",                QT_TRANSLATE_NOOP("elementName", "Score") },
    { ElementType::SYMBOL,               "Symbol",               QT_TRANSLATE_NOOP("elementName", "Symbol") },
    { ElementType::TEXT,                 "Text",                 QT_TRANSLATE_NOOP("elementName", "Text") },
    { ElementType::MEASURE_NUMBER,       "MeasureNumber",        QT_TRANSLATE_NOOP("elementName", "Measure number") },
    { ElementType::MMREST_RANGE,         "MMRestRange",          QT_TRANSLATE_NOOP("elementName", "Multimeasure rest range") },
    { ElementType::INSTRUMENT_NAME,      "InstrumentName",       QT_TRANSLATE_NOOP("elementName", "Instrument name") },
    { ElementType::SLUR_SEGMENT,         "SlurSegment",          QT_TRANSLATE_NOOP("elementName", "Slur segment") },
    { ElementType::TIE_SEGMENT,          "TieSegment",           QT_TRANSLATE_NOOP("elementName", "Tie segment") },
    { ElementType::BAR_LINE,             "BarLine",              QT_TRANSLATE_NOOP("elementName", "Barline") },
    { ElementType::STAFF_LINES,          "StaffLines",           QT_TRANSLATE_NOOP("elementName", "Staff lines") },
    { ElementType::SYSTEM_DIVIDER,       "SystemDivider",        QT_TRANSLATE_NOOP("elementName", "System divider") },
    { ElementType::STEM_SLASH,           "StemSlash",            QT_TRANSLATE_NOOP("elementName", "Stem slash") },
    { ElementType::ARPEGGIO,             "Arpeggio",             QT_TRANSLATE_NOOP("elementName", "Arpeggio") },
    { ElementType::ACCIDENTAL,           "Accidental",           QT_TRANSLATE_NOOP("elementName", "Accidental") },
    { ElementType::LEDGER_LINE,          "LedgerLine",           QT_TRANSLATE_NOOP("elementName", "Ledger line") },
    { ElementType::STEM,                 "Stem",                 QT_TRANSLATE_NOOP("elementName", "Stem") },
    { ElementType::NOTE,                 "Note",                 QT_TRANSLATE_NOOP("elementName", "Note") },
    { ElementType::CLEF,                 "Clef",                 QT_TRANSLATE_NOOP("elementName", "Clef") },
    { ElementType::KEYSIG,               "KeySig",               QT_TRANSLATE_NOOP("elementName", "Key signature") },
    { ElementType::AMBITUS,              "Ambitus",              QT_TRANSLATE_NOOP("elementName", "Ambitus") },
    { ElementType::TIMESIG,              "TimeSig",              QT_TRANSLATE_NOOP("elementName", "Time signature") },
    { ElementType::REST,                 "Rest",                 QT_TRANSLATE_NOOP("elementName", "Rest") },
    { ElementType::MMREST,               "MMRest",               QT_TRANSLATE_NOOP("elementName", "Multimeasure rest") },
    { ElementType::BREATH,               "Breath",               QT_TRANSLATE_NOOP("elementName", "Breath") },
    { ElementType::MEASURE_REPEAT,       "MeasureRepeat",        QT_TRANSLATE_NOOP("elementName", "Measure repeat") },
    { ElementType::TIE,                  "Tie",                  QT_TRANSLATE_NOOP("elementName", "Tie") },
    { ElementType::ARTICULATION,         "Articulation",         QT_TRANSLATE_NOOP("elementName", "Articulation") },
    { ElementType::FERMATA,              "Fermata",              QT_TRANSLATE_NOOP("elementName", "Fermata") },
    { ElementType::CHORDLINE,            "ChordLine",            QT_TRANSLATE_NOOP("elementName", "Chord line") },
    { ElementType::SLIDE,                "Slide",                QT_TRANSLATE_NOOP("elementName", "Slide") },
    { ElementType::DYNAMIC,              "Dynamic",              QT_TRANSLATE_NOOP("elementName", "Dynamic") },
    { ElementType::BEAM,                 "Beam",                 QT_TRANSLATE_NOOP("elementName", "Beam") },
    { ElementType::HOOK,                 "Hook",                 QT_TRANSLATE_NOOP("elementName", "Flag") }, // internally called "Hook", but "Flag" in SMuFL, so here externally too
    { ElementType::LYRICS,               "Lyrics",               QT_TRANSLATE_NOOP("elementName", "Lyrics") },
    { ElementType::FIGURED_BASS,         "FiguredBass",          QT_TRANSLATE_NOOP("elementName", "Figured bass") },
    { ElementType::MARKER,               "Marker",               QT_TRANSLATE_NOOP("elementName", "Marker") },
    { ElementType::JUMP,                 "Jump",                 QT_TRANSLATE_NOOP("elementName", "Jump") },
    { ElementType::FINGERING,            "Fingering",            QT_TRANSLATE_NOOP("elementName", "Fingering") },
    { ElementType::TUPLET,               "Tuplet",               QT_TRANSLATE_NOOP("elementName", "Tuplet") },
    { ElementType::TEMPO_TEXT,           "Tempo",                QT_TRANSLATE_NOOP("elementName", "Tempo") },
    { ElementType::STAFF_TEXT,           "StaffText",            QT_TRANSLATE_NOOP("elementName", "Staff text") },
    { ElementType::SYSTEM_TEXT,          "SystemText",           QT_TRANSLATE_NOOP("elementName", "System text") },
    { ElementType::PLAYTECH_ANNOTATION,  "PlayTechAnnotation",   QT_TRANSLATE_NOOP("elementName", "Playing technique annotation") },
    { ElementType::REHEARSAL_MARK,       "RehearsalMark",        QT_TRANSLATE_NOOP("elementName", "Rehearsal mark") },
    { ElementType::INSTRUMENT_CHANGE,    "InstrumentChange",     QT_TRANSLATE_NOOP("elementName", "Instrument change") },
    { ElementType::STAFFTYPE_CHANGE,     "StaffTypeChange",      QT_TRANSLATE_NOOP("elementName", "Staff type change") },
    { ElementType::HARMONY,              "Harmony",              QT_TRANSLATE_NOOP("elementName", "Chord symbol") },
    { ElementType::FRET_DIAGRAM,         "FretDiagram",          QT_TRANSLATE_NOOP("elementName", "Fretboard diagram") },
    { ElementType::BEND,                 "Bend",                 QT_TRANSLATE_NOOP("elementName", "Bend") },
    { ElementType::TREMOLOBAR,           "TremoloBar",           QT_TRANSLATE_NOOP("elementName", "Tremolo bar") },
    { ElementType::VOLTA,                "Volta",                QT_TRANSLATE_NOOP("elementName", "Volta") },
    { ElementType::HAIRPIN_SEGMENT,      "HairpinSegment",       QT_TRANSLATE_NOOP("elementName", "Hairpin segment") },
    { ElementType::OTTAVA_SEGMENT,       "OttavaSegment",        QT_TRANSLATE_NOOP("elementName", "Ottava segment") },
    { ElementType::TRILL_SEGMENT,        "TrillSegment",         QT_TRANSLATE_NOOP("elementName", "Trill segment") },
    { ElementType::LET_RING_SEGMENT,     "LetRingSegment",       QT_TRANSLATE_NOOP("elementName", "Let ring segment") },
    { ElementType::TEMPO_RANGED_CHANGE_SEGMENT,     "TempoChangeRangedSegment",       QT_TRANSLATE_NOOP("elementName", "Tempo change ranged segment") },
    { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       QT_TRANSLATE_NOOP("elementName", "Vibrato segment") },
    { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      QT_TRANSLATE_NOOP("elementName", "Palm mute segment") },
    { ElementType::WHAMMY_BAR_SEGMENT,   "WhammyBarSegment",     QT_TRANSLATE_NOOP("elementName", "Whammy bar segment") },
    { ElementType::RASGUEADO_SEGMENT,    "RasgueadoSegment",     QT_TRANSLATE_NOOP("elementName", "Rasgueado segment") },
    { ElementType::HARMONIC_MARK_SEGMENT,    "HarmonicMarkSegment",    QT_TRANSLATE_NOOP("elementName", "Harmonic mark segment") },
    { ElementType::TEXTLINE_SEGMENT,     "TextLineSegment",      QT_TRANSLATE_NOOP("elementName", "Text line segment") },
    { ElementType::VOLTA_SEGMENT,        "VoltaSegment",         QT_TRANSLATE_NOOP("elementName", "Volta segment") },
    { ElementType::PEDAL_SEGMENT,        "PedalSegment",         QT_TRANSLATE_NOOP("elementName", "Pedal segment") },
    { ElementType::LYRICSLINE_SEGMENT,   "LyricsLineSegment",    QT_TRANSLATE_NOOP("elementName", "Melisma line segment") },
    { ElementType::GLISSANDO_SEGMENT,    "GlissandoSegment",     QT_TRANSLATE_NOOP("elementName", "Glissando segment") },
    { ElementType::LAYOUT_BREAK,         "LayoutBreak",          QT_TRANSLATE_NOOP("elementName", "Layout break") },
    { ElementType::SPACER,               "Spacer",               QT_TRANSLATE_NOOP("elementName", "Spacer") },
    { ElementType::STAFF_STATE,          "StaffState",           QT_TRANSLATE_NOOP("elementName", "Staff state") },
    { ElementType::NOTEHEAD,             "NoteHead",             QT_TRANSLATE_NOOP("elementName", "Notehead") },
    { ElementType::NOTEDOT,              "NoteDot",              QT_TRANSLATE_NOOP("elementName", "Note dot") },
    { ElementType::TREMOLO,              "Tremolo",              QT_TRANSLATE_NOOP("elementName", "Tremolo") },
    { ElementType::IMAGE,                "Image",                QT_TRANSLATE_NOOP("elementName", "Image") },
    { ElementType::MEASURE,              "Measure",              QT_TRANSLATE_NOOP("elementName", "Measure") },
    { ElementType::SELECTION,            "Selection",            QT_TRANSLATE_NOOP("elementName", "Selection") },
    { ElementType::LASSO,                "Lasso",                QT_TRANSLATE_NOOP("elementName", "Lasso") },
    { ElementType::SHADOW_NOTE,          "ShadowNote",           QT_TRANSLATE_NOOP("elementName", "Shadow note") },
    { ElementType::TAB_DURATION_SYMBOL,  "TabDurationSymbol",    QT_TRANSLATE_NOOP("elementName", "Tab duration symbol") },
    { ElementType::FSYMBOL,              "FSymbol",              QT_TRANSLATE_NOOP("elementName", "Font symbol") },
    { ElementType::PAGE,                 "Page",                 QT_TRANSLATE_NOOP("elementName", "Page") },
    { ElementType::HAIRPIN,              "HairPin",              QT_TRANSLATE_NOOP("elementName", "Hairpin") },
    { ElementType::OTTAVA,               "Ottava",               QT_TRANSLATE_NOOP("elementName", "Ottava") },
    { ElementType::PEDAL,                "Pedal",                QT_TRANSLATE_NOOP("elementName", "Pedal") },
    { ElementType::TRILL,                "Trill",                QT_TRANSLATE_NOOP("elementName", "Trill") },
    { ElementType::LET_RING,             "LetRing",              QT_TRANSLATE_NOOP("elementName", "Let ring") },
    { ElementType::TEMPO_RANGED_CHANGE,  "TempoChangeRanged",    QT_TRANSLATE_NOOP("elementName", "Tempo changed ranged") },
    { ElementType::VIBRATO,              "Vibrato",              QT_TRANSLATE_NOOP("elementName", "Vibrato") },
    { ElementType::PALM_MUTE,            "PalmMute",             QT_TRANSLATE_NOOP("elementName", "Palm mute") },
    { ElementType::WHAMMY_BAR,           "WhammyBar",            QT_TRANSLATE_NOOP("elementName", "Whammy bar") },
    { ElementType::RASGUEADO,            "Rasgueado",            QT_TRANSLATE_NOOP("elementName", "Rasgueado") },
    { ElementType::HARMONIC_MARK,        "HarmonicMark",         QT_TRANSLATE_NOOP("elementName", "Harmonic Mark") },
    { ElementType::TEXTLINE,             "TextLine",             QT_TRANSLATE_NOOP("elementName", "Text line") },
    { ElementType::TEXTLINE_BASE,        "TextLineBase",         QT_TRANSLATE_NOOP("elementName", "Text line base") },    // remove
    { ElementType::NOTELINE,             "NoteLine",             QT_TRANSLATE_NOOP("elementName", "Note line") },
    { ElementType::LYRICSLINE,           "LyricsLine",           QT_TRANSLATE_NOOP("elementName", "Melisma line") },
    { ElementType::GLISSANDO,            "Glissando",            QT_TRANSLATE_NOOP("elementName", "Glissando") },
    { ElementType::BRACKET,              "Bracket",              QT_TRANSLATE_NOOP("elementName", "Bracket") },
    { ElementType::SEGMENT,              "Segment",              QT_TRANSLATE_NOOP("elementName", "Segment") },
    { ElementType::SYSTEM,               "System",               QT_TRANSLATE_NOOP("elementName", "System") },
    { ElementType::COMPOUND,             "Compound",             QT_TRANSLATE_NOOP("elementName", "Compound") },
    { ElementType::CHORD,                "Chord",                QT_TRANSLATE_NOOP("elementName", "Chord") },
    { ElementType::SLUR,                 "Slur",                 QT_TRANSLATE_NOOP("elementName", "Slur") },
    { ElementType::ELEMENT,              "EngravingItem",        QT_TRANSLATE_NOOP("elementName", "EngravingItem") },
    { ElementType::ELEMENT_LIST,         "ElementList",          QT_TRANSLATE_NOOP("elementName", "EngravingItem list") },
    { ElementType::STAFF_LIST,           "StaffList",            QT_TRANSLATE_NOOP("elementName", "Staff list") },
    { ElementType::MEASURE_LIST,         "MeasureList",          QT_TRANSLATE_NOOP("elementName", "Measure list") },
    { ElementType::HBOX,                 "HBox",                 QT_TRANSLATE_NOOP("elementName", "Horizontal frame") },
    { ElementType::VBOX,                 "VBox",                 QT_TRANSLATE_NOOP("elementName", "Vertical frame") },
    { ElementType::TBOX,                 "TBox",                 QT_TRANSLATE_NOOP("elementName", "Text frame") },
    { ElementType::FBOX,                 "FBox",                 QT_TRANSLATE_NOOP("elementName", "Fretboard diagram frame") },
    { ElementType::ACTION_ICON,          "ActionIcon",           QT_TRANSLATE_NOOP("elementName", "Action icon") },
    { ElementType::OSSIA,                "Ossia",                QT_TRANSLATE_NOOP("elementName", "Ossia") },
    { ElementType::BAGPIPE_EMBELLISHMENT,"BagpipeEmbellishment", QT_TRANSLATE_NOOP("elementName", "Bagpipe embellishment") },
    { ElementType::STICKING,             "Sticking",             QT_TRANSLATE_NOOP("elementName", "Sticking") },
    { ElementType::GRACE_NOTES_GROUP,    "GraceNotesGroup",      QT_TRANSLATE_NOOP("elementName", "Grace notes group")},
    { ElementType::ROOT_ITEM,            "RootItem",             QT_TRANSLATE_NOOP("elementName", "Root item") },
    { ElementType::DUMMY,                "Dummy",                QT_TRANSLATE_NOOP("elementName", "Dummy") },
};
/* *INDENT-ON* */

EngravingItem* Factory::createItem(ElementType type, EngravingItem* parent, bool isAccessibleEnabled)
{
    EngravingItem* item = doCreateItem(type, parent);
    item->setAccessibleEnabled(isAccessibleEnabled);

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
    case ElementType::TEMPO_RANGED_CHANGE: return new TempoChangeRanged(parent);
    case ElementType::VIBRATO:           return new Vibrato(parent);
    case ElementType::PALM_MUTE:         return new PalmMute(parent);
    case ElementType::WHAMMY_BAR:        return new WhammyBar(parent);
    case ElementType::RASGUEADO:         return new Rasgueado(parent);
    case ElementType::HARMONIC_MARK:     return new HarmonicMark(parent);
    case ElementType::PEDAL:             return new Pedal(parent);
    case ElementType::HAIRPIN:           return new Hairpin(parent->isSegment() ? toSegment(parent) : dummy->segment());
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
    case ElementType::FERMATA:           return new Fermata(parent);
    case ElementType::CHORDLINE:         return new ChordLine(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::SLIDE:             return new Slide(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::ACCIDENTAL:        return new Accidental(parent);
    case ElementType::DYNAMIC:           return new Dynamic(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TEXT:              return new Text(parent);
    case ElementType::MEASURE_NUMBER:    return new MeasureNumber(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::MMREST_RANGE:      return new MMRestRange(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::INSTRUMENT_NAME:   return new InstrumentName(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::STAFF_TEXT:        return new StaffText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::PLAYTECH_ANNOTATION: return new PlayTechAnnotation(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::SYSTEM_TEXT:       return new SystemText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::REHEARSAL_MARK:    return new RehearsalMark(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::INSTRUMENT_CHANGE: return new InstrumentChange(parent);
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
    case ElementType::TREMOLO:           return new Tremolo(parent->isChord() ? toChord(parent) : dummy->chord());
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
    case ElementType::SPACER:            return new Spacer(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::STAFF_STATE:       return new StaffState(parent);
    case ElementType::TEMPO_TEXT:        return new TempoText(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::HARMONY:           return new Harmony(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::FRET_DIAGRAM:      return new FretDiagram(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::BEND:              return new Bend(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::TREMOLOBAR:        return new TremoloBar(parent);
    case ElementType::LYRICS:            return new Lyrics(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::FIGURED_BASS:      return new FiguredBass(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::STEM:              return new Stem(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::SLUR:              return new Slur(parent);
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

    case ElementType::LYRICSLINE:
    case ElementType::TEXTLINE_BASE:
    case ElementType::TEXTLINE_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::SLUR_SEGMENT:
    case ElementType::TIE_SEGMENT:
    case ElementType::STEM_SLASH:
    case ElementType::PAGE:
    case ElementType::BEAM:
    case ElementType::HOOK:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::OTTAVA_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::TEMPO_RANGED_CHANGE_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
    case ElementType::WHAMMY_BAR_SEGMENT:
    case ElementType::RASGUEADO_SEGMENT:
    case ElementType::HARMONIC_MARK_SEGMENT:
    case ElementType::VOLTA_SEGMENT:
    case ElementType::PEDAL_SEGMENT:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::LEDGER_LINE:
    case ElementType::STAFF_LINES:
    case ElementType::SELECTION:
    case ElementType::LASSO:
    case ElementType::SHADOW_NOTE:
    case ElementType::SEGMENT:
    case ElementType::SYSTEM:
    case ElementType::COMPOUND:
    case ElementType::ELEMENT:
    case ElementType::ELEMENT_LIST:
    case ElementType::STAFF_LIST:
    case ElementType::MEASURE_LIST:
    case ElementType::MAXTYPE:
    case ElementType::INVALID:
    case ElementType::PART:
    case ElementType::STAFF:
    case ElementType::SCORE:
    case ElementType::BRACKET_ITEM:
    case ElementType::OSSIA:
    case ElementType::GRACE_NOTES_GROUP:
    case ElementType::ROOT_ITEM:
    case ElementType::DUMMY:
        break;
    }
    LOGD("cannot create type %d <%s>", int(type), Factory::name(type));
    return 0;
}

EngravingItem* Factory::createItemByName(const AsciiString& name, EngravingItem* parent, bool isAccessibleEnabled)
{
    ElementType type = name2type(name, isAccessibleEnabled);
    if (type == ElementType::INVALID) {
        LOGE() << "Invalid type: " << name;
        return 0;
    }
    return createItem(type, parent, isAccessibleEnabled);
}

ElementType Factory::name2type(const AsciiString& name, bool silent)
{
    for (int i = 0; i < int(ElementType::MAXTYPE); ++i) {
        if (name == elementNames[i].name) {
            return ElementType(i);
        }
    }
    if (!silent) {
        LOGE() << "Unknown type: " << name;
    }
    return ElementType::INVALID;
}

const char* Factory::name(ElementType type)
{
    return elementNames[int(type)].name.ascii();
}

const char* Factory::userName(mu::engraving::ElementType type)
{
    return elementNames[int(type)].userName;
}

#define CREATE_ITEM_IMPL(T, type, P, isAccessibleEnabled) \
    T* Factory::create##T(P * parent, bool isAccessibleEnabled) \
    { \
        EngravingItem* e = createItem(type, parent, isAccessibleEnabled); \
        return to##T(e); \
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

mu::engraving::Chord* Factory::copyChord(const mu::engraving::Chord& src, bool link)
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

CREATE_ITEM_IMPL(Fermata, ElementType::FERMATA, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Fermata, EngravingItem)

CREATE_ITEM_IMPL(FiguredBass, ElementType::FIGURED_BASS, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(FiguredBass, Segment)

CREATE_ITEM_IMPL(FretDiagram, ElementType::FRET_DIAGRAM, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(FretDiagram)
MAKE_ITEM_IMPL(FretDiagram, Segment)

CREATE_ITEM_IMPL(KeySig, ElementType::KEYSIG, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(KeySig)
MAKE_ITEM_IMPL(KeySig, Segment)

CREATE_ITEM_IMPL(LayoutBreak, ElementType::LAYOUT_BREAK, MeasureBase, isAccessibleEnabled)
COPY_ITEM_IMPL(LayoutBreak)
MAKE_ITEM_IMPL(LayoutBreak, MeasureBase)

CREATE_ITEM_IMPL(Lyrics, ElementType::LYRICS, ChordRest, isAccessibleEnabled)
COPY_ITEM_IMPL(Lyrics)

CREATE_ITEM_IMPL(Measure, ElementType::MEASURE, System, isAccessibleEnabled)
COPY_ITEM_IMPL(Measure)

CREATE_ITEM_IMPL(MeasureRepeat, ElementType::MEASURE_REPEAT, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(MeasureRepeat)

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

mu::engraving::Page* Factory::createPage(RootItem * parent, bool isAccessibleEnabled)
{
    Page* page = new Page(parent);
    page->setAccessibleEnabled(isAccessibleEnabled);

    return page;
}

mu::engraving::Rest* Factory::createRest(mu::engraving::Segment* parent, bool isAccessibleEnabled)
{
    Rest* r = new Rest(parent);
    r->setAccessibleEnabled(isAccessibleEnabled);

    return r;
}

mu::engraving::Rest* Factory::createRest(mu::engraving::Segment* parent, const mu::engraving::TDuration& t, bool isAccessibleEnabled)
{
    Rest* r = new Rest(parent, t);
    r->setAccessibleEnabled(isAccessibleEnabled);

    return r;
}

mu::engraving::Rest* Factory::copyRest(const mu::engraving::Rest& src, bool link)
{
    Rest* copy = new Rest(src, link);
    copy->setAccessibleEnabled(src.accessibleEnabled());

    return copy;
}

mu::engraving::Segment* Factory::createSegment(mu::engraving::Measure* parent, bool isAccessibleEnabled)
{
    Segment* s = new Segment(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

mu::engraving::Segment* Factory::createSegment(mu::engraving::Measure* parent, mu::engraving::SegmentType type,
                                               const mu::engraving::Fraction& t, bool isAccessibleEnabled)
{
    Segment* s = new Segment(parent, type, t);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

CREATE_ITEM_IMPL(Slide, ElementType::SLIDE, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(Slide)
MAKE_ITEM_IMPL(Slide, Chord)

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

StaffText* Factory::createStaffText(Segment * parent, mu::engraving::TextStyleType textStyleType, bool isAccessibleEnabled)
{
    StaffText* staffText = new StaffText(parent, textStyleType);
    staffText->setAccessibleEnabled(isAccessibleEnabled);

    return staffText;
}

CREATE_ITEM_IMPL(RehearsalMark, ElementType::REHEARSAL_MARK, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Stem, ElementType::STEM, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(Stem)

mu::engraving::StemSlash* Factory::createStemSlash(mu::engraving::Chord * parent, bool isAccessibleEnabled)
{
    StemSlash* s = new StemSlash(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

COPY_ITEM_IMPL(StemSlash)

mu::engraving::System* Factory::createSystem(mu::engraving::Page * parent, bool isAccessibleEnabled)
{
    System* s = new System(parent);
    s->setAccessibleEnabled(isAccessibleEnabled);

    return s;
}

mu::engraving::SystemText* Factory::createSystemText(mu::engraving::Segment* parent, mu::engraving::TextStyleType textStyleType,
                                                     bool isAccessibleEnabled)
{
    SystemText* systemText = new SystemText(parent, textStyleType);
    systemText->setAccessibleEnabled(isAccessibleEnabled);

    return systemText;
}

CREATE_ITEM_IMPL(InstrumentChange, ElementType::INSTRUMENT_CHANGE, Segment, isAccessibleEnabled)

mu::engraving::InstrumentChange* Factory::createInstrumentChange(mu::engraving::Segment * parent, const Instrument& instrument,
                                                                 bool isAccessibleEnabled)
{
    InstrumentChange* instrumentChange = new InstrumentChange(instrument, parent);
    instrumentChange->setAccessibleEnabled(isAccessibleEnabled);

    return instrumentChange;
}

CREATE_ITEM_IMPL(Sticking, ElementType::STICKING, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Fingering, ElementType::FINGERING, Note, isAccessibleEnabled)

mu::engraving::Fingering* Factory::createFingering(mu::engraving::Note * parent, mu::engraving::TextStyleType textStyleType,
                                                   bool isAccessibleEnabled)
{
    Fingering* fingering = new Fingering(parent, textStyleType);
    fingering->setAccessibleEnabled(isAccessibleEnabled);

    return fingering;
}

CREATE_ITEM_IMPL(Harmony, ElementType::HARMONY, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(TempoText, ElementType::TEMPO_TEXT, Segment, isAccessibleEnabled)

mu::engraving::Text* Factory::createText(mu::engraving::EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Text* t = new Text(parent, tid);
    t->setAccessibleEnabled(isAccessibleEnabled);

    return t;
}

COPY_ITEM_IMPL(Text)

CREATE_ITEM_IMPL(Tie, ElementType::TIE, EngravingItem, isAccessibleEnabled)
COPY_ITEM_IMPL(Tie)

CREATE_ITEM_IMPL(TimeSig, ElementType::TIMESIG, Segment, isAccessibleEnabled)
COPY_ITEM_IMPL(TimeSig)
MAKE_ITEM_IMPL(TimeSig, Segment)

CREATE_ITEM_IMPL(Tremolo, ElementType::TREMOLO, Chord, isAccessibleEnabled)
COPY_ITEM_IMPL(Tremolo)
MAKE_ITEM_IMPL(Tremolo, Chord)

CREATE_ITEM_IMPL(TremoloBar, ElementType::TREMOLOBAR, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(TremoloBar, EngravingItem)

CREATE_ITEM_IMPL(Tuplet, ElementType::TUPLET, Measure, isAccessibleEnabled)
COPY_ITEM_IMPL(Tuplet)

CREATE_ITEM_IMPL(Hairpin, ElementType::HAIRPIN, Segment, isAccessibleEnabled)
MAKE_ITEM_IMPL(Hairpin, Segment)

CREATE_ITEM_IMPL(Glissando, ElementType::GLISSANDO, EngravingItem, isAccessibleEnabled)
MAKE_ITEM_IMPL(Glissando, EngravingItem)

CREATE_ITEM_IMPL(Jump, ElementType::JUMP, Measure, isAccessibleEnabled)

CREATE_ITEM_IMPL(Trill, ElementType::TRILL, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Vibrato, ElementType::VIBRATO, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(TextLine, ElementType::TEXTLINE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Ottava, ElementType::OTTAVA, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(LetRing, ElementType::LET_RING, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Marker, ElementType::MARKER, EngravingItem, isAccessibleEnabled)

mu::engraving::Marker* Factory::createMarker(mu::engraving::EngravingItem * parent, TextStyleType tid, bool isAccessibleEnabled)
{
    Marker* m = new Marker(parent, tid);
    m->setAccessibleEnabled(isAccessibleEnabled);

    return m;
}

CREATE_ITEM_IMPL(TempoChangeRanged, ElementType::TEMPO_RANGED_CHANGE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(PalmMute, ElementType::PALM_MUTE, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(WhammyBar, ElementType::WHAMMY_BAR, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Rasgueado, ElementType::RASGUEADO, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(HarmonicMark, ElementType::HARMONIC_MARK, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Volta, ElementType::VOLTA, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Pedal, ElementType::PEDAL, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(Dynamic, ElementType::DYNAMIC, Segment, isAccessibleEnabled)

CREATE_ITEM_IMPL(Harmony, ElementType::HARMONY, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(MMRest, ElementType::MMREST, EngravingItem, isAccessibleEnabled)

CREATE_ITEM_IMPL(VBox, ElementType::VBOX, System, isAccessibleEnabled)

mu::engraving::VBox* Factory::createVBox(const ElementType& type, System * parent, bool isAccessibleEnabled)
{
    VBox* b = new VBox(type, parent);
    b->setAccessibleEnabled(isAccessibleEnabled);

    return b;
}

CREATE_ITEM_IMPL(HBox, ElementType::HBOX, System, isAccessibleEnabled)

CREATE_ITEM_IMPL(TBox, ElementType::TBOX, System, isAccessibleEnabled)

CREATE_ITEM_IMPL(FBox, ElementType::FBOX, System, isAccessibleEnabled)
