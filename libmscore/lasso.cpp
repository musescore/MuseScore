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
      view = 0;
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
      painter->drawRect(_rect);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(const EditData& ed)
      {
      Qt::CursorShape cursorShape = ed.view->cursor().shape();
      switch (int(ed.curGrip)) {
            case 0:
                  cursorShape = Qt::SizeFDiagCursor;
                  _rect.setTopLeft(_rect.topLeft() + ed.delta);
                  break;
            case 1:
                  cursorShape = Qt::SizeBDiagCursor;
                  _rect.setTopRight(_rect.topRight() + ed.delta);
                  break;
            case 2:
                  cursorShape = Qt::SizeFDiagCursor;
                  _rect.setBottomRight(_rect.bottomRight() + ed.delta);
                  break;
            case 3:
                  cursorShape = Qt::SizeBDiagCursor;
                  _rect.setBottomLeft(_rect.bottomLeft() + ed.delta);
                  break;
            case 4:
                  cursorShape = Qt::SizeVerCursor;
                  _rect.setTop(_rect.top() + ed.delta.y());
                  break;
            case 5:
                  cursorShape = Qt::SizeHorCursor;
                  _rect.setRight(_rect.right() + ed.delta.x());
                  break;
            case 6:
                  cursorShape = Qt::SizeVerCursor;
                  _rect.setBottom(_rect.bottom() + ed.delta.y());
                  break;
            case 7:
                  cursorShape = Qt::SizeHorCursor;
                  _rect.setLeft(_rect.left() + ed.delta.x());
                  break;
            }
      ed.view->setCursor(QCursor(cursorShape));
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Lasso::updateGrips(Grip* defaultGrip, QVector<QRectF>& r) const
      {
      *defaultGrip = Grip(7);
      r[0].translate(_rect.topLeft());
      r[1].translate(_rect.topRight());
      r[2].translate(_rect.bottomRight());
      r[3].translate(_rect.bottomLeft());
      r[4].translate(_rect.x() + _rect.width() * .5, _rect.top());
      r[5].translate(_rect.right(), _rect.y() + _rect.height() * .5);
      r[6].translate(_rect.x() + _rect.width()*.5, _rect.bottom());
      r[7].translate(_rect.left(), _rect.y() + _rect.height() * .5);
      }

//---------------------------------------------------------
//  layout
//---------------------------------------------------------

void Lasso::layout()
      {
#if 0
      QRectF bb(_rect);
      if (view) {
            qreal dx = 1.5 / view->matrix().m11();
            qreal dy = 1.5 / view->matrix().m22();
            for (int i = 0; i < view->gripCount(); ++i)
                  bb |= view->getGrip(i).adjusted(-dx, -dy, dx, dy);
            }
#endif
      setbbox(_rect);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Lasso::startEdit(MuseScoreView* sv, const QPointF&)
      {
      view = sv;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lasso::endEdit()
      {
      view = 0;
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Lasso::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::LASSO_POS:
                  _rect.moveTo(v.toPointF());
                  break;
            case P_ID::LASSO_SIZE:
                  _rect.setSize(v.toSizeF());
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
      switch(propertyId) {
            case P_ID::LASSO_POS:
                  return _rect.topLeft();
            case P_ID::LASSO_SIZE:
                  return _rect.size();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }


}

