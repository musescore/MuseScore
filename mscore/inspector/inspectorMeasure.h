//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_MEASURE_H__
#define __INSPECTOR_MEASURE_H__

#include "inspectorGroupElement.h"
#include "ui_inspector_measure.h"

namespace Ms {

class Fraction;
class Measure;
enum class Pid;

//---------------------------------------------------------
//    InspectorMeasure
//---------------------------------------------------------

class InspectorMeasure : public InspectorGroupElement {
      Q_OBJECT

      Ui::InspectorMeasure me;

      Fraction getInputTimesig() const;
      Measure* measure() const;
      void setProperty(Pid p, QVariant v);

   private slots:
      void timesigValueChanged(int val);
      void excludeToggled(bool val);
      void numberModeChanged(int val);
      void offsetChanged(int val);
      void breakMMRToggled(bool val);
      void layoutStretchChanged(qreal val);
      void playCountChanged(int val);

   public:
      InspectorMeasure(QWidget* parent);
      };

} // namespace Ms
#endif


