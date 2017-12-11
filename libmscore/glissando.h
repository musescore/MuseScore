//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "element.h"
#include "line.h"

namespace Ms {

// the amount of white space to leave before a system-initial chord with glissando
static const qreal      GLISS_STARTOFSYSTEM_WIDTH = 4;      // in sp

class Glissando;
class Note;
enum class GlissandoType;

//---------------------------------------------------------
//   @@ GlissandoSegment
//---------------------------------------------------------

class GlissandoSegment : public LineSegment {
   public:
      GlissandoSegment(Score* s) : LineSegment(s)           {}
      Glissando* glissando() const                          { return (Glissando*)spanner(); }
      virtual ElementType type() const override           { return ElementType::GLISSANDO_SEGMENT; }
      virtual GlissandoSegment* clone() const override      { return new GlissandoSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando : public SLine {
      GlissandoType _glissandoType;
      QString _text;
      bool _showText;
      GlissandoStyle _glissandoStyle;
      bool _playGlissando;

   protected:

   public:
      Glissando(Score* s);
      Glissando(const Glissando&);

      static Note* guessInitialNote(Chord* chord);
      static Note* guessFinalNote(Chord* chord);

      // overriden inherited methods
      virtual Glissando* clone() const override     { return new Glissando(*this);   }
      virtual ElementType type() const override     { return ElementType::GLISSANDO; }
      virtual LineSegment* createLineSegment() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void layout() override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      // Glissando specific methods
      GlissandoType glissandoType() const      { return _glissandoType;  }
      void setGlissandoType(GlissandoType v)   { _glissandoType = v;     }
      GlissandoStyle glissandoStyle() const    { return _glissandoStyle; }
      void setGlissandoStyle(GlissandoStyle s) { _glissandoStyle = s;    }
      bool playGlissando() const               { return _playGlissando;  }
      void setPlayGlissando(bool v)            { _playGlissando = v;     }
      QString text() const                     { return _text;           }
      void setText(const QString& t)           { _text = t;              }
      bool showText() const                    { return _showText;       }
      void setShowText(bool v)                 { _showText = v;          }

      // property methods
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms

#endif

