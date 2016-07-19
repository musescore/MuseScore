//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef TEXTANNOTATION_H
#define TEXTANNOTATION_H


/**
 \file
 Definition of class textAnnotation
*/

#include "text.h"
namespace Ms {

enum class textAnchorType : char {
      SEGMENT, ELEMENT, REGION
      };


//---------------------------------------------------------
//   @@ Annotation
//---------------------------------------------------------

class TextAnnotation : public Text {
      Q_OBJECT
 //   Text* _text;
      textAnchorType _anchorType;

   public:

      TextAnnotation(Score*  = 0);
      virtual TextAnnotation* clone() const override         { return new TextAnnotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

   //   void showAnnotation();
   //   void hideAnnotation();
      textAnchorType anchorType();

   // virtual Element* nextElement() override;
   // virtual Element* prevElement() override;
      };


}     // namespace Ms



#endif // TEXTANNOTATION_H
