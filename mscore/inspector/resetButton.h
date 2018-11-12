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


//---------------------------------------------------------
//   ResetButton
//---------------------------------------------------------

class ResetButton : public QWidget {
      Q_OBJECT

   private slots:
      void _sizeChanged();

   signals:
      void valueChanged(const QVariant&);

   public:
      ResetButton(QWdiget* parent = 0);
      };


#endif



