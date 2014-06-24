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

namespace Ms {

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

TBox::TBox(Score* score)
   : VBox(score)
      {
      setBoxHeight(Spatium(0));
      Text* s = new Text(score);
      s->setTextStyle(score->textStyle(TextStyleType::FRAME));
      add(s);
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
      foreach(Element* e, _el) {
            if (e->isText()) {
                  Text* text = static_cast<Text*>(e);
                  text->layout();
                  qreal h;
                  if (text->isEmpty())
                        h = text->lineSpacing();
                  else
                        h = text->height();
                  text->setPos(leftMargin() * MScore::DPMM, topMargin() * MScore::DPMM);
                  bbox().setRect(0.0, 0.0, system()->width(), h);
                  }
            }
      MeasureBase::layout();  // layout LayoutBreak's
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to text box
//---------------------------------------------------------

void TBox::add(Element* e)
      {
      if (e->type() == Element::Type::TEXT) {
            Text* text = static_cast<Text*>(e);
            text->setLayoutToParentWidth(true);
            text->setFlag(ElementFlag::MOVABLE, false);
            }
      Box::add(e);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* TBox::getText()
      {
      if (_el.empty())
            return 0;
      if ((*_el.begin())->type() == Element::Type::TEXT)
            return static_cast<Text*>(*_el.begin());
      return 0;
      }

}

