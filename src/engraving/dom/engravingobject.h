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

#include "global/allocator.h"
#include "types/string.h"

#include "draw/types/geometry.h"
#include "modularity/ioc.h"
#include "../devtools/iengravingelementsprovider.h"

#include "../style/styledef.h"

#include "../types/propertyvalue.h"
#include "../types/types.h"

#include "../infrastructure/rtti.h"
#include "../infrastructure/eid.h"

namespace mu::engraving {
class Accidental;
class ActionIcon;
class Ambitus;
class Arpeggio;
class Articulation;
class BSymbol;
class BagpipeEmbellishment;
class BarLine;
class Beam;
class BeamSegment;
class Bend;
class Box;
class Bracket;
class BracketItem;
class Breath;
class Chord;
class ChordLine;
class ChordRest;
class Clef;
class Capo;
class DeadSlapped;
class DurationElement;
class Dynamic;
class Expression;
class EngravingItem;
class EngravingObject;
class FBox;
class FSymbol;
class Fermata;
class FiguredBass;
class FiguredBassItem;
class Fingering;
class FretDiagram;
class Glissando;
class GlissandoSegment;
class GraceNotesGroup;
class GradualTempoChange;
class GradualTempoChangeSegment;
class GuitarBend;
class GuitarBendSegment;
class GuitarBendHold;
class GuitarBendHoldSegment;
class GuitarBendText;
class HBox;
class Hairpin;
class HairpinSegment;
class HarmonicMark;
class HarmonicMarkSegment;
class Harmony;
class HarpPedalDiagram;
class Hook;
class Image;
class InstrumentChange;
class InstrumentName;
class Jump;
class KeySig;
class Lasso;
class LayoutBreak;
class LaissezVib;
class LaissezVibSegment;
class LedgerLine;
class LetRing;
class LetRingSegment;
class LineSegment;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;
class MMRest;
class MMRestRange;
class Marker;
class MasterScore;
class Measure;
class MeasureBase;
class MeasureNumber;
class MeasureRepeat;
class MStyle;
class Note;
class NoteDot;
class NoteHead;
class NoteLine;
class NoteLineSegment;
class Ornament;
class Ottava;
class OttavaSegment;
class Page;
class PalmMute;
class PalmMuteSegment;
class Parenthesis;
class Part;
class PartialLyricsLine;
class PartialLyricsLineSegment;
class PartialTie;
class PartialTieSegment;
class Pedal;
class PedalSegment;
class PickScrape;
class PickScrapeSegment;
class PlayTechAnnotation;
class Rasgueado;
class RasgueadoSegment;
class RehearsalMark;
class Rest;
class Score;
class Segment;
class Slur;
class SlurSegment;
class SlurTieSegment;
class Spacer;
class Spanner;
class SpannerSegment;
class Staff;
class StaffLines;
class StaffState;
class StaffText;
class StaffTextBase;
class StaffTypeChange;
class Stem;
class StemSlash;
class Sticking;
class StringTunings;
class Symbol;
class System;
class SystemDivider;
class SystemLockIndicator;
class SystemText;
class SoundFlag;
class TBox;
class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineBase;
class TextLineBaseSegment;
class TextLineSegment;
class Tie;
class TieSegment;
class TimeSig;
class TimeTickAnchor;
class TremoloBar;
class Trill;
class TrillSegment;
class TripletFeel;
class Tuplet;
class VBox;
class Vibrato;
class VibratoSegment;
class Volta;
class VoltaSegment;
class WhammyBar;
class WhammyBarSegment;
class FretCircle;
class ShadowNote;

class LinkedObjects;

enum class Pid : int;
enum class PropertyFlags : char;

class EngravingObjectList : public std::list<EngravingObject*>
{
    OBJECT_ALLOCATOR(engraving, EngravingObjectList)
public:

    EngravingObject* at(size_t i) const;
};

class EngravingObject
{
public:
    EngravingObject(const ElementType& type, EngravingObject* parent);
    EngravingObject(const EngravingObject& se);

    virtual ~EngravingObject();

    inline ElementType type() const { return m_type; }
    inline bool isType(ElementType t) const { return t == m_type; }
    const char* typeName() const;
    virtual TranslatableString typeUserName() const;
    virtual String translatedTypeUserName() const;

    EID eid() const;
    void setEID(EID id) const;
    EID assignNewEID() const;

    EngravingObject* parent() const;
    void setParent(EngravingObject* p);
    EngravingObject* explicitParent() const;
    void resetExplicitParent();
    void moveToDummy();

    const EngravingObjectList& children() const { return m_children; }

    // Score Tree functions for scan function
    friend class EngravingElementsProvider;
    virtual EngravingObject* scanParent() const { return m_parent; }
    virtual EngravingObjectList scanChildren() const { return {}; }
    virtual void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true);

    // context
    virtual void setScore(Score* s);
    Score* score() const;
    MasterScore* masterScore() const;
    bool onSameScore(const EngravingObject* other) const;
    const MStyle& style() const;

    virtual PropertyValue getProperty(Pid) const = 0;
    virtual bool setProperty(Pid, const PropertyValue&) = 0;
    virtual PropertyValue propertyDefault(Pid) const;
    virtual void resetProperty(Pid id);
    virtual bool sizeIsSpatiumDependent() const { return true; }
    virtual bool offsetIsSpatiumDependent() const { return true; }

    virtual void reset();                       // reset all properties & position to default

    virtual void initElementStyle(const ElementStyle*);
    virtual const ElementStyle* styledProperties() const { return m_elementStyle; }

    virtual PropertyFlags* propertyFlagsList() const { return m_propertyFlagsList; }
    virtual PropertyFlags propertyFlags(Pid) const;
    bool isStyled(Pid pid) const;
    PropertyValue styleValue(Pid, Sid) const;

    virtual void setPropertyFlags(Pid, PropertyFlags);

    virtual Sid getPropertyStyle(Pid) const;

    virtual void styleChanged();

    virtual void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps);
    void undoChangeProperty(Pid id, const PropertyValue&);
    void undoResetProperty(Pid id);

    void undoPushProperty(Pid);

    std::list<EngravingObject*> linkList() const;

    void linkTo(EngravingObject*);
    void unlink();
    bool isLinked(EngravingObject* se = nullptr) const;

    virtual void undoUnlink();
    LinkedObjects* links() const { return m_links; }
    void setLinks(LinkedObjects* le);

protected:
    virtual void setParentInternal(EngravingObject* p);
    virtual int getPropertyFlagsIdx(Pid id) const;

    //! NOTE For compatibility reasons, hope, we will remove the need for this method.
    void hack_setType(const ElementType& t) { m_type = t; }

    void addChild(EngravingObject* o);
    void removeChild(EngravingObject* o);

    const ElementStyle* m_elementStyle { &EMPTY_STYLE };
    PropertyFlags* m_propertyFlagsList = nullptr;
    LinkedObjects* m_links = nullptr;
    Score* m_score = nullptr;

private:
    static ElementStyle const EMPTY_STYLE;

    void doSetParent(EngravingObject* p);
    void doSetScore(Score* sc);

    ElementType m_type = ElementType::INVALID;

    EngravingObject* m_parent = nullptr;
    bool m_isParentExplicitlySet = false;
    EngravingObjectList m_children;

public:

    virtual std::list<EngravingObject*> linkListForPropertyPropagation() const { return linkList(); }

    //---------------------------------------------------
    // check type
    //
    // Example for ChordRest:
    //
    //    bool             isChordRest()
    //---------------------------------------------------

#define CONVERT(a, b) \
    bool is##a() const { return type() == ElementType::b; \
    }

    CONVERT(Note,          NOTE)
    CONVERT(Rest,          REST)
    CONVERT(MMRest,        MMREST)
    CONVERT(DeadSlapped,   DEAD_SLAPPED)
    CONVERT(Chord,         CHORD)
    CONVERT(BarLine,       BAR_LINE)
    CONVERT(Articulation,  ARTICULATION)
    CONVERT(Ornament,      ORNAMENT)
    CONVERT(Fermata,       FERMATA)
    CONVERT(Marker,        MARKER)
    CONVERT(Clef,          CLEF)
    CONVERT(KeySig,        KEYSIG)
    CONVERT(TimeSig,       TIMESIG)
    CONVERT(Measure,       MEASURE)
    CONVERT(TempoText,     TEMPO_TEXT)
    CONVERT(Breath,        BREATH)
    CONVERT(HBox,          HBOX)
    CONVERT(VBox,          VBOX)
    CONVERT(TBox,          TBOX)
    CONVERT(FBox,          FBOX)
    CONVERT(Slur,          SLUR)
    CONVERT(Glissando,     GLISSANDO)
    CONVERT(GlissandoSegment,     GLISSANDO_SEGMENT)
    CONVERT(GuitarBend,    GUITAR_BEND)
    CONVERT(GuitarBendSegment, GUITAR_BEND_SEGMENT)
    CONVERT(GuitarBendHold, GUITAR_BEND_HOLD)
    CONVERT(GuitarBendHoldSegment, GUITAR_BEND_HOLD_SEGMENT)
    CONVERT(GuitarBendText, GUITAR_BEND_TEXT)
    CONVERT(SystemDivider, SYSTEM_DIVIDER)
    CONVERT(RehearsalMark, REHEARSAL_MARK)
    CONVERT(TripletFeel, TRIPLET_FEEL)
    CONVERT(Harmony,       HARMONY)
    CONVERT(Volta,         VOLTA)
    CONVERT(Jump,          JUMP)
    CONVERT(Ottava,        OTTAVA)
    CONVERT(LayoutBreak,   LAYOUT_BREAK)
    CONVERT(SystemLockIndicator, SYSTEM_LOCK_INDICATOR)
    CONVERT(Segment,       SEGMENT)
    CONVERT(System,        SYSTEM)
    CONVERT(Lyrics,        LYRICS)
    CONVERT(Stem,          STEM)
    CONVERT(Beam,          BEAM)
    CONVERT(BeamSegment,   BEAM_SEGMENT)
    CONVERT(Hook,          HOOK)
    CONVERT(StemSlash,     STEM_SLASH)
    CONVERT(SlurSegment,   SLUR_SEGMENT)
    CONVERT(LaissezVibSegment,    LAISSEZ_VIB_SEGMENT)
    CONVERT(LaissezVib,    LAISSEZ_VIB)
    CONVERT(PartialTieSegment,    PARTIAL_TIE_SEGMENT)
    CONVERT(PartialTie,    PARTIAL_TIE)
    CONVERT(Spacer,        SPACER)
    CONVERT(StaffLines,    STAFF_LINES)
    CONVERT(Ambitus,       AMBITUS)
    CONVERT(Bracket,       BRACKET)
    CONVERT(InstrumentChange, INSTRUMENT_CHANGE)
    CONVERT(StaffTypeChange, STAFFTYPE_CHANGE)
    CONVERT(Hairpin,       HAIRPIN)
    CONVERT(HairpinSegment, HAIRPIN_SEGMENT)
    CONVERT(Bend,          BEND)
    CONVERT(TremoloBar,    TREMOLOBAR)
    CONVERT(MeasureRepeat, MEASURE_REPEAT)
    CONVERT(Tuplet,        TUPLET)
    CONVERT(NoteDot,       NOTEDOT)
    CONVERT(Dynamic,       DYNAMIC)
    CONVERT(Expression,    EXPRESSION)
    CONVERT(InstrumentName, INSTRUMENT_NAME)
    CONVERT(Accidental,    ACCIDENTAL)
    CONVERT(TextLine,      TEXTLINE)
    CONVERT(TextLineSegment,      TEXTLINE_SEGMENT)
    CONVERT(Pedal,         PEDAL)
    CONVERT(PedalSegment,  PEDAL_SEGMENT)
    CONVERT(OttavaSegment, OTTAVA_SEGMENT)
    CONVERT(LedgerLine,    LEDGER_LINE)
    CONVERT(ActionIcon,    ACTION_ICON)
    CONVERT(VoltaSegment,  VOLTA_SEGMENT)
    CONVERT(NoteLine,      NOTELINE)
    CONVERT(NoteLineSegment,      NOTELINE_SEGMENT)
    CONVERT(Trill,         TRILL)
    CONVERT(TrillSegment,  TRILL_SEGMENT)
    CONVERT(LetRing,       LET_RING)
    CONVERT(LetRingSegment, LET_RING_SEGMENT)
    CONVERT(GradualTempoChange, GRADUAL_TEMPO_CHANGE)
    CONVERT(GradualTempoChangeSegment, GRADUAL_TEMPO_CHANGE_SEGMENT)
    CONVERT(Vibrato,       VIBRATO)
    CONVERT(PalmMute,      PALM_MUTE)
    CONVERT(PalmMuteSegment, PALM_MUTE_SEGMENT)
    CONVERT(WhammyBar,      WHAMMY_BAR)
    CONVERT(WhammyBarSegment, WHAMMY_BAR_SEGMENT)
    CONVERT(Rasgueado,      RASGUEADO)
    CONVERT(RasgueadoSegment, RASGUEADO_SEGMENT)
    CONVERT(HarmonicMark,      HARMONIC_MARK)
    CONVERT(HarmonicMarkSegment, HARMONIC_MARK_SEGMENT)
    CONVERT(PickScrape,      PICK_SCRAPE)
    CONVERT(PickScrapeSegment, PICK_SCRAPE_SEGMENT)
    CONVERT(VibratoSegment,  VIBRATO_SEGMENT)
    CONVERT(Symbol,        SYMBOL)
    CONVERT(FSymbol,       FSYMBOL)
    CONVERT(Fingering,     FINGERING)
    CONVERT(NoteHead,      NOTEHEAD)
    CONVERT(PartialLyricsLine,    PARTIAL_LYRICSLINE)
    CONVERT(PartialLyricsLineSegment, PARTIAL_LYRICSLINE_SEGMENT)
    CONVERT(FiguredBass,   FIGURED_BASS)
    CONVERT(FiguredBassItem, FIGURED_BASS_ITEM)
    CONVERT(StaffState,    STAFF_STATE)
    CONVERT(Arpeggio,      ARPEGGIO)
    CONVERT(Image,         IMAGE)
    CONVERT(ChordLine,     CHORDLINE)
    CONVERT(FretDiagram,   FRET_DIAGRAM)
    CONVERT(HarpPedalDiagram, HARP_DIAGRAM)
    CONVERT(Page,          PAGE)
    CONVERT(Text,          TEXT)
    CONVERT(MeasureNumber, MEASURE_NUMBER)
    CONVERT(MMRestRange,   MMREST_RANGE)
    CONVERT(StaffText,     STAFF_TEXT)
    CONVERT(SystemText,    SYSTEM_TEXT)
    CONVERT(SoundFlag,     SOUND_FLAG)
    CONVERT(PlayTechAnnotation, PLAYTECH_ANNOTATION)
    CONVERT(Capo,          CAPO)
    CONVERT(BracketItem,   BRACKET_ITEM)
    CONVERT(Score,         SCORE)
    CONVERT(Staff,         STAFF)
    CONVERT(Part,          PART)
    CONVERT(BagpipeEmbellishment, BAGPIPE_EMBELLISHMENT)
    CONVERT(Lasso,         LASSO)
    CONVERT(Sticking,      STICKING)
    CONVERT(GraceNotesGroup, GRACE_NOTES_GROUP)
    CONVERT(FretCircle, FRET_CIRCLE)
    CONVERT(StringTunings, STRING_TUNINGS)
    CONVERT(TimeTickAnchor, TIME_TICK_ANCHOR)
    CONVERT(Parenthesis, PARENTHESIS)
    CONVERT(ShadowNote, SHADOW_NOTE)
#undef CONVERT

    virtual bool isEngravingItem() const { return false; }   // overridden in element.h
    bool isRestFamily() const { return isRest() || isMMRest() || isMeasureRepeat(); }
    bool isChordRest() const { return isRestFamily() || isChord(); }
    bool isDurationElement() const { return isChordRest() || isTuplet(); }
    bool isSlurTieSegment() const { return isSlurSegment() || isTieSegment(); }
    bool isSLineSegment() const;
    bool isBox() const { return isVBox() || isHBox() || isTBox() || isFBox(); }
    bool isVBoxBase() const { return isVBox() || isTBox() || isFBox(); }
    bool isMeasureBase() const { return isMeasure() || isBox(); }
    bool isTextBase() const;
    bool isTextLineBaseSegment() const
    {
        return isHairpinSegment()
               || isLetRingSegment()
               || isGradualTempoChangeSegment()
               || isTextLineSegment()
               || isOttavaSegment()
               || isPalmMuteSegment()
               || isPickScrapeSegment()
               || isWhammyBarSegment()
               || isRasgueadoSegment()
               || isHarmonicMarkSegment()
               || isPedalSegment()
               || isVoltaSegment()
               || isNoteLineSegment();
    }

    bool isLineSegment() const
    {
        return isGlissandoSegment()
               || isLyricsLineSegment()
               || isTextLineBaseSegment()
               || isTrillSegment()
               || isVibratoSegment()
               || isGuitarBendSegment()
               || isGuitarBendHoldSegment()
        ;
    }

    bool isTieSegment() const
    {
        return type() == ElementType::TIE_SEGMENT || type() == ElementType::LAISSEZ_VIB_SEGMENT
               || type() == ElementType::PARTIAL_TIE_SEGMENT;
    }

    bool isTie() const
    {
        return type() == ElementType::TIE || type() == ElementType::LAISSEZ_VIB || type() == ElementType::PARTIAL_TIE;
    }

    bool isSpannerSegment() const
    {
        return isLineSegment() || isTextLineBaseSegment() || isSlurSegment() || isTieSegment();
    }

    bool isBSymbol() const { return isImage() || isSymbol(); }
    bool isTextLineBase() const
    {
        return isHairpin()
               || isLetRing()
               || isGradualTempoChange()
               || isNoteLine()
               || isOttava()
               || isPalmMute()
               || isWhammyBar()
               || isPickScrape()
               || isRasgueado()
               || isHarmonicMark()
               || isPedal()
               || isTextLine()
               || isVolta()
        ;
    }

    bool isLyricsLine() const { return type() == ElementType::LYRICSLINE || type() == ElementType::PARTIAL_LYRICSLINE; }

    bool isLyricsLineSegment() const
    {
        return type() == ElementType::LYRICSLINE_SEGMENT || type() == ElementType::PARTIAL_LYRICSLINE_SEGMENT;
    }

    bool isSLine() const
    {
        return isTextLineBase() || isTrill() || isGlissando() || isVibrato() || isGuitarBend() || isGuitarBendHold();
    }

    bool isSpanner() const
    {
        return isSlur()
               || isTie()
               || isGlissando()
               || isLyricsLine()
               || isTextLineBase()
               || isSLine()
        ;
    }

    bool isStaffTextBase() const
    {
        return isStaffText() || isSystemText() || isTripletFeel() || isPlayTechAnnotation() || isCapo() || isStringTunings();
    }

    bool isArticulationFamily() const
    {
        return isArticulation() || isOrnament();
    }
};

//---------------------------------------------------
// safe casting of ScoreElement
//
// Example for ChordRest:
//
//    ChordRest* toChordRest(EngravingItem* e)
//---------------------------------------------------

static inline ChordRest* toChordRest(EngravingObject* e)
{
    assert(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
           || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT);
    return (ChordRest*)e;
}

static inline const ChordRest* toChordRest(const EngravingObject* e)
{
    assert(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
           || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT);
    return (const ChordRest*)e;
}

static inline DurationElement* toDurationElement(EngravingObject* e)
{
    assert(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
           || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT
           || e->type() == ElementType::TUPLET);
    return (DurationElement*)e;
}

static inline const DurationElement* toDurationElement(const EngravingObject* e)
{
    assert(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
           || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT
           || e->type() == ElementType::TUPLET);
    return (const DurationElement*)e;
}

static inline Rest* toRest(EngravingObject* e)
{
    assert(!e || e->isRestFamily());
    return (Rest*)e;
}

static inline const Rest* toRest(const EngravingObject* e)
{
    assert(!e || e->isRestFamily());
    return (const Rest*)e;
}

static inline SlurTieSegment* toSlurTieSegment(EngravingObject* e)
{
    assert(
        e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT
        || e->type() == ElementType::LAISSEZ_VIB_SEGMENT || e->type() == ElementType::PARTIAL_TIE_SEGMENT);
    return (SlurTieSegment*)e;
}

static inline const SlurTieSegment* toSlurTieSegment(const EngravingObject* e)
{
    assert(
        e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT
        || e->type() == ElementType::LAISSEZ_VIB_SEGMENT || e->type() == ElementType::PARTIAL_TIE_SEGMENT);
    return (const SlurTieSegment*)e;
}

static inline const MeasureBase* toMeasureBase(const EngravingObject* e)
{
    assert(e == 0 || e->isMeasure() || e->isVBox() || e->isHBox() || e->isTBox() || e->isFBox());
    return (const MeasureBase*)e;
}

static inline MeasureBase* toMeasureBase(EngravingObject* e)
{
    assert(e == 0 || e->isMeasureBase());
    return (MeasureBase*)e;
}

static inline Box* toBox(EngravingObject* e)
{
    assert(e == 0 || e->isBox());
    return (Box*)e;
}

static inline SpannerSegment* toSpannerSegment(EngravingObject* e)
{
    assert(e == 0 || e->isSpannerSegment());
    return (SpannerSegment*)e;
}

static inline const SpannerSegment* toSpannerSegment(const EngravingObject* e)
{
    assert(e == 0 || e->isSpannerSegment());
    return (const SpannerSegment*)e;
}

static inline BSymbol* toBSymbol(EngravingObject* e)
{
    assert(e == 0 || e->isBSymbol());
    return (BSymbol*)e;
}

static inline TextLineBase* toTextLineBase(EngravingObject* e)
{
    assert(e == 0 || e->isTextLineBase());
    return (TextLineBase*)e;
}

static inline TextBase* toTextBase(EngravingObject* e)
{
    assert(e == 0 || e->isTextBase());
    return (TextBase*)e;
}

static inline const TextBase* toTextBase(const EngravingObject* e)
{
    assert(e == 0 || e->isTextBase());
    return (const TextBase*)e;
}

static inline StaffTextBase* toStaffTextBase(EngravingObject* e)
{
    assert(e == 0 || e->isStaffTextBase());
    return (StaffTextBase*)e;
}

static inline const StaffTextBase* toStaffTextBase(const EngravingObject* e)
{
    assert(e == 0 || e->isStaffTextBase());
    return (const StaffTextBase*)e;
}

static inline Bend* toBend(EngravingObject* e)
{
    assert(e == 0 || e->isBend());
    return (Bend*)e;
}

static inline const Bend* toBend(const EngravingObject* e)
{
    assert(e == 0 || e->isBend());
    return (const Bend*)e;
}

static inline Articulation* toArticulation(EngravingObject* e)
{
    assert(e == 0 || e->isArticulationFamily());
    return (Articulation*)e;
}

static inline const Articulation* toArticulation(const EngravingObject* e)
{
    assert(e == 0 || e->isArticulationFamily());
    return (const Articulation*)e;
}

static inline Tie* toTie(EngravingObject* e)
{
    assert(e == 0 || e->isTie() || e->isLaissezVib());
    return (Tie*)e;
}

static inline const Tie* toTie(const EngravingObject* e)
{
    assert(e == 0 || e->isTie() || e->isLaissezVib());
    return (const Tie*)e;
}

#define CONVERT(a)  \
    static inline a* to##a(EngravingObject * e) { assert(e == 0 || e->is##a()); return (a*)e; } \
    static inline const a* to##a(const EngravingObject * e) { assert(e == 0 || e->is##a()); return (const a*)e; }

CONVERT(EngravingItem)
CONVERT(Note)
CONVERT(Chord)
CONVERT(BarLine)
CONVERT(Ornament)
CONVERT(Fermata)
CONVERT(Marker)
CONVERT(Clef)
CONVERT(KeySig)
CONVERT(TimeSig)
CONVERT(Measure)
CONVERT(TempoText)
CONVERT(Breath)
CONVERT(HBox)
CONVERT(VBox)
CONVERT(TBox)
CONVERT(FBox)
CONVERT(Spanner)
CONVERT(Slur)
CONVERT(Glissando)
CONVERT(GlissandoSegment)
CONVERT(GuitarBend)
CONVERT(GuitarBendSegment)
CONVERT(GuitarBendHold)
CONVERT(GuitarBendHoldSegment)
CONVERT(GuitarBendText)
CONVERT(SystemDivider)
CONVERT(RehearsalMark)
CONVERT(TripletFeel)
CONVERT(Harmony)
CONVERT(Volta)
CONVERT(Jump)
CONVERT(StaffText)
CONVERT(PlayTechAnnotation)
CONVERT(Capo)
CONVERT(Ottava)
CONVERT(LayoutBreak)
CONVERT(SystemLockIndicator)
CONVERT(Segment)
CONVERT(System)
CONVERT(Lyrics)
CONVERT(Stem)
CONVERT(Beam)
CONVERT(BeamSegment)
CONVERT(Hook)
CONVERT(StemSlash)
CONVERT(LineSegment)
CONVERT(SlurSegment)
CONVERT(TieSegment)
CONVERT(LaissezVibSegment)
CONVERT(PartialTieSegment)
CONVERT(Spacer)
CONVERT(StaffLines)
CONVERT(Ambitus)
CONVERT(Bracket)
CONVERT(InstrumentChange)
CONVERT(StaffTypeChange)
CONVERT(Text)
CONVERT(MeasureNumber)
CONVERT(MMRestRange)
CONVERT(Hairpin)
CONVERT(HairpinSegment)
CONVERT(TremoloBar)
CONVERT(MeasureRepeat)
CONVERT(MMRest)
CONVERT(Tuplet)
CONVERT(NoteDot)
CONVERT(Dynamic)
CONVERT(Expression)
CONVERT(InstrumentName)
CONVERT(Accidental)
CONVERT(TextLine)
CONVERT(TextLineSegment)
CONVERT(Pedal)
CONVERT(PedalSegment)
CONVERT(OttavaSegment)
CONVERT(LedgerLine)
CONVERT(ActionIcon)
CONVERT(VoltaSegment)
CONVERT(NoteLine)
CONVERT(NoteLineSegment)
CONVERT(Trill)
CONVERT(TrillSegment)
CONVERT(LetRing)
CONVERT(LetRingSegment)
CONVERT(GradualTempoChange)
CONVERT(GradualTempoChangeSegment)
CONVERT(Vibrato)
CONVERT(VibratoSegment)
CONVERT(PalmMute)
CONVERT(PalmMuteSegment)
CONVERT(WhammyBar)
CONVERT(WhammyBarSegment)
CONVERT(Rasgueado)
CONVERT(RasgueadoSegment)
CONVERT(HarmonicMark)
CONVERT(HarmonicMarkSegment)
CONVERT(PickScrape)
CONVERT(PickScrapeSegment)
CONVERT(Symbol)
CONVERT(FSymbol)
CONVERT(Fingering)
CONVERT(NoteHead)
CONVERT(LyricsLine)
CONVERT(LyricsLineSegment)
CONVERT(FiguredBass)
CONVERT(FiguredBassItem)
CONVERT(StaffState)
CONVERT(Arpeggio)
CONVERT(Image)
CONVERT(ChordLine)
CONVERT(FretDiagram)
CONVERT(HarpPedalDiagram)
CONVERT(Page)
CONVERT(SystemText)
CONVERT(BracketItem)
CONVERT(Staff)
CONVERT(Part)
CONVERT(Lasso)
CONVERT(BagpipeEmbellishment)
CONVERT(Sticking)
CONVERT(GraceNotesGroup)
CONVERT(FretCircle)
CONVERT(DeadSlapped)
CONVERT(StringTunings)
CONVERT(SoundFlag)
CONVERT(TimeTickAnchor)
CONVERT(LaissezVib)
CONVERT(PartialTie)
CONVERT(PartialLyricsLine)
CONVERT(PartialLyricsLineSegment)
CONVERT(Parenthesis)
CONVERT(ShadowNote)
#undef CONVERT
}
