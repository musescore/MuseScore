//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: transposedialog.h 4388 2011-06-18 13:17:58Z wschweer $
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __TRANSPOSEDIALOG_H__
#define __TRANSPOSEDIALOG_H__

#include "ui_transposedialog.h"
#include "libmscore/mscore.h"

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

class TransposeDialog : public QDialog, Ui::TransposeDialogBase {
      Q_OBJECT

   private slots:
      void transposeByKeyToggled(bool);
      void transposeByIntervalToggled(bool);

   public:
      TransposeDialog(QWidget* parent = 0);
      void enableTransposeKeys(bool val)  { transposeKeys->setEnabled(val);       }
      void enableTransposeByKey(bool val);
      bool getTransposeKeys() const       { return transposeKeys->isChecked();    }
      bool getTransposeChordNames() const { return transposeChordNames->isChecked(); }
      int transposeKey() const            { return keyList->currentIndex() - 7;      }
      int transposeInterval() const       { return intervalList->currentIndex();     }
      TransposeDirection direction() const;
      TransposeMode mode() const;
      void setKey(int k)                  { keyList->setCurrentIndex(k + 7); }
      bool useDoubleSharpsFlats() const   { return accidentalOptions->currentIndex() == 1; }
      };
#endif

