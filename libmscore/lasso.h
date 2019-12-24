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

namespace Ms {

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
   public:
      Lasso(Score*);
      virtual Lasso* clone() const override        { return new Lasso(*this); }
      ElementType type() const final { return ElementType::LASSO; }
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override     { return true; }
      virtual void editDrag(EditData&) override;
      virtual void endDrag(EditData&)              {}

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;

      // TODO: single click behavior?
      int gripsCount() const override { return 8; }
      Grip initialEditModeGrip() const override { return Grip(7); }
      Grip defaultGrip() const override { return Grip(7); } // TODO
      std::vector<QPointF> gripsPositions(const EditData&) const override;
      };


}     // namespace Ms
#endif

