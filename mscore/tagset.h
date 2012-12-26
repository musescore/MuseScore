//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: mscore.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
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

#ifndef __TAGSET_H__
#define __TAGSET_H__

#include "ui_tagset.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   TagSetManager
//---------------------------------------------------------

class TagSetManager : public QDialog, public Ui::TagSetManager {
      Q_OBJECT

      Score* score;

   private slots:
      void createClicked();
      void deleteClicked();
      void addTagClicked();
      void deleteTagClicked();
      virtual void accept();

   public:
      TagSetManager(Score*, QWidget* parent = 0);
      };


} // namespace Ms
#endif
