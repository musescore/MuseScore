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

//---------------------------------------------------------
//   @@ GlissandoSegment
//---------------------------------------------------------

class GlissandoSegment : public LineSegment {
      Q_OBJECT

   protected:

   public:
      GlissandoSegment(Score* s) : LineSegment(s)           {}
      Glissando* glissando() const                          { return (Glissando*)spanner(); }
      virtual Element::Type type() const override           { return Element::Type::GLISSANDO_SEGMENT; }
      virtual GlissandoSegment* clone() const override      { return new GlissandoSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };

//---------------------------------------------------------
//   @@ Glissando
//   @P glissandoType  enum (Glissando.STRAIGHT, Glissando.WAVY)
//   @P showText       bool
//   @P text           string
//---------------------------------------------------------

class Glissando : public SLine {
      Q_OBJECT

      Q_PROPERTY(Ms::Glissando::Type glissandoType READ glissandoType  WRITE undoSetGlissandoType)
      Q_PROPERTY(QString text                      READ text     WRITE undoSetText)
      Q_PROPERTY(bool showText                     READ showText WRITE undoSetShowText)
      Q_ENUMS(Type)

  public:
      enum class Type : char {
            STRAIGHT, WAVY
            };

   private:
      Type _glissandoType;
      QString _text;
      bool _showText;
      MScore::GlissandoStyle _glissandoStyle;
      bool _playGlissando;

   protected:

   public:
      Glissando(Score* s);
      Glissando(const Glissando&);

      static Note* guessInitialNote(Chord* chord);
      static Note* guessFinalNote(Chord* chord);

      // overriden inherited methods
      virtual Glissando* clone() const override       { return new Glissando(*this); }
      virtual Element::Type type() const override     { return Element::Type::GLISSANDO; }
      virtual LineSegment* createLineSegment() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      // Glissando specific methods
      Type glissandoType() const          { return _glissandoType;}
      void setGlissandoType(Type v)       { _glissandoType = v;   }
      MScore::GlissandoStyle glissandoStyle() const { return _glissandoStyle;}
      void setGlissandoStyle(MScore::GlissandoStyle s) { _glissandoStyle = s; }
      bool playGlissando() const          { return _playGlissando;}
      void setPlayGlissando(bool v)       { _playGlissando = v; }
      QString text() const                { return _text;         }
      void setText(const QString& t)      { _text = t;            }
      bool showText() const               { return _showText;     }
      void setShowText(bool v)            { _showText = v;        }

      void undoSetGlissandoType(Type);
      void undoSetText(const QString&);
      void undoSetShowText(bool);

      // property methods
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Glissando::Type);

#endif

