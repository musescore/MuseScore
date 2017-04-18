//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: pagesettings.h 4998 2011-11-17 11:04:44Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __PAGESETTINGS_H__
#define __PAGESETTINGS_H__

#include "ui_pagesettings.h"

namespace Ms {

class Score;
class Navigator;

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

class PageSettings : public QDialog, private Ui::PageSettingsBase {
      Q_OBJECT

      Navigator* preview;
      bool mmUnit;
      Score* cs;
      void updateValues();
      void updatePreview(int);
      void blockSignals(bool);
      void applyToScore(Score*);

   private slots:
      void mmClicked();
      void inchClicked();
      void pageFormatSelected(int);

      void apply();
      void applyToAllParts();
      void ok();
      void done(int val);

      void twosidedToggled(bool);
      void landscapeToggled(bool);
      void otmChanged(double val);
      void obmChanged(double val);
      void olmChanged(double val);
      void ormChanged(double val);
      void etmChanged(double val);
      void ebmChanged(double val);
      void elmChanged(double val);
      void ermChanged(double val);
      void spatiumChanged(double val);
      void pageHeightChanged(double);
      void pageWidthChanged(double);
      void pageOffsetChanged(int val);

   public:
      PageSettings(QWidget* parent = 0);
      ~PageSettings();
      void setScore(Score*);
      };


} // namespace Ms
#endif

