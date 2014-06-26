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

#ifndef __LASSO_H__
#define __LASSO_H__

#include "element.h"

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
      Q_OBJECT

      QRectF _rect;
      MuseScoreView* view;        // valid in edit mode

   public:
      Lasso(Score*);
      virtual Lasso* clone() const        { return new Lasso(*this); }
      virtual Element::Type type() const  { return Element::Type::LASSO; }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const     { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, int*, QRectF*) const override;
      QRectF rect() const                 { return _rect; }
      void setRect(const QRectF& r)       { _rect = r;    }
      void setSize(qreal w, qreal h)      { _rect.setWidth(w), _rect.setHeight(h); }
      virtual void layout();
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      };


}     // namespace Ms
#endif

