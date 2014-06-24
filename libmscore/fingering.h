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
      virtual Fingering* clone() const   { return new Fingering(*this); }
      virtual Element::Type type() const { return Element::Type::FINGERING; }

      Note* note() const { return (Note*)parent(); }

      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      virtual void reset();
      };


}     // namespace Ms
#endif

