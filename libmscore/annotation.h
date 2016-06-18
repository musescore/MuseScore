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

#ifndef ANNOTATION_H
#define ANNOTATION_H


/**
 \file
 Definition of class Annotation
*/

#include "element.h"
namespace Ms {

enum class AnnotationType : char {
      TEXT, SHAPE, HIGHLIGHT
      };
enum class AnchorType : char {
      SEGMENT, ELEMENT, REGION
      };


//---------------------------------------------------------
//   @@ Annotation
//---------------------------------------------------------

class Annotation : public Element {
      Q_OBJECT
      Text* _text;
      AnchorType _anchorType;
      AnnotationType _annotationType;


   public:

      Annotation(Score* s);
      virtual Annotation* clone() const override         { return new Annotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::ANNOTATION; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

   //   void showAnnotation();
   //   void hideAnnotation();
   //   void setFgColor();
   //   void setBgColor();
   //   void addShape();
      void setTextAnnotation(Text text);
      Text* textAnnotation()                          { return _text ; }
      AnnotationType annotationType();
      AnchorType anchorType();

      void      draw(QPainter*) const override;
      void      layout() override;


   //   virtual Element* nextElement() override;
   //   virtual Element* prevElement() override;
      };


}     // namespace Ms



#endif // ANNOTATION_H
