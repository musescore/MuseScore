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

#ifndef __SCORE_ELEMENT_H__
#define __SCORE_ELEMENT_H__

#include "types.h"
#include "style.h"

#include "draw/geometry.h"

namespace Ms {
class ScoreElement;
class MasterScore;
class XmlWriter;
class ConnectorInfoReader;
class Measure;
class Staff;
class Part;
class Score;
class Sym;
class MuseScoreView;
class Segment;
class Element;
class BarLine;
class Articulation;
class Marker;
class Clef;
class KeySig;
class TimeSig;
class TempoText;
class Breath;
class Box;
class HBox;
class VBox;
class TBox;
class FBox;
class ChordRest;
class Slur;
class Tie;
class Glissando;
class GlissandoSegment;
class SystemDivider;
class RehearsalMark;
class Harmony;
class Volta;
class Jump;
class StaffText;
class Ottava;
class Note;
class Chord;
class Rest;
class MMRest;
class LayoutBreak;
class Tremolo;
class System;
class Sticking;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;
class Stem;
class SlurSegment;
class TieSegment;
class OttavaSegment;
class Beam;
class Hook;
class StemSlash;
class Spacer;
class StaffLines;
class Ambitus;
class Bracket;
class InstrumentChange;
class Text;
class TextBase;
class Hairpin;
class HairpinSegment;
class Bend;
class TremoloBar;
class MeasureRepeat;
class Tuplet;
class NoteDot;
class Dynamic;
class InstrumentName;
class DurationElement;
class Accidental;
class TextLine;
class TextLineSegment;
class Pedal;
class PedalSegment;
class LedgerLine;
class Icon;
class VoltaSegment;
class NoteLine;
class Trill;
class TrillSegment;
class Symbol;
class FSymbol;
class Fingering;
class NoteHead;
class FiguredBass;
class StaffState;
class Arpeggio;
class Image;
class ChordLine;
class SlurTieSegment;
class FretDiagram;
class StaffTypeChange;
class MeasureBase;
class Page;
class SystemText;
class BracketItem;
class Spanner;
class SpannerSegment;
class Lasso;
class BagpipeEmbellishment;
class LineSegment;
class BSymbol;
class TextLineBase;
class Fermata;

class LetRing;
class LetRingSegment;
class Vibrato;
class VibratoSegment;
class PalmMute;
class PalmMuteSegment;
class MeasureNumber;
class MMRestRange;

class StaffTextBase;

enum class Pid : int;
enum class PropertyFlags : char;

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

class LinkedElements : public QList<ScoreElement*>
{
    int _lid;           // unique id for every linked list

public:
    LinkedElements(Score*);
    LinkedElements(Score*, int id);

    void setLid(Score*, int val);
    int lid() const { return _lid; }

    ScoreElement* mainElement();
};

//---------------------------------------------------------
//   ElementName
//---------------------------------------------------------

struct ElementName {
    ElementType type;
    const char* name;
    const char* userName;
};

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

class ScoreElement
{
    Score* _score;
    static ElementStyle const emptyStyle;

protected:
    const ElementStyle* _elementStyle {& emptyStyle };
    PropertyFlags* _propertyFlagsList { 0 };
    LinkedElements* _links            { 0 };
    virtual int getPropertyFlagsIdx(Pid id) const;

public:
    ScoreElement(Score* s)
        : _score(s) {}
    ScoreElement(const ScoreElement& se);

    virtual ~ScoreElement();

    // Score Tree functions
    virtual ScoreElement* treeParent() const { return nullptr; }
    virtual ScoreElement* treeChild(int n) const { Q_UNUSED(n); return nullptr; }
    virtual int treeChildCount() const { return 0; }

    int treeChildIdx(ScoreElement* child) const;

    // For iterating over child elements
    class iterator
    {
        ScoreElement* el;
        int i;
    public:
        iterator(ScoreElement* el, int pos)
            : el(el), i(pos) {}
        iterator operator++() { return iterator(el, i++); }
        ScoreElement* operator*() { return el->treeChild(i); }
        bool operator!=(const iterator& o) { return o.el != el || o.i != i; }
    };

    class const_iterator
    {
        const ScoreElement* el;
        int i;
    public:
        const_iterator(const ScoreElement* el, int pos)
            : el(el), i(pos) {}
        const_iterator operator++() { return const_iterator(el, i++); }
        const ScoreElement* operator*() { return el->treeChild(i); }
        bool operator!=(const const_iterator& o) { return o.el != el || o.i != i; }
    };

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, treeChildCount()); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, treeChildCount()); }

    Score* score() const { return _score; }
    MasterScore* masterScore() const;
    virtual void setScore(Score* s) { _score = s; }
    const char* name() const;
    virtual QString userName() const;
    virtual ElementType type() const = 0;

    static ElementType name2type(const QStringRef&, bool silent = false);
    static ElementType name2type(const QString& s) { return name2type(QStringRef(&s)); }
    static const char* name(ElementType);

    virtual void scanElements(void* data, void (* func)(void*, Element*), bool all=true);

    virtual QVariant getProperty(Pid) const = 0;
    virtual bool setProperty(Pid, const QVariant&) = 0;
    virtual QVariant propertyDefault(Pid) const;
    virtual void resetProperty(Pid id);
    QVariant propertyDefault(Pid pid, Tid tid) const;
    virtual bool sizeIsSpatiumDependent() const { return true; }
    virtual bool offsetIsSpatiumDependent() const { return true; }

    virtual void reset();                       // reset all properties & position to default

    virtual Pid propertyId(const QStringRef& xmlName) const;
    virtual QString propertyUserValue(Pid) const;

    virtual void initElementStyle(const ElementStyle*);
    virtual const ElementStyle* styledProperties() const { return _elementStyle; }

    virtual PropertyFlags* propertyFlagsList() const { return _propertyFlagsList; }
    virtual PropertyFlags propertyFlags(Pid) const;
    bool isStyled(Pid pid) const;
    QVariant styleValue(Pid, Sid) const;

    void setPropertyFlags(Pid, PropertyFlags);

    virtual Sid getPropertyStyle(Pid) const;
    bool readProperty(const QStringRef&, XmlReader&, Pid);
    void readProperty(XmlReader&, Pid);
    bool readStyledProperty(XmlReader& e, const QStringRef& tag);

    virtual void readAddConnector(ConnectorInfoReader* info, bool pasteMode);

    virtual void styleChanged();

    virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps);
    void undoChangeProperty(Pid id, const QVariant&);
    void undoResetProperty(Pid id);

    void undoPushProperty(Pid);
    void writeProperty(XmlWriter& xml, Pid id) const;
    void writeStyledProperties(XmlWriter&) const;

    QList<ScoreElement*> linkList() const;

    void linkTo(ScoreElement*);
    void unlink();
    bool isLinked(ScoreElement* se = nullptr) const;

    virtual void undoUnlink();
    int lid() const { return _links ? _links->lid() : 0; }
    LinkedElements* links() const { return _links; }
    void setLinks(LinkedElements* le) { _links = le; }

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
    CONVERT(Chord,         CHORD)
    CONVERT(BarLine,       BAR_LINE)
    CONVERT(Articulation,  ARTICULATION)
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
    CONVERT(Tie,           TIE)
    CONVERT(Slur,          SLUR)
    CONVERT(Glissando,     GLISSANDO)
    CONVERT(GlissandoSegment,     GLISSANDO_SEGMENT)
    CONVERT(SystemDivider, SYSTEM_DIVIDER)
    CONVERT(RehearsalMark, REHEARSAL_MARK)
    CONVERT(Harmony,       HARMONY)
    CONVERT(Volta,         VOLTA)
    CONVERT(Jump,          JUMP)
    CONVERT(Ottava,        OTTAVA)
    CONVERT(LayoutBreak,   LAYOUT_BREAK)
    CONVERT(Segment,       SEGMENT)
    CONVERT(Tremolo,       TREMOLO)
    CONVERT(System,        SYSTEM)
    CONVERT(Lyrics,        LYRICS)
    CONVERT(Stem,          STEM)
    CONVERT(Beam,          BEAM)
    CONVERT(Hook,          HOOK)
    CONVERT(StemSlash,     STEM_SLASH)
    CONVERT(SlurSegment,   SLUR_SEGMENT)
    CONVERT(TieSegment,    TIE_SEGMENT)
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
    CONVERT(InstrumentName, INSTRUMENT_NAME)
    CONVERT(Accidental,    ACCIDENTAL)
    CONVERT(TextLine,      TEXTLINE)
    CONVERT(TextLineSegment,      TEXTLINE_SEGMENT)
    CONVERT(Pedal,         PEDAL)
    CONVERT(PedalSegment,  PEDAL_SEGMENT)
    CONVERT(OttavaSegment, OTTAVA_SEGMENT)
    CONVERT(LedgerLine,    LEDGER_LINE)
    CONVERT(Icon,          ICON)
    CONVERT(VoltaSegment,  VOLTA_SEGMENT)
    CONVERT(NoteLine,      NOTELINE)
    CONVERT(Trill,         TRILL)
    CONVERT(TrillSegment,  TRILL_SEGMENT)
    CONVERT(LetRing,       LET_RING)
    CONVERT(LetRingSegment, LET_RING_SEGMENT)
    CONVERT(Vibrato,       VIBRATO)
    CONVERT(PalmMute,      PALM_MUTE)
    CONVERT(PalmMuteSegment, PALM_MUTE_SEGMENT)
    CONVERT(VibratoSegment,  VIBRATO_SEGMENT)
    CONVERT(Symbol,        SYMBOL)
    CONVERT(FSymbol,       FSYMBOL)
    CONVERT(Fingering,     FINGERING)
    CONVERT(NoteHead,      NOTEHEAD)
    CONVERT(LyricsLine,    LYRICSLINE)
    CONVERT(LyricsLineSegment, LYRICSLINE_SEGMENT)
    CONVERT(FiguredBass,   FIGURED_BASS)
    CONVERT(StaffState,    STAFF_STATE)
    CONVERT(Arpeggio,      ARPEGGIO)
    CONVERT(Image,         IMAGE)
    CONVERT(ChordLine,     CHORDLINE)
    CONVERT(FretDiagram,   FRET_DIAGRAM)
    CONVERT(Page,          PAGE)
    CONVERT(Text,          TEXT)
    CONVERT(MeasureNumber, MEASURE_NUMBER)
    CONVERT(MMRestRange,   MMREST_RANGE)
    CONVERT(StaffText,     STAFF_TEXT)
    CONVERT(SystemText,    SYSTEM_TEXT)
    CONVERT(BracketItem,   BRACKET_ITEM)
    CONVERT(Score,         SCORE)
    CONVERT(Staff,         STAFF)
    CONVERT(Part,          PART)
    CONVERT(BagpipeEmbellishment, BAGPIPE_EMBELLISHMENT)
    CONVERT(Lasso,         LASSO)
    CONVERT(Sticking,      STICKING)
#undef CONVERT

    virtual bool isElement() const { return false; }   // overridden in element.h
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
               || isTextLineSegment()
               || isOttavaSegment()
               || isPalmMuteSegment()
               || isPedalSegment()
               || isVoltaSegment()
        ;
    }

    bool isLineSegment() const
    {
        return isGlissandoSegment()
               || isLyricsLineSegment()
               || isTextLineBaseSegment()
               || isTrillSegment()
               || isVibratoSegment()
        ;
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
               || isNoteLine()
               || isOttava()
               || isPalmMute()
               || isPedal()
               || isTextLine()
               || isVolta()
        ;
    }

    bool isSLine() const
    {
        return isTextLineBase() || isTrill() || isGlissando() || isVibrato();
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
        return isStaffText() || isSystemText();
    }
};

//---------------------------------------------------
// safe casting of ScoreElement
//
// Example for ChordRest:
//
//    ChordRest* toChordRest(Element* e)
//---------------------------------------------------

static inline ChordRest* toChordRest(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
             || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT);
    return (ChordRest*)e;
}

static inline const ChordRest* toChordRest(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
             || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT);
    return (const ChordRest*)e;
}

static inline DurationElement* toDurationElement(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
             || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT
             || e->type() == ElementType::TUPLET);
    return (DurationElement*)e;
}

static inline const DurationElement* toDurationElement(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
             || e->type() == ElementType::MMREST || e->type() == ElementType::MEASURE_REPEAT
             || e->type() == ElementType::TUPLET);
    return (const DurationElement*)e;
}

static inline Rest* toRest(ScoreElement* e)
{
    Q_ASSERT(!e || e->isRestFamily());
    return (Rest*)e;
}

static inline const Rest* toRest(const ScoreElement* e)
{
    Q_ASSERT(!e || e->isRestFamily());
    return (const Rest*)e;
}

static inline SlurTieSegment* toSlurTieSegment(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT);
    return (SlurTieSegment*)e;
}

static inline const SlurTieSegment* toSlurTieSegment(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT);
    return (const SlurTieSegment*)e;
}

static inline const MeasureBase* toMeasureBase(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isMeasure() || e->isVBox() || e->isHBox() || e->isTBox() || e->isFBox());
    return (const MeasureBase*)e;
}

static inline MeasureBase* toMeasureBase(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isMeasureBase());
    return (MeasureBase*)e;
}

static inline Box* toBox(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isBox());
    return (Box*)e;
}

static inline SpannerSegment* toSpannerSegment(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isSpannerSegment());
    return (SpannerSegment*)e;
}

static inline const SpannerSegment* toSpannerSegment(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isSpannerSegment());
    return (const SpannerSegment*)e;
}

static inline BSymbol* toBSymbol(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isBSymbol());
    return (BSymbol*)e;
}

static inline TextLineBase* toTextLineBase(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isTextLineBase());
    return (TextLineBase*)e;
}

static inline TextBase* toTextBase(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isTextBase());
    return (TextBase*)e;
}

static inline const TextBase* toTextBase(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isTextBase());
    return (const TextBase*)e;
}

static inline StaffTextBase* toStaffTextBase(ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isStaffTextBase());
    return (StaffTextBase*)e;
}

static inline const StaffTextBase* toStaffTextBase(const ScoreElement* e)
{
    Q_ASSERT(e == 0 || e->isStaffTextBase());
    return (const StaffTextBase*)e;
}

#define CONVERT(a)  \
    static inline a* to##a(ScoreElement * e) { Q_ASSERT(e == 0 || e->is##a()); return (a*)e; } \
    static inline const a* to##a(const ScoreElement * e) { Q_ASSERT(e == 0 || e->is##a()); return (const a*)e; }

CONVERT(Element)
CONVERT(Note)
CONVERT(Chord)
CONVERT(BarLine)
CONVERT(Articulation)
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
CONVERT(Tie)
CONVERT(Slur)
CONVERT(Glissando)
CONVERT(GlissandoSegment)
CONVERT(SystemDivider)
CONVERT(RehearsalMark)
CONVERT(Harmony)
CONVERT(Volta)
CONVERT(Jump)
CONVERT(StaffText)
CONVERT(Ottava)
CONVERT(LayoutBreak)
CONVERT(Segment)
CONVERT(Tremolo)
CONVERT(System)
CONVERT(Lyrics)
CONVERT(Stem)
CONVERT(Beam)
CONVERT(Hook)
CONVERT(StemSlash)
CONVERT(LineSegment)
CONVERT(SlurSegment)
CONVERT(TieSegment)
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
CONVERT(Bend)
CONVERT(TremoloBar)
CONVERT(MeasureRepeat)
CONVERT(MMRest)
CONVERT(Tuplet)
CONVERT(NoteDot)
CONVERT(Dynamic)
CONVERT(InstrumentName)
CONVERT(Accidental)
CONVERT(TextLine)
CONVERT(TextLineSegment)
CONVERT(Pedal)
CONVERT(PedalSegment)
CONVERT(OttavaSegment)
CONVERT(LedgerLine)
CONVERT(Icon)
CONVERT(VoltaSegment)
CONVERT(NoteLine)
CONVERT(Trill)
CONVERT(TrillSegment)
CONVERT(LetRing)
CONVERT(LetRingSegment)
CONVERT(Vibrato)
CONVERT(VibratoSegment)
CONVERT(PalmMute)
CONVERT(PalmMuteSegment)
CONVERT(Symbol)
CONVERT(FSymbol)
CONVERT(Fingering)
CONVERT(NoteHead)
CONVERT(LyricsLine)
CONVERT(LyricsLineSegment)
CONVERT(FiguredBass)
CONVERT(StaffState)
CONVERT(Arpeggio)
CONVERT(Image)
CONVERT(ChordLine)
CONVERT(FretDiagram)
CONVERT(Page)
CONVERT(SystemText)
CONVERT(BracketItem)
CONVERT(Staff)
CONVERT(Part)
CONVERT(Lasso)
CONVERT(BagpipeEmbellishment)
CONVERT(Sticking)
#undef CONVERT
}

#endif
