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
#include "property.h"

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
      virtual ElementType type() const override { return ElementType::FINGERING; }

      Note* note() const { return toNote(parent()); }

      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;
      virtual int subtype() const override         { return (int) subStyle(); }
      virtual QString subtypeName() const override;

      virtual QString accessibleInfo() const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant& v) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyFlags propertyFlags(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;
      virtual void styleChanged() override;
      virtual void reset() override;
      };


}     // namespace Ms
#endif

