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

namespace Ms {

extern Score* gscore;

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void SymbolDialog::createSymbolPalette()
      {
      sp = new Palette();
      createSymbols();
      }

//---------------------------------------------------------
//   createSymbols
//---------------------------------------------------------

void SymbolDialog::createSymbols()
      {
      int currentIndex = fontList->currentIndex();
      const ScoreFont* f = &ScoreFont::scoreFonts()[currentIndex];
      // init the font if not done yet
      ScoreFont::fontFactory(f->name());
      sp->clear();
      for (int i = 0; i < int(SymId::lastSym); ++i) {
            if (f->isValid(SymId(i))) {
                  Symbol* s = new Symbol(gscore);
                  s->setSym(SymId(i), f);
                  bool match = true;
                  if (!search->text().isEmpty())
                        match = Sym::id2userName(SymId(i)).contains(search->text(), Qt::CaseInsensitive);
                  if (match)
                        sp->append(s, Sym::id2userName(SymId(i)));
                  }
            }
      sp->update();
      }

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(QWidget* parent)
   : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
      {
      setupUi(this);
      int idx = 0;
      int currentIndex = 0;
      for (const ScoreFont& f : ScoreFont::scoreFonts()) {
            fontList->addItem(f.name());
            if (f.name() == "Bravura")
                  currentIndex = idx;
            ++idx;
            }
      fontList->setCurrentIndex(currentIndex);

      setWindowTitle(tr("Symbols"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      createSymbolPalette();

      QScrollArea* sa = new PaletteScrollArea(sp);
      l->addWidget(sa);

      sp->setAcceptDrops(false);
      sp->setDrawGrid(true);
      sp->setSelectable(true);

      connect(systemFlag, SIGNAL(stateChanged(int)), SLOT(systemFlagChanged(int)));
      connect(fontList, SIGNAL(currentIndexChanged(int)), SLOT(systemFontChanged(int)));

      sa->setWidget(sp);
      }

//---------------------------------------------------------
//   systemFlagChanged
//---------------------------------------------------------

void SymbolDialog::systemFlagChanged(int state)
      {
      bool sysFlag = state == Qt::Checked;
      for (int i = 0; i < sp->size(); ++i) {
            Element* e = sp->element(i);
            if (e && e->type() == Element::Type::SYMBOL)
                  static_cast<Symbol*>(e)->setSystemFlag(sysFlag);
            }
      }

//---------------------------------------------------------
//   systemFontChanged
//---------------------------------------------------------

void SymbolDialog::systemFontChanged(int)
      {
      createSymbols();
      }

void SymbolDialog::on_search_textChanged(const QString &searchPhrase)
      {
      Q_UNUSED(searchPhrase);
      createSymbols();
      }

void SymbolDialog::on_clearSearch_clicked()
      {
      search->clear();
      createSymbols();
      }

}

