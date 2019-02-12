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
      _text  = new Text(score, Tid::FRAME);
      _text->setLayoutToParentWidth(true);
      _text->setParent(this);
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
      bbox().setRect(0.0, 0.0, system()->width(), 0);
      _text->layout();

      qreal h = _text->height();
      if (_text->empty()) {
            QFontMetricsF fm = QFontMetricsF(_text->font(), MScore::paintDevice());
            h = fm.ascent();
            }
      else
            h = _text->height();
      qreal y = topMargin() * DPMM;
#if 0
      if (_text->align() & Align::BOTTOM)
            y += h;
      else if (_text->align() & Align::VCENTER)
            y +=  h * .5;
      else
            ; // y = 0;
#endif
      _text->setPos(leftMargin() * DPMM, y);
      h += topMargin() * DPMM + bottomMargin() * DPMM;
      bbox().setRect(0.0, 0.0, system()->width(), h);

      MeasureBase::layout();  // layout LayoutBreak's
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TBox::write(XmlWriter& xml) const
      {
      xml.stag(this);
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

Element* TBox::drop(EditData& data)
      {
      Element* e = data.dropElement;
      switch (e->type()) {
            case ElementType::TEXT:
                  _text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
                  delete e;
                  return _text;
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
      if (e->isText()) {
            // does not normally happen, since drop() handles this directly
            _text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
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
            _text = new Text(score(), Tid::FRAME);
            _text->setLayoutToParentWidth(true);
            _text->setParent(this);
           }
      else {
            VBox::remove(el);
           }
      }

}

