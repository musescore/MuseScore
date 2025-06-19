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

#include "../xmlwriter.h"
#include "writecontext.h"

#include "../../dom/property.h"

namespace mu::engraving {
class EngravingItem;
class ElementList;

class Accidental;
class ActionIcon;
class Ambitus;
class Arpeggio;
class Articulation;
class Audio;

class BagpipeEmbellishment;
class BarLine;
class Beam;
class Bend;
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

class DurationElement;
class Dynamic;
class Expression;
class Fermata;
class FiguredBass;
class FiguredBassItem;
class Fingering;
class FretDiagram;

class Glissando;
class GradualTempoChange;
class Groups;
class GuitarBend;
class GuitarBendSegment;

class Hairpin;
class HammerOnPullOff;
class HammerOnPullOffSegment;
class HammerOnPullOffText;
class Harmony;
class HarmonicMark;
class HarpPedalDiagram;
class Hook;

class Image;
class Instrument;
class InstrChannel;
class InstrumentChange;

class Jump;

class KeySig;

class LayoutBreak;
class LedgerLine;
class LetRing;
class Location;
class Lyrics;

class Marker;
class MeasureNumber;
class MeasureRepeat;
struct MidiArticulation;
class MMRest;
class MMRestRange;

struct NamedEventList;
class Note;
class NoteEvent;
class NoteDot;
class NoteHead;
class NoteLine;
class Ornament;
class Ottava;

class Page;
class PalmMute;
class Part;
class Pedal;
class PickScrape;
class PlayTechAnnotation;

class Rasgueado;
class RehearsalMark;
class Rest;

class Segment;
class Slur;
class SlurTie;
class SlurTieSegment;
class SLine;
class Spanner;
class Spacer;
class Staff;
class StaffName;
class StaffNameList;
class StaffState;
class StaffText;
class StaffTextBase;
class StaffType;
class StaffTypeChange;
class Stem;
class StemSlash;
class Sticking;
class StringData;
class Symbol;
class BSymbol;
class FSymbol;
class StringTunings;
class System;
class SystemDivider;
class SystemText;
class SoundFlag;

class Tapping;
class TappingHalfSlur;
class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineBase;
class Tie;
class TimeSig;
class TremoloSingleChord;
class TremoloTwoChord;
class TremoloBar;
class Trill;
class Tuplet;

class Vibrato;
class Volta;

class WhammyBar;
}

namespace mu::engraving::write {
class TWrite
{
public:
    TWrite() = default;

    static void writeItem(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx);
    static void writeItems(const ElementList& items, XmlWriter& xml, WriteContext& ctx);

    static void write(const Accidental* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const ActionIcon* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Ambitus* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Arpeggio* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Articulation* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Audio* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const BagpipeEmbellishment* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const BarLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Beam* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Bend* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Box* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const HBox* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const VBox* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const FBox* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TBox* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Bracket* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Breath* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Chord* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const ChordLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Clef* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Capo* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Dynamic* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Expression* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Fermata* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const FiguredBass* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const FiguredBassItem* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Fingering* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const FretDiagram* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Glissando* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const GradualTempoChange* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Groups* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const GuitarBend* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Hairpin* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const HammerOnPullOff* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Harmony* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const HarmonicMark* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const HarpPedalDiagram* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Hook* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Image* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Instrument* item, XmlWriter& xml, WriteContext& ctx, const Part* part);
    static void write(const InstrChannel* item, XmlWriter& xml, const Part* part);
    static void write(const InstrumentChange* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Jump* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const KeySig* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const LaissezVib* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const LayoutBreak* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const LedgerLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const LetRing* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Location* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Lyrics* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Marker* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const MeasureNumber* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const MeasureRepeat* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const MidiArticulation* item, XmlWriter& xml);
    static void write(const MMRest* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const MMRestRange* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const NamedEventList* item, XmlWriter& xml, const AsciiStringView& n);
    static void write(const Note* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteEvent* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteDot* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteHead* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const NoteLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Ornament* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Ottava* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Page* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PalmMute* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Parenthesis* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Part* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PartialTie* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PartialLyricsLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Pedal* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PickScrape* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const PlayTechAnnotation* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Rasgueado* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const RehearsalMark* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Rest* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Segment* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Slur* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Spacer* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Staff* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffName* item, XmlWriter& xml, const char* tag);
    static void write(const StaffNameList* item, XmlWriter& xml, const char* name);
    static void write(const StaffState* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffText* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffType* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StaffTypeChange* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Stem* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StemSlash* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Sticking* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const StringData* item, XmlWriter& xml);
    static void write(const StringTunings* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Symbol* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const FSymbol* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const System* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const SystemDivider* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const SystemText* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const SoundFlag* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Tapping* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TappingHalfSlur* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TempoText* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Text* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TextLine* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Tie* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TimeSig* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TremoloSingleChord* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TremoloTwoChord* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const TremoloBar* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Trill* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Tuplet* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const Vibrato* item, XmlWriter& xml, WriteContext& ctx);
    static void write(const Volta* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const WhammyBar* item, XmlWriter& xml, WriteContext& ctx);

    static void writeSegments(XmlWriter& xml, WriteContext& ctx, track_idx_t st, track_idx_t et, Segment* sseg, Segment* eseg, bool, bool);

    static void writeProperty(const EngravingItem* item, XmlWriter& xml, Pid pid, bool force = false);

    static void writeSystemLocks(const Score* score, XmlWriter& xml);

    static void writeItemEid(const EngravingObject* item, XmlWriter& xml, WriteContext& ctx);
    static void writeItemLink(const EngravingObject* item, XmlWriter& xml, WriteContext& ctx);

private:

    static void writeStyledProperties(const EngravingItem* item, XmlWriter& xml);

    static void writeItemProperties(const EngravingItem* item, XmlWriter& xml, WriteContext& ctx);
    static void writeBoxProperties(const Box* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const Articulation* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const Box* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const HBox* item, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const ChordRest* item, XmlWriter& xml, WriteContext& ctx);
    static void writeChordRestBeam(const ChordRest* item, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const Rest* item, XmlWriter& xml, WriteContext& ctx);

    static void write(const StaffTextBase* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const StaffTextBase* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const SlurTie* item, XmlWriter& xml, WriteContext& ctx);
    static void writeSlur(const SlurTieSegment* seg, XmlWriter& xml, WriteContext& ctx, int no);

    static void writeProperties(const HammerOnPullOffSegment* seg, XmlWriter& xml, WriteContext& ctx);
    static void write(const HammerOnPullOffText* item, XmlWriter& xml, WriteContext& ctx, size_t idx);

    static void writeProperties(const SLine* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const GuitarBendSegment* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const Spanner* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const BSymbol* item, XmlWriter& xml, WriteContext& ctx);

    static void writeProperties(const TextLineBase* item, XmlWriter& xml, WriteContext& ctx);
    static void writeProperties(const TextBase* item, XmlWriter& xml, WriteContext& ctx, bool writeText);

    static void writeSpannerStart(Spanner* s, XmlWriter& xml, WriteContext& ctx, const EngravingItem* current, track_idx_t track,
                                  Fraction frac = { -1, 1 });
    static void writeSpannerEnd(Spanner* s, XmlWriter& xml, WriteContext& ctx, const EngravingItem* current, track_idx_t track,
                                Fraction frac = { -1, 1 });

    static void writeTupletStart(DurationElement* item, XmlWriter& xml, WriteContext& ctx);
    static void writeTupletEnd(DurationElement* item, XmlWriter& xml, WriteContext& ctx);

    static void writeSystemLock(const SystemLock* systemLock, XmlWriter& xml);
};
}
