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
      GlissandoSegment(Spanner* sp, Score* s) : LineSegment(sp, s) {}
      Glissando* glissando() const                          { return toGlissando(spanner()); }

      ElementType type() const override             { return ElementType::GLISSANDO_SEGMENT; }
      GlissandoSegment* clone() const override      { return new GlissandoSegment(*this); }
      void draw(QPainter*) const override;
      void layout() override;

      Element* propertyDelegate(Pid) override;
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
      M_PROPERTY(FontStyle, fontStyle, setFontStyle)
      M_PROPERTY(int, easeIn, setEaseIn)
      M_PROPERTY(int, easeOut, setEaseOut)

      static const std::array<const char *, 2> glissandoTypeNames;

   public:
      Glissando(Score* s);
      Glissando(const Glissando&);

      static Note* guessInitialNote(Chord* chord);
      static Note* guessFinalNote(Chord* chord);

      QString glissandoTypeName() { return qApp->translate("Palette", glissandoTypeNames[int(glissandoType())]); }

      // overridden inherited methods
      Glissando* clone() const override     { return new Glissando(*this);   }
      ElementType type() const override     { return ElementType::GLISSANDO; }
      LineSegment* createLineSegment() override;
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      void layout() override;
      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      // property/style methods
      QVariant getProperty(Pid propertyId) const override;
      bool     setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;
      Pid propertyId(const QStringRef& xmlName) const override;
      };


}     // namespace Ms

#endif

