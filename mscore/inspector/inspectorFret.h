//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_FRET_H__
#define __INSPECTOR_FRET_H__

#include "inspector.h"
#include "inspectorBase.h"
#include "ui_inspector_fret.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFretDiagram
//---------------------------------------------------------

class InspectorFretDiagram : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorFretDiagram   f;
      virtual void setElement() override;
      void genericButtonToggled(QPushButton* b, bool v, FretDotType dtype);
      std::vector<QPushButton*> dotTypeButtons;

   private slots:
      virtual void valueChanged(int idx) override;
      void fretNumberChanged(int fretNumber);
      void resetFretNumber();

      void circleButtonToggled(bool v);
      void crossButtonToggled(bool v);
      void squareButtonToggled(bool v);
      void triangleButtonToggled(bool v);
      void barreButtonToggled(bool v);
      void multidotButtonToggled(bool v);
      void clearButtonClicked();

   public:
      InspectorFretDiagram(QWidget* parent);
      };


} // namespace Ms
#endif

