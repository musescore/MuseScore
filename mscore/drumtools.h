//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$ drumtools.h
//
//  Copyright (C) 2010-2016 Werner Schweer and others
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

#ifndef __DRUMTOOLS_H__
#define __DRUMTOOLS_H__

namespace Ms {

class Score;
class Drumset;
class Palette;
class Staff;

//---------------------------------------------------------
//   DrumTools
//---------------------------------------------------------

class DrumTools : public QDockWidget {
      Q_OBJECT

      Score* _score;
      Staff* staff;
      Palette* drumPalette;
      QToolButton* editButton;
      const Drumset* drumset;

   private slots:
      void drumNoteSelected(int val);
      void editDrumset();

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate();

   public:
      DrumTools(QWidget* parent = 0);
      void setDrumset(Score*, Staff*, const Drumset*);
      void updateDrumset(const Drumset* ds);
      int selectedDrumNote();
      };



} // namespace Ms
#endif

