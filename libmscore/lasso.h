//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
      QRectF _rect;
      MuseScoreView* view;        // valid in edit mode

   public:
      Lasso(Score*);
      virtual Lasso* clone() const       { return new Lasso(*this); }
      virtual ElementType type() const   { return LASSO; }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const     { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      QRectF rect() const                 { return _rect; }
      void setRect(const QRectF& r)       { _rect = r;    }
      virtual void layout();
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      };

#endif

