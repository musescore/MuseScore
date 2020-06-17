//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __ALIGN_SELECT_H__
#define __ALIGN_SELECT_H__

#include "ui_align_select.h"

namespace Ms {

enum class Align : char;

//---------------------------------------------------------
//   AlignSelect
//---------------------------------------------------------

class AlignSelect : public QWidget, public Ui::AlignSelect {
      Q_OBJECT

      QButtonGroup* g1;
      QButtonGroup* g2;

      void blockAlign(bool val);

   private slots:
      void _alignChanged();

   signals:
      void alignChanged(Align);

   public:
      AlignSelect(QWidget* parent);
      Align align() const;
      void setAlign(Align);
      };

}


#endif

