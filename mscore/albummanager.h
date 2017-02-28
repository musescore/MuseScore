//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ALBUMMANAGER_H__
#define __ALBUMMANAGER_H__

#include "ui_albummanager.h"
#include "abstractdialog.h"

namespace Ms {

class Movements;

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager : public AbstractDialog, public Ui::AlbumManager {
      Q_OBJECT
      Movements* album;

      virtual void hideEvent(QHideEvent*);

   private slots:
      void addClicked();
      void addNewClicked();
      void upClicked();
      void downClicked();
      void removeClicked();
      void currentScoreChanged(int);
      void itemChanged(QListWidgetItem*);   // score name in list is edited
      void buttonBoxClicked(QAbstractButton*);

   protected:
      virtual void retranslate() { retranslateUi(this); }

   public:
      AlbumManager(QWidget* parent = 0);
      void setAlbum(Movements*);
      };
}

#endif

