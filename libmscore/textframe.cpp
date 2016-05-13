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

TBox::TBox(const TBox& tbox)
   : VBox(tbox)
      {
      _text = new Text(*(tbox._text));
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
      _text->setPos(leftMargin() * DPMM, topMargin() * DPMM);
      qreal h = _text->empty() ? _text->lineSpacing() : _text->height();
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
//   drop
//---------------------------------------------------------

Element* TBox::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case Element::Type::TEXT:
                  {
                  Text* t = static_cast<Text*>(e);
                  _text->undoSetText(t->xmlText());
                  _text->undoChangeProperty(P_ID::TEXT_STYLE, QVariant::fromValue(t->textStyle()));
                  delete e;
                  return _text;
                  }
            default:
                  return VBox::drop(data);
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to TBox
//---------------------------------------------------------

void TBox::add(Element* e)
      {
      if (e->type() == Element::Type::TEXT) {
            // does not normally happen, since drop() handles this directly
            Text* t = static_cast<Text*>(e);
            _text->undoSetText(t->xmlText());
            _text->undoChangeProperty(P_ID::TEXT_STYLE, QVariant::fromValue(t->textStyle()));
            }
      else {
            VBox::add(e);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TBox::remove(Element* el)
      {
      if (el == _text) {
            // does not normally happen, since Score::deleteItem() handles this directly
            // but if it does:
            // replace with new empty text element
            // this keeps undo/redo happier than just clearing the text
            qDebug("TBox::remove() - replacing _text");
            _text = new Text(score());
            _text->setLayoutToParentWidth(true);
            _text->setParent(this);
            _text->setTextStyleType(TextStyleType::FRAME);
           }
      else {
            VBox::remove(el);
           }
      }

}

