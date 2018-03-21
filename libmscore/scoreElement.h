//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCORE_ELEMENT_H__
#define __SCORE_ELEMENT_H__

#include "types.h"
#include "style.h"

namespace Ms {

class ScoreElement;
class MasterScore;
class XmlWriter;
class Measure;
class Staff;
class Part;
class Score;
class Sym;
class MuseScoreView;
class Segment;
class TextStyle;
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
class LayoutBreak;
class Tremolo;
class System;
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
class Hairpin;
class HairpinSegment;
class Bend;
class TremoloBar;
class RepeatMeasure;
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

enum class P_ID : int;
enum class PropertyFlags : char;
enum class StyleIdx : int;

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

class LinkedElements : public QList<ScoreElement*> {
      int _lid;         // unique id for every linked list

   public:
      LinkedElements(Score*);
      LinkedElements(Score*, int id);

      void setLid(Score*, int val);
      int lid() const   { return _lid;    }
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

class ScoreElement {
      Score* _score;

   protected:
      LinkedElements* _links            { 0 };
      PropertyFlags* _propertyFlagsList { 0 };
      SubStyleId _subStyleId            { SubStyleId::EMPTY };

   public:
      ScoreElement(Score* s) : _score(s)   {}
      ScoreElement(const ScoreElement& se);
      virtual ~ScoreElement();

      Score* score() const                 { return _score;      }
      MasterScore* masterScore() const;
      virtual void setScore(Score* s)      { _score = s;         }
      const char* name() const;
      virtual QString userName() const;
      virtual ElementType type() const = 0;

      static ElementType name2type(const QStringRef&);
      static const char* name(ElementType);

      virtual QVariant getProperty(P_ID) const = 0;
      virtual bool setProperty(P_ID, const QVariant&) = 0;
      virtual QVariant propertyDefault(P_ID) const { return QVariant(); }
      virtual void resetProperty(P_ID id);

      SubStyleId subStyleId() const                          { return _subStyleId; }
      void setSubStyleId(SubStyleId);
      void initSubStyle(SubStyleId);
      virtual const StyledProperty* styledProperties() const { return subStyle(_subStyleId).data(); }
      virtual PropertyFlags* propertyFlagsList()             { return _propertyFlagsList; }
      virtual PropertyFlags& propertyFlags(P_ID);

      virtual void setPropertyFlags(P_ID, PropertyFlags);

      virtual StyleIdx getPropertyStyle(P_ID) const;
      bool readProperty(const QStringRef&, XmlReader&, P_ID);
      bool readStyledProperty(XmlReader& e, const QStringRef& tag);

      virtual void styleChanged();

      virtual void undoChangeProperty(P_ID id, const QVariant&, PropertyFlags ps);
      void undoChangeProperty(P_ID id, const QVariant&);
      void undoResetProperty(P_ID id);


      void undoPushProperty(P_ID);
      void writeProperty(XmlWriter& xml, P_ID id) const;

      QList<ScoreElement*> linkList() const;

      void linkTo(ScoreElement*);
      void unlink();
      virtual void undoUnlink();
      int lid() const                         { return _links ? _links->lid() : 0; }
      const LinkedElements* links() const     { return _links;      }
      void setLinks(LinkedElements* le)       { _links = le;        }

      //---------------------------------------------------
      // check type
      //
      // Example for ChordRest:
      //
      //    bool             isChordRest()
      //---------------------------------------------------


#define CONVERT(a,b) \
      bool is##a() const { return type() == ElementType::b; }

      CONVERT(Note,          NOTE)
      CONVERT(Rest,          REST)
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
      CONVERT(HairpinSegment,HAIRPIN_SEGMENT)
      CONVERT(Bend,          BEND)
      CONVERT(TremoloBar,    TREMOLOBAR)
      CONVERT(RepeatMeasure, REPEAT_MEASURE)
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
      CONVERT(StaffText,     STAFF_TEXT)
      CONVERT(SystemText,    SYSTEM_TEXT)
      CONVERT(BracketItem,   BRACKET_ITEM)
      CONVERT(Staff,         STAFF)
      CONVERT(BagpipeEmbellishment, BAGPIPE_EMBELLISHMENT)
#undef CONVERT

      bool isChordRest() const       { return isRest() || isChord() || isRepeatMeasure(); }
      bool isDurationElement() const { return isChordRest() || isTuplet(); }
      bool isSlurTieSegment() const  { return isSlurSegment() || isTieSegment(); }
      bool isSLineSegment() const;
      bool isBox() const { return isVBox() || isHBox() || isTBox() || isFBox(); }
      bool isMeasureBase() const { return isMeasure() || isBox(); }
      bool isText() const;
      bool isTextLineBaseSegment() const {
         return isHairpinSegment()
         || isLetRingSegment()
         || isTextLineSegment()
         || isOttavaSegment()
         || isPalmMuteSegment()
         || isPedalSegment()
         || isVoltaSegment()
         ;
         }
      bool isLineSegment() const {
         return isGlissandoSegment()
         || isLyricsLineSegment()
         || isTextLineBaseSegment()
         || isTrillSegment()
         || isVibratoSegment()
         ;
         }
      bool isSpannerSegment() const { return isLineSegment() || isTextLineBaseSegment() || isSlurSegment() || isTieSegment(); }
      bool isBSymbol() const { return isImage() || isSymbol(); }
      bool isTextLineBase() const {
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
      bool isSLine() const {
            return isTextLineBase() || isTrill() || isGlissando() || isVibrato();
            }

      bool isSpanner() const {
         return isSlur()
         || isTie()
         || isGlissando()
         || isLyricsLine()
         || isTextLineBase()
         || isSLine()
         ;
         }
      };

//---------------------------------------------------
// safe casting of ScoreElement
//
// Example for ChordRest:
//
//    ChordRest* toChordRest(Element* e)
//---------------------------------------------------

static inline ChordRest* toChordRest(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
         || e->type() == ElementType::REPEAT_MEASURE);
      return (ChordRest*)e;
      }
static inline const ChordRest* toChordRest(const ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
         || e->type() == ElementType::REPEAT_MEASURE);
      return (const ChordRest*)e;
      }
static inline DurationElement* toDurationElement(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
         || e->type() == ElementType::REPEAT_MEASURE || e->type() == ElementType::TUPLET);
      return (DurationElement*)e;
      }
static inline const DurationElement* toDurationElement(const ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::CHORD || e->type() == ElementType::REST
         || e->type() == ElementType::REPEAT_MEASURE || e->type() == ElementType::TUPLET);
      return (const DurationElement*)e;
      }
static inline SlurTieSegment* toSlurTieSegment(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT);
      return (SlurTieSegment*)e;
      }
static inline const SlurTieSegment* toSlurTieSegment(const ScoreElement* e) {
      Q_ASSERT(e == 0 || e->type() == ElementType::SLUR_SEGMENT || e->type() == ElementType::TIE_SEGMENT);
      return (const SlurTieSegment*)e;
      }
static inline const MeasureBase* toMeasureBase(const ScoreElement* e) {
     Q_ASSERT(e == 0 || e->isMeasure() || e->isVBox() || e->isHBox() || e->isTBox() || e->isFBox());
      return (const MeasureBase*)e;
      }
static inline MeasureBase* toMeasureBase(ScoreElement* e) {
     Q_ASSERT(e == 0 || e->isMeasureBase());
      return (MeasureBase*)e;
      }
static inline Box* toBox(ScoreElement* e) {
     Q_ASSERT(e == 0 || e->isBox());
      return (Box*)e;
      }
static inline Spanner* toSpanner(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->isSpanner());
      return (Spanner*)e;
      }
static inline SpannerSegment* toSpannerSegment(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->isSpannerSegment());
      return (SpannerSegment*)e;
      }
static inline LineSegment* toLineSegment(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->isLineSegment());
      return (LineSegment*)e;
      }
static inline BSymbol* toBSymbol(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->isBSymbol());
      return (BSymbol*)e;
      }
static inline TextLineBase* toTextLineBase(ScoreElement* e) {
      Q_ASSERT(e == 0 || e->isTextLineBase());
      return (TextLineBase*)e;
      }

#define CONVERT(a)  \
static inline a* to##a(ScoreElement* e)             { Q_ASSERT(e == 0 || e->is##a()); return (a*)e; } \
static inline const a* to##a(const ScoreElement* e) { Q_ASSERT(e == 0 || e->is##a()); return (const a*)e; }

      CONVERT(Note)
      CONVERT(Rest)
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
      CONVERT(SlurSegment)
      CONVERT(TieSegment)
      CONVERT(Spacer)
      CONVERT(StaffLines)
      CONVERT(Ambitus)
      CONVERT(Bracket)
      CONVERT(InstrumentChange)
      CONVERT(StaffTypeChange)
      CONVERT(Text)
      CONVERT(Hairpin)
      CONVERT(HairpinSegment)
      CONVERT(Bend)
      CONVERT(TremoloBar)
      CONVERT(RepeatMeasure)
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
      CONVERT(BagpipeEmbellishment)
#undef CONVERT

}

#endif

