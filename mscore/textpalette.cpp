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

SymId commonScoreSymbols[] = {
      SymId::accidentalFlat,
      SymId::accidentalNatural,
      SymId::accidentalSharp,
      SymId::accidentalDoubleFlat,
      SymId::accidentalDoubleSharp,
      SymId::unicodeNoteWhole,
      SymId::unicodeNoteHalfUp,
      SymId::unicodeNoteQuarterUp,
      SymId::unicodeNote8thUp,
      SymId::unicodeNote16thUp,
      SymId::unicodeNote32ndUp,
      SymId::unicodeNote64thUp,
      SymId::unicodeNote128thUp,
      SymId::unicodeAugmentationDot,
      SymId::restWholeLegerLine,
      SymId::restHalfLegerLine,
      SymId::restQuarter,
      SymId::rest8th,
      SymId::rest16th,
      SymId::rest32nd,
      SymId::rest64th,
      SymId::rest128th,
      SymId::segno,
      SymId::coda,
      SymId::segnoSerpent1,
      SymId::codaSquare,
      SymId::repeat1Bar,
      SymId::repeat2Bars,
      SymId::repeat4Bars,
      SymId::gClef,
      SymId::fClef,
      SymId::cClef,
      SymId::lyricsElisionNarrow,
      SymId::lyricsElision,
      SymId::lyricsElisionWide,
      SymId::dynamicPiano,
      SymId::dynamicMezzo,
      SymId::dynamicForte,
      SymId::dynamicNiente,
      SymId::dynamicRinforzando,
      SymId::dynamicSforzando,
      SymId::dynamicZ,
      SymId::space
      };

int commonTextSymbols[] = {
      0xa9,
      0x00c0,
      0x00c1,
      0x00c2,
      0x00c3,
      0x00c4,
      0x00c5,
      0x00c6,
      0x00c7,
      0x00c8,
      0x00c9,
      0x00ca,
      0x00cb,
      0x00cc,
      0x00cd,
      0x00ce,
      0x00cf,

      0x00d0,
      0x00d1,
      0x00d2,
      0x00d3,
      0x00d4,
      0x00d5,
      0x00d6,
      0x00d7,
      0x00d8,
      0x00d9,
      0x00da,
      0x00db,
      0x00dc,
      0x00dd,
      0x00de,
      0x00df,

      //capital letters esperanto
      0x0108,
      0x011c,
      0x0124,
      0x0134,
      0x015c,
      0x016c,

      0x00e0,
      0x00e1,
      0x00e2,
      0x00e3,
      0x00e4,
      0x00e5,
      0x00e6,
      0x00e7,
      0x00e8,
      0x00e9,
      0x00ea,
      0x00eb,
      0x00ec,
      0x00ed,
      0x00ee,
      0x00ef,

      0x00f0,
      0x00f1,
      0x00f2,
      0x00f3,
      0x00f4,
      0x00f5,
      0x00f6,
      0x00f7,
      0x00f8,
      0x00f9,
      0x00fa,
      0x00fb,
      0x00fc,
      0x00fd,
      0x00fe,
      0x00ff,
      //small letters esperanto
      0x0109,
      0x011d,
      0x0125,
      0x0135,
      0x015d,
      0x016d,

      0x00BC,
      0x00BD,
      0x00BE,
      0x2153,
      0x2154,
      0x2155,
      0x2156,
      0x2157,
      0x2158,
      0x2159,
      0x215A,
      0x215B,
      0x215C,
      0x215D,
      0x215E,

      0x0152,
      0x0153,

      // 0x203F,    // curved ligature to connect two syllables
      0x35c,    // curved ligature to connect two syllables
      0x361    // curved ligature (top)
      };

void TextPalette::populateCommon()
      {
      pCommon->clear();

      for (auto id : commonScoreSymbols) {
            Symbol* s = new Symbol(gscore);
            s->setSym(id, gscore->scoreFont());
            pCommon->append(s, Sym::id2userName(id));
            }

      for (auto id : commonTextSymbols) {
            FSymbol* fs = new FSymbol(gscore);
            fs->setCode(id);
            fs->setFont(_font);
            pCommon->append(fs, QString(id));
            }

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

