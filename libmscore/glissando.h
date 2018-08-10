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
#include "property.h"

namespace Ms {

// the amount of white space to leave before a system-initial chord with glissando
static const qreal      GLISS_STARTOFSYSTEM_WIDTH = 4;      // in sp

class Glissando;
class Note;
enum class GlissandoType;

//---------------------------------------------------------
//   @@ GlissandoSegment
//---------------------------------------------------------

class GlissandoSegment final : public LineSegment {
   public:
      GlissandoSegment(Score* s) : LineSegment(s) {}
      Glissando* glissando() const                          { return toGlissando(spanner()); }
      virtual ElementType type() const override             { return ElementType::GLISSANDO_SEGMENT; }
      virtual GlissandoSegment* clone() const override      { return new GlissandoSegment(*this); }
      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual Element* propertyDelegate(Pid) override;
      };

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando final : public SLine {
      M_PROPERTY(QString, text, setText)
      M_PROPERTY(GlissandoType, glissandoType, setGlissandoType)
      M_PROPERTY(GlissandoStyle, glissandoStyle, setGlissandoStyle)
      M_PROPERTY(QString, fontFace, setFontFace)
      M_PROPERTY(qreal, fontSize, setFontSize)
      M_PROPERTY(bool, showText, setShowText)
      M_PROPERTY(bool, playGlissando, setPlayGlissando)
      M_PROPERTY(bool, fontBold, setFontBold)
      M_PROPERTY(bool, fontItalic, setFontItalic)
      M_PROPERTY(bool, fontUnderline, setFontUnderline)

   public:
      Glissando(Score* s);
      Glissando(const Glissando&);

      static Note* guessInitialNote(Chord* chord);
      static Note* guessFinalNote(Chord* chord);

      // overridden inherited methods
      virtual Glissando* clone() const override     { return new Glissando(*this);   }
      virtual ElementType type() const override     { return ElementType::GLISSANDO; }
      virtual LineSegment* createLineSegment() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void layout() override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;
      virtual void read300(XmlReader&) override;

      // property/style methods
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool     setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms

#endif

