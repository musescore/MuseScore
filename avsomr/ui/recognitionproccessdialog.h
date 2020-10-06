//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef AVS_RECOGNITIONPROCCESSDIALOG_H
#define AVS_RECOGNITIONPROCCESSDIALOG_H

#include <QProgressDialog>
#include <QString>
#include <QTimer>
#include <QTime>

#include "iavsomrrecognizer.h"

class QPushButton;

namespace Ms {
class TaskbarProgress;

namespace Avs {

class RecognitionProccessDialog : private QProgressDialog
      {
   public:
      RecognitionProccessDialog();

      void setType(const QString& type);
      void show();

      void onStep(const IAvsOmrRecognizer::Step& step);
      void onFinished(bool success);

   private:

      QString formatStep(const IAvsOmrRecognizer::Step& step) const;
      void update();

      QString _type;
      IAvsOmrRecognizer::Step _lastStep;

      QTimer _updater;
      QElapsedTimer _time;
      QPushButton* _closeBtn{nullptr};
      TaskbarProgress* _taskbarProgress{nullptr};
      };

} // Avs
} // Ms

#endif // AVS_RECOGNITIONPROCCESSDIALOG_H
