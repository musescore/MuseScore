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

#include "text.h"
#include "chord.h"

class QPainter;

namespace Ms {

class Segment;
class Chord;

//---------------------------------------------------------
//   @@ Lyrics
//   @P syllabic  Ms::Lyrics::Syllabic  (SINGLE, BEGIN, END, MIDDLE)
//---------------------------------------------------------

class Lyrics : public Text {
      Q_OBJECT
      Q_PROPERTY(Ms::Lyrics::Syllabic syllabic READ syllabic WRITE setSyllabic)
      Q_ENUMS(Syllabic)

   public:
      enum class Syllabic : char { SINGLE, BEGIN, END, MIDDLE };

   private:
      int _ticks;             ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
      Syllabic _syllabic;
      QList<Line*> _separator;
      Text* _verseNumber;

   protected:
      int _no;                ///< row index

   public:
      Lyrics(Score* = 0);
      Lyrics(const Lyrics&);
      ~Lyrics();
      virtual Lyrics* clone() const override      { return new Lyrics(*this); }
      virtual Element::Type type() const override { return Element::Type::LYRICS; }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      Segment* segment() const     { return (Segment*)parent()->parent(); }
      Measure* measure() const     { return (Measure*)parent()->parent()->parent(); }
      ChordRest* chordRest() const { return (ChordRest*)parent(); }

      virtual void layout() override;
      virtual void layout1() override;

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      void setNo(int n);
      int no() const                { return _no; }
      void setSyllabic(Syllabic s)  { _syllabic = s; }
      Syllabic syllabic() const     { return _syllabic; }
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void draw(QPainter*) const override;
      virtual void endEdit() override;

      int ticks() const                { return _ticks;    }
      void setTicks(int tick)          { _ticks = tick;    }
      int endTick() const;
      bool isMelisma() const           { return _ticks > 0; }

      void clearSeparator()            { _separator.clear(); } // TODO: memory leak
      QList<Line*>* separatorList()    { return &_separator; }

      using Text::paste;
      void paste(MuseScoreView * scoreview);

      Text* verseNumber() const        { return _verseNumber; }
      void setVerseNumber(Text* t)     { _verseNumber = t;    }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Lyrics::Syllabic);

#endif

