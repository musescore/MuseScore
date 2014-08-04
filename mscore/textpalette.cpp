//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.cpp 4612 2011-07-27 13:14:35Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "palette.h"
#include "textpalette.h"
#include "icons.h"
#include "libmscore/text.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/style.h"
#include "libmscore/clef.h"
#include "libmscore/score.h"
#include "musescore.h"

namespace Ms {

const int buttonSize = 40;
const int iconSize   = 20;
const int fontSize   = 20;

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setWindowFlags(Qt::Tool);
      setupUi(this);

      codePage->setEnabled(false);

      pCommon = new Palette;
      pCommon->setName(QT_TRANSLATE_NOOP("Palette", "common symbols"));
      pCommon->setMag(0.8);
      pCommon->setGrid(33, 60);
      pCommon->setReadOnly(true);

      pAll = new Palette;
      pAll->setName(QT_TRANSLATE_NOOP("Palette", "all font symbols"));
      pAll->setMag(0.8);
      pAll->setGrid(33, 60);
      pAll->setReadOnly(true);

      PaletteScrollArea* psa = new PaletteScrollArea(pCommon);
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      psa = new PaletteScrollArea(pAll);
      psa->setRestrictHeight(false);
      stack->addWidget(psa);

      connect(commonButton,  SIGNAL(toggled(bool)),       SLOT(commonToggled(bool)));
      connect(stack,         SIGNAL(currentChanged(int)), SLOT(pageChanged(int)));
      connect(codePage,      SIGNAL(valueChanged(int)),   SLOT(populateAll()));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   pageChanged
//---------------------------------------------------------

void TextPalette::pageChanged(int /*idx*/)
      {
      }

//---------------------------------------------------------
//   populateCommon
//---------------------------------------------------------

void TextPalette::populateCommon()
      {
      pCommon->clear();

      Symbol* s = new Symbol(gscore);
      s->setSym(SymId::gClef, gscore->scoreFont());
      pCommon->append(s, Sym::id2userName(SymId::gClef));

      FSymbol* fs = new FSymbol(gscore);
      fs->setCode(int('A'));
      fs->setFont(_font);
      pCommon->append(fs, "A");
      }

//---------------------------------------------------------
//   populateAll
//---------------------------------------------------------

void TextPalette::populateAll()
      {
      int page = codePage->value();
      codePage->setMaximum(255);

      QFontMetrics fm(_font);

      pAll->clear();
      for (int row = 0; row < 16; ++row) {
            for (int col = 0; col < 16; ++col) {
                  int code = row * 16 + col + page * 256;
                  FSymbol* fs = new FSymbol(gscore);
                  fs->setCode(code);
                  fs->setFont(_font);
                  pAll->append(fs, QString("0x%1").arg(code, 5, 16, QLatin1Char('0')));
                  }
            }
      }

//---------------------------------------------------------
//   commonToggled
//---------------------------------------------------------

void TextPalette::commonToggled(bool val)
      {
      codePage->setEnabled(!val);
      stack->setCurrentIndex(val ? 0 : 1);
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextPalette::setText(Text* te)
      {
      _textElement = te;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TextPalette::closeEvent(QCloseEvent* ev)
      {
      QWidget::closeEvent(ev);
      getAction("show-keys")->setChecked(false);
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void TextPalette::setFont(const QFont& font)
      {
      _font = font;
      _font.setPointSize(20);
      populateAll();
      populateCommon();
      update();
      }
}

