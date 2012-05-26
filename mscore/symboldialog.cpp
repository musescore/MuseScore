//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: symboldialog.cpp 5384 2012-02-27 12:21:49Z wschweer $
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "symboldialog.h"
#include "palette.h"
#include "musescore.h"
#include "libmscore/sym.h"
#include "libmscore/style.h"
#include "libmscore/element.h"
#include "libmscore/symbol.h"
#include "preferences.h"

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void SymbolDialog::createSymbolPalette()
      {
      sp = new Palette();
      for (int i = 0; i < lastSym; ++i)
            sp->append(i);
      }

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Symbols"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      createSymbolPalette();
      QScrollArea* sa = new PaletteScrollArea(sp);
      l->addWidget(sa);

      sp->setAcceptDrops(false);
      sp->setDrawGrid(true);
      sp->setSelectable(true);

      connect(systemFlag, SIGNAL(stateChanged(int)), SLOT(systemFlagChanged(int)));

      sa->setWidget(sp);
      }

//---------------------------------------------------------
//   systemFlagChanged
//---------------------------------------------------------

void SymbolDialog::systemFlagChanged(int state)
      {
//      bool sysFlag = false;
//      if (state == Qt::Checked)
//            sysFlag = true;
//      for (int i = 0; i < sp->size(); ++i) {
//            Element* e = sp->element(i);
//            if (e)
//                  e->setSystemFlag(sysFlag);
//            }
      }


