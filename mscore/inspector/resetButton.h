//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __RESET_BUTTON_H__
#define __RESET_BUTTON_H__

namespace Ms {

//---------------------------------------------------------
//   ResetButton
//---------------------------------------------------------

class ResetButton : public QWidget {
      Q_OBJECT
      QToolButton* reset;
      QToolButton* setStyle;

   private slots:

   signals:
      void resetClicked();
      void setStyleClicked();

   public:
      ResetButton(QWidget* parent = 0);
      void enableSetStyle(bool);
      };

} // namespace

#endif



