//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FINGERING_H__
#define __FINGERING_H__

#include "text.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ Fingering
//---------------------------------------------------------

class Fingering : public Text {
      Q_OBJECT

   public:
      Fingering(Score* s);
      virtual Fingering* clone() const override   { return new Fingering(*this); }
      virtual Element::Type type() const override { return Element::Type::FINGERING; }

      Note* note() const { return (Note*)parent(); }

      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;
      virtual void reset() override;
      virtual int subtype() const override         { return (int) textStyleType(); }
      virtual QString subtypeName() const override { return textStyle().name(); }

      virtual QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

