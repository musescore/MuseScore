//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textstyle.h 4388 2011-06-18 13:17:58Z wschweer $
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

#ifndef __TEXTSTYLEDIALOG_H__
#define __TEXTSTYLEDIALOG_H__

#include "libmscore/style.h"
#include "ui_textstyledialog.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

class TextStyleDialog : public QDialog, Ui::TextStyleDialog {
      Q_OBJECT

      QPushButton* buttonApplyToAllParts;
      QList<TextStyle> styles;   // local copy of text style
      Score* cs;
      int current;

      void saveStyle(int);
      void apply();
      void applyToAllParts();
      void applyToScore(Score*);

   private slots:
      void nameSelected(int);
      void buttonClicked(QAbstractButton*);
      void newClicked();

   signals:

   public:
      TextStyleDialog(QWidget* parent, Score*);
      void setPage(QString);
      ~TextStyleDialog();
      };
}

#endif

