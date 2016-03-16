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
      virtual Lasso* clone() const override        { return new Lasso(*this); }
      virtual Element::Type type() const override  { return Element::Type::LASSO; }
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override     { return true; }
      virtual void editDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 8; }

      QRectF rect() const                 { return _rect; }
      void setRect(const QRectF& r)       { _rect = r;    }
      void setSize(qreal w, qreal h)      { _rect.setWidth(w), _rect.setHeight(h); }

      virtual void layout() override;
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      };


}     // namespace Ms
#endif

