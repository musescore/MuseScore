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

#ifndef __INSPECTOR_GROUP_ELEMENT_H__
#define __INSPECTOR_GROUP_ELEMENT_H__

#include "inspectorBase.h"
#include "ui_inspector_group_element.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorGroupElement
//---------------------------------------------------------

class InspectorGroupElement : public InspectorBase {
      Q_OBJECT

      Ui::InspectorGroupElement ge;
      QToolButton* notes;
      QToolButton* graceNotes;
      QToolButton* rests;

   private slots:
      void setColor();
      void setVisible();
      void setInvisible();
      void notesClicked();
      void graceNotesClicked();
      void restsClicked();

   public:
      InspectorGroupElement(QWidget* parent);
      };


} // namespace Ms
#endif


