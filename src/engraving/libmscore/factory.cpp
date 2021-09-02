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

#include "log.h"

using namespace mu::engraving;
using namespace Ms;

struct ElementName {
    ElementType type;
    const char* name;
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
    { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       QT_TRANSLATE_NOOP("elementName", "Vibrato segment") },
    { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      QT_TRANSLATE_NOOP("elementName", "Palm mute segment") },
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
    { ElementType::VIBRATO,              "Vibrato",              QT_TRANSLATE_NOOP("elementName", "Vibrato") },
    { ElementType::PALM_MUTE,            "PalmMute",             QT_TRANSLATE_NOOP("elementName", "Palm mute") },
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
    { ElementType::STICKING,             "Sticking",             QT_TRANSLATE_NOOP("elementName", "Sticking") }
};
/* *INDENT-ON* */

EngravingItem* Factory::createItem(ElementType type, EngravingItem* parent)
{
    auto dummy = parent->score()->dummy();
    switch (type) {
    case ElementType::VOLTA:             return new Volta(parent);
    case ElementType::OTTAVA:            return new Ottava(parent);
    case ElementType::TEXTLINE:          return new TextLine(parent);
    case ElementType::NOTELINE:          return new NoteLine(parent);
    case ElementType::TRILL:             return new Trill(parent);
    case ElementType::LET_RING:          return new LetRing(parent);
    case ElementType::VIBRATO:           return new Vibrato(parent);
    case ElementType::PALM_MUTE:         return new PalmMute(parent);
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
    case ElementType::ACCIDENTAL:        return new Accidental(parent);
    case ElementType::DYNAMIC:           return new Dynamic(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::TEXT:              return new Text(parent);
    case ElementType::MEASURE_NUMBER:    return new MeasureNumber(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::MMREST_RANGE:      return new MMRestRange(parent->isMeasure() ? toMeasure(parent) : dummy->measure());
    case ElementType::INSTRUMENT_NAME:   return new InstrumentName(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::STAFF_TEXT:        return new StaffText(parent->isSegment() ? toSegment(parent) : dummy->segment());
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
    case ElementType::LYRICS:            return new Lyrics(parent);
    case ElementType::FIGURED_BASS:      return new FiguredBass(parent->isSegment() ? toSegment(parent) : dummy->segment());
    case ElementType::STEM:              return new Stem(parent->isChord() ? toChord(parent) : dummy->chord());
    case ElementType::SLUR:              return new Slur(parent);
    case ElementType::TIE:               return new Tie(parent);
    case ElementType::FINGERING:         return new Fingering(parent->isNote() ? toNote(parent) : dummy->note());
    case ElementType::HBOX:              return new HBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::VBOX:              return new VBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::TBOX:              return new TBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::FBOX:              return new FBox(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::MEASURE:           return new Measure(parent->isSystem() ? toSystem(parent) : dummy->system());
    case ElementType::TAB_DURATION_SYMBOL: return new TabDurationSymbol(parent->isChordRest() ? toChordRest(parent) : dummy->chord());
    case ElementType::OSSIA:               return new Ossia(parent);
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
    case ElementType::TUPLET:
    case ElementType::HAIRPIN_SEGMENT:
    case ElementType::OTTAVA_SEGMENT:
    case ElementType::TRILL_SEGMENT:
    case ElementType::LET_RING_SEGMENT:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::PALM_MUTE_SEGMENT:
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
        break;
    }
    qDebug("cannot create type %d <%s>", int(type), Factory::name(type));
    return 0;
}

EngravingItem* Factory::createItemByName(const QStringRef& name, EngravingItem* parent)
{
    ElementType type = name2type(name, true);
    if (type == ElementType::INVALID) {
        LOGE() << "Invalide type: " << name.toString();
        return 0;
    }
    return createItem(type, parent);
}

ElementType Factory::name2type(const QStringRef& name, bool silent)
{
    for (int i = 0; i < int(ElementType::MAXTYPE); ++i) {
        if (name == elementNames[i].name) {
            return ElementType(i);
        }
    }
    if (!silent) {
        LOGE() << "Unknown type: " << name.toString();
    }
    return ElementType::INVALID;
}

const char* Factory::name(ElementType type)
{
    return elementNames[int(type)].name;
}

const char* Factory::userName(Ms::ElementType type)
{
    return elementNames[int(type)].userName;
}
