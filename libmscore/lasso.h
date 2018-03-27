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
      virtual ElementType type() const override    { return ElementType::LASSO; }
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override     { return true; }
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual void endDrag(EditData&)              {}

      virtual void startEdit(EditData&) override;
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      };


}     // namespace Ms
#endif

