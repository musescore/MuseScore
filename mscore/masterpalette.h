//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MASTERPALETTE_H__
#define __MASTERPALETTE_H__

#include "ui_masterpalette.h"

namespace Ms {

class Palette;
class TimeDialog;
class KeyEditor;

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

class MasterPalette : public QWidget, Ui::MasterPalette
      {
      Q_OBJECT

      TimeDialog* timeDialog;
      KeyEditor* keyEditor;
      QTreeWidgetItem* keyItem;
      QTreeWidgetItem* timeItem;
      QTreeWidgetItem* symbolItem;

      virtual void closeEvent(QCloseEvent*);
      Palette* createPalette(int w, int h, bool grid, double mag = 1.0);
      void addPalette(Palette* sp);

   signals:
      void closed(bool);

   private slots:
      void currentChanged(QTreeWidgetItem*, QTreeWidgetItem*);
      void clicked(QTreeWidgetItem*, int);

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate(bool firstTime = false);
      virtual void keyPressEvent(QKeyEvent *ev);

   public:
      MasterPalette(QWidget* parent = 0);
      void selectItem(const QString& s);
      QString selectedItem();
      };

} // namespace Ms
#endif

