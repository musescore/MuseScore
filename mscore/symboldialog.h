//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: symboldialog.h 4341 2011-06-06 08:18:18Z lasconic $
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

#ifndef __SYMBOLDIALOG_H__
#define __SYMBOLDIALOG_H__

#include "ui_symboldialog.h"

namespace Ms {

class Palette;
class Element;


//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

class SymbolDialog : public QWidget, Ui::SymbolDialogBase {
      Q_OBJECT

      Palette* sp;
      void createSymbolPalette();
      void createSymbols();

   private slots:
      void systemFlagChanged(int);
      void systemFontChanged(int);
      void on_search_textChanged(const QString &searchPhrase);
      void on_clearSearch_clicked();

   public:
      SymbolDialog(QWidget* parent = 0);
      };
}

#endif
