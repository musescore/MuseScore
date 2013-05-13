//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_NOTE_H__
#define __INSPECTOR_NOTE__H__

#include "inspectorBase.h"
#include "ui_inspector_element.h"
#include "ui_inspector_note.h"
#include "ui_inspector_chord.h"
#include "ui_inspector_segment.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

class InspectorNote : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement b;
      Ui::InspectorNote    n;
      Ui::InspectorChord   c;
      Ui::InspectorSegment s;

      QToolButton* dot1;
      QToolButton* dot2;
      QToolButton* dot3;
      QToolButton* hook;
      QToolButton* stem;
      QToolButton* beam;

      void block(bool);

   private slots:
      void dot1Clicked();
      void dot2Clicked();
      void dot3Clicked();
      void hookClicked();
      void stemClicked();
      void beamClicked();

   public:
      InspectorNote(QWidget* parent);
      virtual void setElement();
      };


} // namespace Ms
#endif

