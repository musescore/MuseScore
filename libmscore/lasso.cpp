//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "lasso.h"
#include "mscore.h"
#include "mscoreview.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
   : Element(s)
      {
      setVisible(false);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lasso::draw(QPainter* painter) const
      {
      painter->setBrush(QBrush(QColor(0, 0, 50, 50)));
      // always 2 pixel width
      qreal w = 2.0 / painter->transform().m11();
      painter->setPen(QPen(MScore::selectColor[0], w));
      painter->drawRect(bbox());
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(EditData& ed)
      {
      Qt::CursorShape cursorShape = ed.view->cursor().shape();
      switch (int(ed.curGrip)) {
            case 0:
                  cursorShape = Qt::SizeFDiagCursor;
                  bbox().setTopLeft(bbox().topLeft() + ed.delta);
                  break;
            case 1:
                  cursorShape = Qt::SizeBDiagCursor;
                  bbox().setTopRight(bbox().topRight() + ed.delta);
                  break;
            case 2:
                  cursorShape = Qt::SizeFDiagCursor;
                  bbox().setBottomRight(bbox().bottomRight() + ed.delta);
                  break;
            case 3:
                  cursorShape = Qt::SizeBDiagCursor;
                  bbox().setBottomLeft(bbox().bottomLeft() + ed.delta);
                  break;
            case 4:
                  cursorShape = Qt::SizeVerCursor;
                  bbox().setTop(bbox().top() + ed.delta.y());
                  break;
            case 5:
                  cursorShape = Qt::SizeHorCursor;
                  bbox().setRight(bbox().right() + ed.delta.x());
                  break;
            case 6:
                  cursorShape = Qt::SizeVerCursor;
                  bbox().setBottom(bbox().bottom() + ed.delta.y());
                  break;
            case 7:
                  cursorShape = Qt::SizeHorCursor;
                  bbox().setLeft(bbox().left() + ed.delta.x());
                  break;
            }
      ed.view->setCursor(QCursor(cursorShape));
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Lasso::updateGrips(EditData& ed) const
      {
      ed.grip[0].translate(bbox().topLeft());
      ed.grip[1].translate(bbox().topRight());
      ed.grip[2].translate(bbox().bottomRight());
      ed.grip[3].translate(bbox().bottomLeft());
      ed.grip[4].translate(bbox().x() + bbox().width() * .5, bbox().top());
      ed.grip[5].translate(bbox().right(), bbox().y() + bbox().height() * .5);
      ed.grip[6].translate(bbox().x() + bbox().width()*.5, bbox().bottom());
      ed.grip[7].translate(bbox().left(), bbox().y() + bbox().height() * .5);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Lasso::startEdit(EditData& ed)
      {
      Element::startEdit(ed);
      ed.grips   = 8;
      ed.curGrip = Grip(7);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lasso::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::LASSO_POS:
                  bbox().moveTo(v.toPointF());
                  break;
            case P_ID::LASSO_SIZE:
                  bbox().setSize(v.toSizeF());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setUpdateAll();
      return true;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Lasso::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LASSO_POS:
                  return bbox().topLeft();
            case P_ID::LASSO_SIZE:
                  return bbox().size();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }


}

