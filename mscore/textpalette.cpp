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

#include "textpalette.h"
#include "icons.h"
#include "libmscore/text.h"
#include "libmscore/sym.h"
#include "libmscore/style.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   codeIcon
//---------------------------------------------------------

static QIcon codeIcon(const QString& s, QFont f)
      {
      f.setPixelSize(40);
      int w = 40;
      int h = 40;

      QWidget wi;

      QPixmap image(w, h);
      QColor bg(wi.palette().brush(QPalette::Normal, QPalette::Window).color());

      image.fill(QColor(255, 255, 255, 0));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setFont(f);

      QPen pen(wi.palette().brush(QPalette::Normal, QPalette::Text).color());

      painter.setPen(pen);
      painter.drawText(0, 0, w, h, Qt::AlignCenter, s);
      painter.end();
      return QIcon(image);
      }

//---------------------------------------------------------
//   symbolIcon
//---------------------------------------------------------

static QIcon symbolIcon(SymId id)
      {
      int w = 40;
      int h = 40;
      QPixmap image(w, h);

      QWidget wi;

      QColor bg(wi.palette().brush(QPalette::Normal, QPalette::Window).color());

      image.fill(QColor(255, 255, 255, 0));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      QPen pen(wi.palette().brush(QPalette::Normal, QPalette::Text).color());
      painter.setPen(pen);
      ScoreFont* sf = ScoreFont::fontFactory("Bravura");
      const qreal mag = 0.72;
      painter.scale(mag, mag);
      painter.setFont(sf->font());
      QRectF r(sf->bbox(id, mag));
      QPointF o((40.0 - r.width()) * .5 - r.x(), (40.0 - r.height()) * .5 - r.y());
      painter.drawText(o, sf->toString(id));

      painter.end();
      return QIcon(image);
      }

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setWindowFlags(Qt::Tool);
      setupUi(this);
      QGridLayout* gl = new QGridLayout;
      gl->setMargin(5);
      gl->setSpacing(0);
      symbolBox->setLayout(gl);
      sg = new QButtonGroup(this);

      musicalSymbols->setChecked(true);

      for (unsigned i = 0; i < 256; ++i) {
            QPushButton* tb = new QPushButton;
            buttons[i] = tb;
            tb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            tb->setFixedSize(40, 40);
            tb->setIconSize(QSize(40, 40));
            gl->addWidget(tb, i / 16, i % 16);
            sg->addButton(tb, i);
            }
      curPage = 0;
      populate();
      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));
      connect(musicalSymbols, SIGNAL(toggled(bool)), SLOT(populate()));
      connect(codePage, SIGNAL(valueChanged(int)), SLOT(populate()));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   populate
//---------------------------------------------------------

void TextPalette::populate()
      {
      bool musical = musicalSymbols->isChecked();

      int page = codePage->value();
      if (musical) {
            codePage->setMaximum((int(SymId::lastSym)+1)/256);
            int id = 0;
            for (int row = 0; row < 16; ++row) {
                  for (int col = 0; col < 16; ++col) {
                        int idx = row * 16 + col;
                        id  = idx + page * 256;
                        if (id >= int(SymId::lastSym))
                              break;
                        QPushButton* tb = buttons[idx];
                        tb->setToolTip(Sym::id2userName(SymId(id)));
                        tb->setIcon(symbolIcon(SymId(id)));
                        tb->setEnabled(true);
                        sg->setId(tb, id);
                        }
                  if (id >= int(SymId::lastSym))
                        break;
                  }
            }
      else {
            codePage->setMaximum(255);

            QFont f("FreeSerif");
            f.setPixelSize(40);

            f.setStyleStrategy(QFont::NoFontMerging);
            QFontMetrics fm(f);

            int rowOffset = 0;
            bool pageEmpty = true;

            for (int row = 0; row < 16; ++row) {
                  bool rowEmpty = true;
                  for (int col = 0; col < 16; ++col) {
                        int idx = (row - rowOffset) * 16 + col;
                        int code = row * 16 + col + page * 256;
                        QPushButton* tb = buttons[idx];
                        //
                        // Font->inFont(QChar) does only work
                        // for unicode plane 0, as QChar is only
                        // 16 bit
                        //
                        tb->setFont(f);
                        if (fm.inFontUcs4(code)) {
                              rowEmpty = false;
                              QString ss;
                              if (code & 0xffff0000) {
                                    ss  = QChar(QChar::highSurrogate(code));
                                    ss += QChar(QChar::lowSurrogate(code));
                                    }
                              else
                                    ss = QChar(code);
                              tb->setToolTip(QString("0x%1").arg(code, 5, 16, QLatin1Char('0')));
                              tb->setIcon(codeIcon(ss, f));
                              sg->setId(tb, code);
                              tb->setEnabled(true);
                              }
                        else {
                              tb->setIcon(QIcon());
                              tb->setEnabled(false);
                              sg->setId(tb, -1);      // no glyph available
                              }
                        }
                  if (rowEmpty)
                        ++rowOffset;
                  else
                        pageEmpty = false;
                  }
            for (int row = 16-rowOffset; row < 16; ++row) {
                  for (int col = 0; col < 16; ++col) {
                        int idx = row * 16 + col;
                        QPushButton* tb = buttons[idx];
                        tb->setIcon(QIcon());
                        sg->setId(tb, -1);
                        tb->setToolTip(QString(""));
                        }
                  }
            if (pageEmpty) {
                  int diff = 1;
                  if (curPage > page)
                        diff = -1;
                  curPage = page;
                  page += diff;
                  codePage->setValue(page);
                  }
            else
                  curPage = page;
            }
      }

//---------------------------------------------------------
//   symbolClicked
//---------------------------------------------------------

void TextPalette::symbolClicked(int n)
      {
      if (n == -1)
            return;
      if (musicalSymbols->isChecked())
            _textElement->insertSym(SymId(n));
      else
            _textElement->insertText(QString(QChar(n)));
      mscore->activateWindow();
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
}

