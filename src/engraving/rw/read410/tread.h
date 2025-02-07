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
#pragma once

#include "global/types/string.h"

#include "../xmlreader.h"
#include "readcontext.h"

#include "../../dom/property.h"

namespace mu::engraving {
class XmlReader;
class EngravingItem;
class TextBase;
class TempoText;
class Dynamic;
class Expression;
class FretDiagram;
class Sticking;
class SystemText;
class RehearsalMark;
class Fermata;
class Image;
class Ambitus;
class Accidental;
class MMRestRange;
class ActionIcon;
class Arpeggio;
class Articulation;
class Audio;
class BagpipeEmbellishment;
class BarLine;
class Beam;
class Bend;
class StretchedBend;
class Box;
class HBox;
class VBox;
class FBox;
class TBox;
class Bracket;
class Breath;

class Chord;
class ChordRest;
class ChordLine;
class Clef;
class Capo;

class Excerpt;

class FiguredBass;
class FiguredBassItem;
class Fingering;

class Glissando;
class GradualTempoChange;
class Groups;
class GuitarBend;
class GuitarBendSegment;
class GuitarBendHold;

class Hairpin;
class Harmony;
class HarmonicMark;
class HarpPedalDiagram;
class Hook;

class Instrument;
class InstrChannel;
class InstrumentChange;

class Jump;

class KeyList;
class KeySig;

class LayoutBreak;
class LedgerLine;
class LetRing;
class LineSegment;
class Location;
class Lyrics;

class Marker;
class MeasureBase;
class MeasureNumber;
class MeasureNumberBase;
class MeasureRepeat;
struct MidiArticulation;
class MMRest;

struct NamedEventList;
class Note;
class NoteEvent;
class NoteDot;
class NoteHead;
class Ornament;
class Ottava;

class Page;
class PalmMute;
class Part;
class Pedal;
class PlayTechAnnotation;

class Rasgueado;
class Rest;

class Segment;
class SLine;
class Slur;
class SlurTie;
class SlurTieSegment;
class Spanner;
class Spacer;
class Staff;
class StaffName;
class StaffState;
class StaffText;
class StaffTextBase;
class StaffType;
class StaffTypeChange;
class Stem;
class StemSlash;
class StringData;
class StringTunings;
class System;
class SystemDivider;
class Symbol;
class SoundFlag;
class BSymbol;
class FSymbol;

class Text;
class TextLine;
class TextLineBase;
class Tie;
class TimeSig;
class TimeSigMap;
class SigEvent;
class TremoloSingleChord;
class TremoloTwoChord;
class TremoloBar;
class Trill;
class Tuplet;
class Vibrato;
class Volta;
}

namespace mu::engraving::compat {
struct TremoloCompat;
}

namespace mu::engraving::read410 {
class TRead
{
public:
    TRead() = default;

    // factory
    static void readItem(EngravingItem* el, XmlReader& xml, ReadContext& ctx);

    // types

    static void read(TextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(TempoText* t, XmlReader& xml, ReadContext& ctx);
    static void read(Dynamic* d, XmlReader& xml, ReadContext& ctx);
    static void read(SystemText* t, XmlReader& xml, ReadContext& ctx);
    static void read(RehearsalMark* m, XmlReader& xml, ReadContext& ctx);
    static void read(Image* i, XmlReader& xml, ReadContext& ctx);
    static void read(Ambitus* a, XmlReader& xml, ReadContext& ctx);
    static void read(Accidental* a, XmlReader& xml, ReadContext& ctx);
    static void read(Jump* j, XmlReader& xml, ReadContext& ctx);
    static void read(MMRestRange* r, XmlReader& xml, ReadContext& ctx);
    static void read(ActionIcon* i, XmlReader& xml, ReadContext& ctx);
    static void read(Arpeggio* a, XmlReader& xml, ReadContext& ctx);
    static void read(Articulation* a, XmlReader& xml, ReadContext& ctx);
    static void read(Audio* a, XmlReader& xml, ReadContext& ctx);

    static void read(BagpipeEmbellishment* b, XmlReader& xml, ReadContext& ctx);
    static void read(BarLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(Beam* b, XmlReader& xml, ReadContext& ctx);
    static void read(Bend* b, XmlReader& xml, ReadContext& ctx);
    static void read(StretchedBend* b, XmlReader& xml, ReadContext& ctx);
    static void read(Box* b, XmlReader& xml, ReadContext& ctx);
    static void read(HBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(VBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(FBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(TBox* b, XmlReader& xml, ReadContext& ctx);
    static void read(Bracket* b, XmlReader& xml, ReadContext& ctx);
    static void read(Breath* b, XmlReader& xml, ReadContext& ctx);

    static void read(Chord* ch, XmlReader& xml, ReadContext& ctx);
    static void read(ChordLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(Clef* c, XmlReader& xml, ReadContext& ctx);
    static void read(Capo* c, XmlReader& xml, ReadContext& ctx);

    static void read(Excerpt* item, XmlReader& xml, ReadContext& ctx);
    static void read(Expression* item, XmlReader& xml, ReadContext& ctx);

    static void read(Fermata* f, XmlReader& xml, ReadContext& ctx);
    static void read(FiguredBass* b, XmlReader& xml, ReadContext& ctx);
    static void read(FiguredBassItem* i, XmlReader& xml, ReadContext& ctx);
    static void read(Fingering* f, XmlReader& xml, ReadContext& ctx);
    static void read(FretDiagram* d, XmlReader& xml, ReadContext& ctx);

    static void read(Glissando* g, XmlReader& xml, ReadContext& ctx);
    static void read(GradualTempoChange* c, XmlReader& xml, ReadContext& ctx);
    static void read(Groups* g, XmlReader& xml, ReadContext& ctx);
    static void read(GuitarBend* g, XmlReader& xml, ReadContext& ctx);
    static void read(GuitarBendHold* h, XmlReader& xml, ReadContext& ctx);

    static void read(Hairpin* h, XmlReader& xml, ReadContext& ctx);
    static void read(Harmony* h, XmlReader& xml, ReadContext& ctx);
    static void read(HarpPedalDiagram* h, XmlReader& xml, ReadContext& ctx);
    static void read(HarmonicMark* h, XmlReader& xml, ReadContext& ctx);
    static void read(Hook* h, XmlReader& xml, ReadContext& ctx);

    static void read(Instrument* item, XmlReader& xml, ReadContext& ctx, Part* part);
    static void read(InstrChannel* item, XmlReader& e, ReadContext& ctx, Part* part, const InstrumentTrackId& instrId);
    static void read(InstrumentChange* c, XmlReader& xml, ReadContext& ctx);

    static void read(KeyList* item, XmlReader& xml, ReadContext& ctx);
    static void read(KeySig* s, XmlReader& xml, ReadContext& ctx);

    static void read(LaissezVib* lv, XmlReader& xml, ReadContext& ctx);
    static void read(LayoutBreak* b, XmlReader& xml, ReadContext& ctx);
    static void read(LedgerLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(LetRing* r, XmlReader& xml, ReadContext& ctx);
    static void read(LineSegment* l, XmlReader& xml, ReadContext& ctx);
    static void read(Location* l, XmlReader& xml, ReadContext& ctx);
    static void read(Lyrics* l, XmlReader& xml, ReadContext& ctx);

    static void read(Marker* m, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumber* n, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureNumberBase* b, XmlReader& xml, ReadContext& ctx);
    static void read(MeasureRepeat* r, XmlReader& xml, ReadContext& ctx);
    static void read(MidiArticulation* item, XmlReader& xml);
    static void read(MMRest* r, XmlReader& xml, ReadContext& ctx);

    static void read(Note* n, XmlReader& xml, ReadContext& ctx);
    static void read(NoteEvent* item, XmlReader& xml, ReadContext& ctx);
    static void read(NoteDot* d, XmlReader& xml, ReadContext& ctx);
    static void read(NoteHead* h, XmlReader& xml, ReadContext& ctx);
    static void read(NoteLine* nl, XmlReader& xml, ReadContext& ctx);
    static void read(Ornament* o, XmlReader& xml, ReadContext& ctx);
    static void read(Ottava* o, XmlReader& xml, ReadContext& ctx);

    static void read(Page* p, XmlReader& xml, ReadContext& ctx);
    static void read(PalmMute* p, XmlReader& xml, ReadContext& ctx);
    static void read(Parenthesis* p, XmlReader& xml, ReadContext& ctx);
    static void read(Part* p, XmlReader& xml, ReadContext& ctx);
    static void read(PartialLyricsLine* p, XmlReader& xml, ReadContext& ctx);
    static void read(PartialTie* p, XmlReader& xml, ReadContext& ctx);
    static void read(Pedal* p, XmlReader& xml, ReadContext& ctx);
    static void read(PlayTechAnnotation* a, XmlReader& xml, ReadContext& ctx);

    static void read(Rasgueado* r, XmlReader& xml, ReadContext& ctx);
    static void read(Rest* r, XmlReader& xml, ReadContext& ctx);

    static void read(Segment* s, XmlReader& xml, ReadContext& ctx);
    static void read(SLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(Slur* s, XmlReader& xml, ReadContext& ctx);
    static void read(SlurTie* s, XmlReader& xml, ReadContext& ctx);
    static void read(SlurTieSegment* s, XmlReader& xml, ReadContext& ctx);
    static void read(Spacer* s, XmlReader& xml, ReadContext& ctx);
    static void read(Staff* s, XmlReader& xml, ReadContext& ctx);
    static void read(StaffName* item, XmlReader& xml);
    static void read(StaffState* s, XmlReader& xml, ReadContext& ctx);
    static void read(StaffText* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffType* t, XmlReader& xml, ReadContext& ctx);
    static void read(StaffTypeChange* c, XmlReader& xml, ReadContext& ctx);
    static void read(Sticking* s, XmlReader& xml, ReadContext& ctx);
    static void read(Stem* s, XmlReader& xml, ReadContext& ctx);
    static void read(StemSlash* s, XmlReader& xml, ReadContext& ctx);
    static void read(StringData* item, XmlReader& xml);
    static void read(StringTunings* s, XmlReader& xml, ReadContext& ctx);
    static void read(System* s, XmlReader& xml, ReadContext& ctx);
    static void read(SystemDivider* d, XmlReader& xml, ReadContext& ctx);
    static void read(Symbol* sym, XmlReader& xml, ReadContext& ctx);
    static void read(SoundFlag* sym, XmlReader& xml, ReadContext& ctx);
    static void read(FSymbol* sym, XmlReader& xml, ReadContext& ctx);

    static void read(Text* t, XmlReader& xml, ReadContext& ctx);
    static void read(TextLine* l, XmlReader& xml, ReadContext& ctx);
    static void read(TextLineBase* b, XmlReader& xml, ReadContext& ctx);
    static void read(Tie* t, XmlReader& xml, ReadContext& ctx);
    static void read(TimeSig* s, XmlReader& xml, ReadContext& ctx);
    static void read(TimeSigMap* item, XmlReader& xml, ReadContext& ctx);
    static void read(TremoloTwoChord* t, XmlReader& xml, ReadContext& ctx);
    static void read(TremoloSingleChord* t, XmlReader& xml, ReadContext& ctx);

    static void read(TremoloBar* b, XmlReader& xml, ReadContext& ctx);
    static void read(Trill* t, XmlReader& xml, ReadContext& ctx);
    static void read(Tuplet* t, XmlReader& xml, ReadContext& ctx);
    static void read(Vibrato* v, XmlReader& xml, ReadContext& ctx);
    static void read(Volta* v, XmlReader& xml, ReadContext& ctx);

    // compat
    static void read(compat::TremoloCompat* tc, XmlReader& xml, ReadContext& ctx);

    // temp compat

    static PropertyValue readPropertyValue(Pid type, XmlReader& e, ReadContext& ctx);
    static bool readProperty(EngravingItem* item, const AsciiStringView&, XmlReader&, ReadContext&, Pid);
    static void readProperty(EngravingItem* item, XmlReader&, ReadContext&, Pid);
    static bool readStyledProperty(EngravingItem* item, const AsciiStringView& tag, XmlReader& xml, ReadContext& ctx);

    static bool readItemProperties(EngravingItem* item, XmlReader& xml, ReadContext& ctx);
    static bool readBoxProperties(Box* b, XmlReader& xml, ReadContext& ctx);
    static bool readTextProperties(TextBase* t, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(Ambitus* a, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Articulation* a, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(BSymbol* sym, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Chord* ch, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(ChordRest* ch, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Clef* c, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Fermata* f, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(GuitarBendSegment* g, const AsciiStringView& tag, XmlReader& xml, ReadContext&);

    static bool readProperties(Instrument* item, XmlReader& xml, ReadContext& ctx, Part* part, bool* customDrumset);

    static bool readProperties(LedgerLine* l, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(LineSegment* l, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Lyrics* l, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(MMRest* r, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(MeasureNumberBase* r, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(MeasureBase* b, XmlReader& xml, ReadContext& ctx);

    static void read(NamedEventList* item, XmlReader& xml);
    static bool readProperties(Note* n, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(Ornament* o, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Ottava* o, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(Part* p, XmlReader& xml, ReadContext& ctx);

    static int read(SigEvent* item, XmlReader& xml, int fileDivision);
    static bool readProperties(SLine* l, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Slur* s, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(SlurTie* s, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Spanner* s, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(Staff* s, XmlReader& e, ReadContext& ctx);
    static bool readProperties(Stem* s, XmlReader& e, ReadContext& ctx);

    static bool readProperties(TextLineBase* b, XmlReader& e, ReadContext& ctx);
    static bool readProperties(Volta* v, XmlReader& e, ReadContext& ctx);

    static void readSpanner(XmlReader& e, ReadContext& ctx, EngravingItem* current, track_idx_t track);
    static void readSpanner(XmlReader& e, ReadContext& ctx, Score* current, track_idx_t track);

    static void readSystemLocks(Score* score, XmlReader& e);

private:
    static bool readProperties(Box* b, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(HBox* b, XmlReader& xml, ReadContext& ctx);

    static bool readProperties(TextBase* t, XmlReader& xml, ReadContext& ctx);
    static bool readProperties(StaffTextBase* t, XmlReader& xml, ReadContext& ctx);

    static void readSystemLock(Score* score, XmlReader& e);
};
}
