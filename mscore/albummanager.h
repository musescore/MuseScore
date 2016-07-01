//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: albummanager.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __ALBUMMANAGER_H__
#define __ALBUMMANAGER_H__

#include "ui_albummanager.h"
#include "abstractdialog.h"
#include "album.h"

namespace Ms {

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

class AlbumManager : public AbstractDialog, public Ui::AlbumManager {
      Q_OBJECT
      Album* album;

      void setAlbum(Album*);
      virtual void closeEvent(QCloseEvent*);

   private slots:
      void addClicked();
      void loadClicked();
      void printClicked();
      void createScoreClicked();
      void upClicked();
      void downClicked();
      void removeClicked();
      void createNewClicked();
      void albumNameChanged(const QString&);
      void currentScoreChanged(int);
      void itemChanged(QListWidgetItem*);   // score name in list is edited
      void buttonBoxClicked(QAbstractButton*);

   protected:
      virtual void retranslate() { retranslateUi(this); }

   private:
      void writeAlbum();

   public:
      AlbumManager(QWidget* parent = 0);
      };
}

#endif

