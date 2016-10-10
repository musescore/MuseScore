//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __LYRICS_H__
#define __LYRICS_H__

#include "line.h"
#include "text.h"

// uncomment the following line to use the actual metrics of
// the font used by each lyrics rather than conventional values
//
// NOTE: CURRENTLY DOES NOT WORK (Font::tightBoundingBox() returns unusable values for glyphs not on base line)
//
//#define USE_FONT_DASH_METRIC

#if defined(USE_FONT_DASH_METRIC)
// the following line is used to turn the single font dash thickness value on or off
// when the other font dash parameters are on;
// the rationale is that the dash thickness is the most unreliable of the dash parameters
// retrievable from font metrics and it may make sense to use the other values but ignore this one.
//   #define USE_FONT_DASH_TICKNESS
#endif

namespace Ms {

//---------------------------------------------------------
//   @@ Lyrics
//   @P syllabic  enum (Lyrics.SINGLE, Lyrics.BEGIN, Lyrics.END, Lyrics.MIDDLE)
//---------------------------------------------------------

class LyricsLine;

class Lyrics : public Text {
      Q_OBJECT
      Q_PROPERTY(Ms::Lyrics::Syllabic syllabic READ syllabic WRITE setSyllabic)
      Q_ENUMS(Syllabic)

   public:
      enum class Syllabic : char { SINGLE, BEGIN, END, MIDDLE };
      // MELISMA FIRST UNDERSCORE:
      // used as_ticks value to mark a melisma for which only the first chord has been spanned so far
      // and to give the user a visible feedback that the undercore has been actually entered;
      // it should be cleared to 0 at some point, so that it will not be carried over
      // if the melisma is not extended beyond a single chord, but no suitable place to do this
      // has been identified yet.
      static const int  TEMP_MELISMA_TICKS      = 1;

      // metrics for dashes and melisma; all in sp. units:
      static constexpr qreal  MELISMA_DEFAULT_PAD                 = 0.10;     // the empty space before a melisma line
      static constexpr qreal  LYRICS_DASH_DEFAULT_PAD             = 0.05;     // the min. empty space before and after a dash
// WORD_MIN_DISTANCE has never been implemented
//      static constexpr qreal  LYRICS_WORD_MIN_DISTANCE            = 0.33;     // min. distance between lyrics from different words
      // These values are used when USE_FONT_DASH_METRIC is not defined
#if !defined(USE_FONT_DASH_METRIC)
      static constexpr qreal  LYRICS_DASH_DEFAULT_LINE_THICKNESS  = 0.15;     // in sp. units
      static constexpr qreal  LYRICS_DASH_Y_POS_RATIO             = 0.67;     // the fraction of lyrics font x-height to
                                                                              // raise the dashes above text base line;
#endif

   private:
      int _ticks;             ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
      Syllabic _syllabic;
      LyricsLine* _separator;
      PropertyStyle placementStyle;

   protected:
      int _no;                ///< row index
#if defined(USE_FONT_DASH_METRIC)
      qreal _dashY;           // dash dimensions for lyrics line dashes
      qreal _dashLength;
   #if defined (USE_FONT_DASH_TICKNESS)
      qreal _dashThickness;
   #endif
#endif

   public:
      Lyrics(Score* = 0);
      Lyrics(const Lyrics&);
      ~Lyrics();
      virtual Lyrics* clone() const override          { return new Lyrics(*this); }
      virtual Element::Type type() const override     { return Element::Type::LYRICS; }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      Segment* segment() const                        { return (Segment*)parent()->parent(); }
      Measure* measure() const                        { return (Measure*)parent()->parent()->parent(); }
      ChordRest* chordRest() const                    { return (ChordRest*)parent(); }

      virtual void layout() override;
      virtual void layout1() override;

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual int subtype() const override            { return _no; }
      virtual QString subtypeName() const override    { return tr("Verse %1").arg(_no + 1); }
      void setNo(int n);
      int no() const                                  { return _no; }
      void setSyllabic(Syllabic s)                    { _syllabic = s; }
      Syllabic syllabic() const                       { return _syllabic; }
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void endEdit() override;

      int ticks() const                               { return _ticks;    }
      void setTicks(int tick)                         { _ticks = tick;    }
      int endTick() const;
      bool isMelisma() const;
      void removeFromScore();

#if defined(USE_FONT_DASH_METRIC)
      qreal dashLength() const                        { return _dashLength;         }
      qreal dashY() const                             { return _dashY;              }
   #if defined (USE_FONT_DASH_TICKNESS)
      qreal dashThickness() const                     { return _dashThickness;      }
   #endif
#endif

      using Text::paste;
      void paste(MuseScoreView * scoreview);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;
      virtual void reset() override;
      virtual void styleChanged() override;
      virtual void resetProperty(P_ID id) override;
      };

//---------------------------------------------------------
//   LyricsLine
//---------------------------------------------------------

class LyricsLine : public SLine {
      Q_OBJECT

   protected:
      Lyrics* _nextLyrics;

   public:
      LyricsLine(Score* s);
      LyricsLine(const LyricsLine&);

      virtual LyricsLine* clone() const override      { return new LyricsLine(*this); }
      virtual Element::Type type() const override     { return Element::Type::LYRICSLINE; }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void removeUnmanaged() override;

      Lyrics* lyrics() const                          { return (Lyrics*)parent();   }
      Lyrics* nextLyrics() const                      { return _nextLyrics;         }
      virtual bool setProperty(P_ID propertyId, const QVariant& v) override;
      };

//---------------------------------------------------------
//   LyricsLineSegment
//---------------------------------------------------------

class LyricsLineSegment : public LineSegment {
      Q_OBJECT

   protected:
      int   _numOfDashes;
      qreal _dashLength;

public:
      LyricsLineSegment(Score* s);

      virtual LyricsLineSegment* clone() const override     { return new LyricsLineSegment(*this); }
      virtual Element::Type type() const override           { return Element::Type::LYRICSLINE_SEGMENT; }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      LyricsLine* lyricsLine() const                        { return (LyricsLine*)spanner(); }
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Lyrics::Syllabic);

#endif

