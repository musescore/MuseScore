//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "element.h"

namespace Ms {

//-------------------------------------------------------------------
//   @@ StaffLines
///    The StaffLines class is the graphic representation of a staff,
///    it draws the horizontal staff lines.
//-------------------------------------------------------------------

class StaffLines final : public Element {
      qreal lw;
      QVector<QLineF> lines;

   public:
      StaffLines(Score*);
      virtual StaffLines* clone() const override    { return new StaffLines(*this); }
      virtual ElementType type() const override     { return ElementType::STAFF_LINES; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual QPointF pagePos() const override;    ///< position in page coordinates
      virtual QPointF canvasPos() const override;  ///< position in page coordinates

      QVector<QLineF>& getLines() { return lines; }
      Measure* measure() const { return (Measure*)parent(); }
      qreal y1() const;
      };

}     // namespace Ms

