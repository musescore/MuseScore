//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __MEDIADIALOG_H__
#define __MEDIADIALOG_H__

#include "ui_mediadialog.h"

namespace Ms {

class Score;
class MasterScore;

//---------------------------------------------------------
//   MediaDialog
//---------------------------------------------------------

class MediaDialog : public QDialog, Ui::MediaDialog {
      Q_OBJECT
      MasterScore* score;

   private slots:
      void addScanPressed();
      void removeScanPressed();
      void addAudioPressed();
      void removeAudioPressed();
      void scanFileButtonPressed();
      void audioFileButtonPressed();

   public:
      MediaDialog(QWidget* parent = 0);
      void setScore(Score*);
      };


} // namespace Ms
#endif
