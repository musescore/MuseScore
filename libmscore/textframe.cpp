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

#include "box.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "repeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "mscore.h"
#include "textframe.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

TBox::TBox(Score* score)
   : VBox(score)
      {
      setBoxHeight(Spatium(1));
      _text  = new Text(score);
      _text->setLayoutToParentWidth(true);
      _text->setParent(this);
      _text->setTextStyleType(TextStyleType::FRAME);
      }

TBox::~TBox()
      {
      delete _text;
      }

//---------------------------------------------------------
//   layout
///   The text box layout() adjusts the frame height to text
///   height.
//---------------------------------------------------------

void TBox::layout()
      {
      setPos(QPointF());      // !?
      bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
      _text->layout();
      _text->setPos(leftMargin() * MScore::DPMM, topMargin() * MScore::DPMM);
      qreal h = _text->isEmpty() ? _text->lineSpacing() : _text->height();
      bbox().setRect(0.0, 0.0, system()->width(), h);
      MeasureBase::layout();  // layout LayoutBreak's
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TBox::write(Xml& xml) const
      {
      xml.stag(name());
      Box::writeProperties(xml);
      _text->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TBox::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Text")
                  _text->read(e);
            else if (Box::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TBox::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      _text->scanElements(data, func, all);
      Box::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TBox::remove(Element* el)
     {
     if (el == _text) {
           _text->clear();
           }
     else
           MeasureBase::remove(el);
     }
}

