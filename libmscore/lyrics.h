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

namespace Ms {

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class LyricsLine;

class Lyrics final : public TextBase {
   public:
      enum class Syllabic : char { SINGLE, BEGIN, END, MIDDLE };

      // MELISMA FIRST UNDERSCORE:
      // used as_ticks value to mark a melisma for which only the first chord has been spanned so far
      // and to give the user a visible feedback that the undercore has been actually entered;
      // it should be cleared to 0 at some point, so that it will not be carried over
      // if the melisma is not extended beyond a single chord, but no suitable place to do this
      // has been identified yet.
      static constexpr int    TEMP_MELISMA_TICKS      = 1;

      // WORD_MIN_DISTANCE has never been implemented
      // static constexpr qreal  LYRICS_WORD_MIN_DISTANCE = 0.33;     // min. distance between lyrics from different words

   private:
      int _ticks;             ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
      Syllabic _syllabic;
      LyricsLine* _separator;

      qreal _oldrypos;

      bool isMelisma() const;
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

   protected:
      int _no;                ///< row index
      bool _even;

   public:
      Lyrics(Score* = 0);
      Lyrics(const Lyrics&);
      ~Lyrics();
      virtual Lyrics* clone() const override          { return new Lyrics(*this); }
      virtual ElementType type() const override       { return ElementType::LYRICS; }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;

      Segment* segment() const                        { return toSegment(parent()->parent()); }
      Measure* measure() const                        { return toMeasure(parent()->parent()->parent()); }
      ChordRest* chordRest() const                    { return toChordRest(parent()); }

      virtual void layout() override;
      void layout2(int);

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual bool readProperties(XmlReader&);
      virtual int subtype() const override            { return _no; }
      virtual QString subtypeName() const override    { return QObject::tr("Verse %1").arg(_no + 1); }
      void setNo(int n)                               { _no = n; }
      int no() const                                  { return _no; }
      bool isEven() const                             { return _no % 1; }
      void setSyllabic(Syllabic s)                    { _syllabic = s; }
      Syllabic syllabic() const                       { return _syllabic; }
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void endEdit(EditData&) override;

      int ticks() const                               { return _ticks;    }
      void setTicks(int tick)                         { _ticks = tick;    }
      int endTick() const;
      void removeFromScore();
      void setOldryPos()                             { rypos() = _oldrypos; }

      using ScoreElement::undoChangeProperty;
      using TextBase::paste;
      virtual void paste(EditData&) override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      };

//---------------------------------------------------------
//   LyricsLine
//---------------------------------------------------------

class LyricsLine final : public SLine {
   protected:
      Lyrics* _nextLyrics;

   public:
      LyricsLine(Score* s);
      LyricsLine(const LyricsLine&);

      virtual LyricsLine* clone() const override      { return new LyricsLine(*this); }
      virtual ElementType type() const override       { return ElementType::LYRICSLINE; }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;
      virtual void removeUnmanaged() override;
      virtual void styleChanged() override;

      Lyrics* lyrics() const                          { return toLyrics(parent());   }
      Lyrics* nextLyrics() const                      { return _nextLyrics;         }
      virtual bool setProperty(Pid propertyId, const QVariant& v) override;
      };

//---------------------------------------------------------
//   LyricsLineSegment
//---------------------------------------------------------

class LyricsLineSegment final : public LineSegment {
   protected:
      int   _numOfDashes;
      qreal _dashLength;

   public:
      LyricsLineSegment(Score* s);

      virtual LyricsLineSegment* clone() const override     { return new LyricsLineSegment(*this); }
      virtual ElementType type() const override             { return ElementType::LYRICSLINE_SEGMENT; }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      // helper functions
      LyricsLine* lyricsLine() const                        { return toLyricsLine(spanner()); }
      Lyrics* lyrics() const                                { return lyricsLine()->lyrics(); }
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Lyrics::Syllabic);

#endif

