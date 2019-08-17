//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __MEASUREPROPERTIES_H__
#define __MEASUREPROPERTIES_H__

#include "ui_measureproperties.h"
#include "libmscore/sig.h"

namespace Ms {

class Measure;

//---------------------------------------------------------
//   MeasureProperties
//---------------------------------------------------------

class MeasureProperties : public QDialog, private Ui::MeasurePropertiesBase {
      Q_OBJECT
      Measure* m;

      void apply();
      Fraction len() const;
      bool isIrregular() const;
      int repeatCount() const;
      bool visible(int staffIdx);
      bool slashStyle(int staffIdx);
      void setMeasure(Measure* _m);

      virtual void hideEvent(QHideEvent*);

   private slots:
      void bboxClicked(QAbstractButton* button);
      void gotoNextMeasure();
      void gotoPreviousMeasure();

   public:
      MeasureProperties(Measure*, QWidget* parent = 0);
      };


} // namespace Ms
#endif

