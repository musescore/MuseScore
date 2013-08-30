//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "lineproperties.h"
#include "textproperties.h"
#include "libmscore/textline.h"
#include "libmscore/style.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/xml.h"
#include "libmscore/utils.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "libmscore/sym.h"
#include "libmscore/text.h"

namespace Ms {

//---------------------------------------------------------
//   populateLineSymbolComboBox
//---------------------------------------------------------

static void populateLineSymbolComboBox(QComboBox* cb)
      {
      cb->clear();
      cb->addItem(cb->tr("no symbol"), noSym);
      for (int i = 0; i < lastSym; ++i)
            cb->addItem(Sym::id2userName(SymId(i)), i);
      }

//---------------------------------------------------------
//   setLineSymbolComboBox
//---------------------------------------------------------

static void setLineSymbolComboBox(QComboBox* cb, int sym)
      {
      for (int i = 0; i < cb->count(); ++i) {
            if (cb->itemData(i).toInt() == sym) {
                  cb->setCurrentIndex(i);
                  return;
                  }
            }
      qDebug("setLineSymbol: not found %d\n", sym);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLine* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      tl = l;

      populateLineSymbolComboBox(beginSymbol);
      populateLineSymbolComboBox(continueSymbol);
      populateLineSymbolComboBox(endSymbol);

      if (tl->beginText())
            _beginText = new Text(*tl->beginText());
      else
            _beginText = 0;
      if (tl->continueText())
            _continueText = new Text(*tl->continueText());
      else
            _continueText = 0;

      beginTextRb->setChecked(tl->beginText());
      continueTextRb->setChecked(tl->continueText());
      beginSymbolRb->setChecked(tl->beginSymbol() != -1);
      continueSymbolRb->setChecked(tl->continueSymbol() != -1);
      endSymbolRb->setChecked(tl->endSymbol() != -1);

      bool bt = beginTextRb->isChecked();
      beginText->setEnabled(bt);
      beginTextTb->setEnabled(bt);
      beginTextPlace->setEnabled(bt);
      bt = beginSymbolRb->isChecked();
      beginSymbol->setEnabled(bt);
      beginSymbolX->setEnabled(bt);
      beginSymbolY->setEnabled(bt);

      bt = continueTextRb->isChecked();
      continueText->setEnabled(bt);
      continueTextTb->setEnabled(bt);
      continueTextPlace->setEnabled(bt);
      bt = continueSymbolRb->isChecked();
      continueSymbol->setEnabled(bt);
      continueSymbolX->setEnabled(bt);
      continueSymbolY->setEnabled(bt);

      beginText->setText(tl->beginText() ? tl->beginText()->text() : "");
      continueText->setText(tl->continueText() ? tl->continueText()->text() : "");

      setLineSymbolComboBox(beginSymbol, tl->beginSymbol());
      setLineSymbolComboBox(continueSymbol, tl->continueSymbol());
      setLineSymbolComboBox(endSymbol, tl->endSymbol());

      beginSymbolX->setValue(tl->beginSymbolOffset().x());
      beginSymbolY->setValue(tl->beginSymbolOffset().y());
      continueSymbolX->setValue(tl->continueSymbolOffset().x());
      continueSymbolY->setValue(tl->continueSymbolOffset().y());
      endSymbolX->setValue(tl->endSymbolOffset().x());
      endSymbolY->setValue(tl->endSymbolOffset().y());

      int idx = 0;
      switch(tl->beginTextPlace()) {
            case PLACE_ABOVE: idx = 0; break;
            case PLACE_BELOW: idx = 1; break;
            case PLACE_LEFT:  idx = 2; break;
            default:
                  qDebug("illegal text placement\n");
            }
      beginTextPlace->setCurrentIndex(idx);

      idx = 0;
      switch(tl->continueTextPlace()) {
            case PLACE_ABOVE: idx = 0; break;
            case PLACE_BELOW: idx = 1; break;
            case PLACE_LEFT:  idx = 2; break;
            default:
                  qDebug("illegal text placement\n");
            }
      continueTextPlace->setCurrentIndex(idx);

      beginHook->setChecked(tl->beginHook());
      endHook->setChecked(tl->endHook());
      beginHookHeight->setValue(tl->beginHookHeight().val());
      endHookHeight->setValue(tl->endHookHeight().val());
      beginHookType90->setChecked(tl->beginHookType() == HOOK_90);
      beginHookType45->setChecked(tl->beginHookType() == HOOK_45);
      endHookType90->setChecked(tl->endHookType() == HOOK_90);
      endHookType45->setChecked(tl->endHookType() == HOOK_45);

      connect(beginTextRb, SIGNAL(toggled(bool)), SLOT(beginTextToggled(bool)));
      connect(beginSymbolRb, SIGNAL(toggled(bool)), SLOT(beginSymbolToggled(bool)));
      connect(continueTextRb, SIGNAL(toggled(bool)), SLOT(continueTextToggled(bool)));
      connect(continueSymbolRb, SIGNAL(toggled(bool)), SLOT(continueSymbolToggled(bool)));
      connect(beginTextTb, SIGNAL(clicked()), SLOT(beginTextProperties()));
      connect(continueTextTb, SIGNAL(clicked()), SLOT(continueTextProperties()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      tl->setBeginHookHeight(Spatium(beginHookHeight->value()));
      tl->setBeginHook(beginHook->isChecked());
      tl->setEndHookHeight(Spatium(endHookHeight->value()));
      tl->setEndHook(endHook->isChecked());
      tl->setBeginHookType(beginHookType90->isChecked() ? HOOK_90 : HOOK_45);
      tl->setEndHookType(endHookType90->isChecked() ? HOOK_90 : HOOK_45);

      if (beginTextRb->isChecked()) {
            if (_beginText) {
                  _beginText->setText(beginText->text());
                  tl->setBeginText(_beginText);
                  }
            else
                  tl->setBeginText(beginText->text(), tl->score()->textStyle(TEXT_STYLE_TEXTLINE));
            }
      else
            tl->setBeginText(0);

      if (continueTextRb->isChecked()) {
            if (_continueText) {
                  _continueText->setText(continueText->text());
                  tl->setContinueText(_continueText);
                  }
            else {
                  tl->setContinueText(continueText->text(), tl->score()->textStyle(TEXT_STYLE_TEXTLINE));
                  }
            }
      else
            tl->setContinueText(0);

      SymId sym = SymId(beginSymbol->itemData(beginSymbol->currentIndex()).toInt());
      tl->setBeginSymbol(beginSymbolRb->isChecked() ? sym : noSym);

      sym = SymId(continueSymbol->itemData(continueSymbol->currentIndex()).toInt());
      tl->setContinueSymbol(continueSymbolRb->isChecked() ? sym : noSym);

      sym = SymId(endSymbol->itemData(endSymbol->currentIndex()).toInt());
      tl->setEndSymbol(endSymbolRb->isChecked() ? sym : noSym);

      PlaceText p = PLACE_ABOVE;
      switch(beginTextPlace->currentIndex()) {
            case 0: p = PLACE_ABOVE; break;
            case 1: p = PLACE_BELOW; break;
            case 2: p = PLACE_LEFT; break;
            }
      tl->setBeginTextPlace(p);

      p = PLACE_ABOVE;
      switch(continueTextPlace->currentIndex()) {
            case 0: p = PLACE_ABOVE; break;
            case 1: p = PLACE_BELOW; break;
            case 2: p = PLACE_LEFT; break;
            }
      tl->setContinueTextPlace(p);

      tl->setBeginSymbolOffset(QPointF(beginSymbolX->value(), beginSymbolY->value()));
      tl->setContinueSymbolOffset(QPointF(continueSymbolX->value(), continueSymbolY->value()));
      tl->setEndSymbolOffset(QPointF(endSymbolX->value(), endSymbolY->value()));

      QDialog::accept();
      }

//---------------------------------------------------------
//   beginTextToggled
//---------------------------------------------------------

void LineProperties::beginTextToggled(bool val)
      {
      if (val)
            beginSymbolRb->setChecked(false);
      beginText->setEnabled(val);
      beginTextPlace->setEnabled(val);
      beginTextTb->setEnabled(val);
      }

//---------------------------------------------------------
//   beginSymbolToggled
//---------------------------------------------------------

void LineProperties::beginSymbolToggled(bool val)
      {
      if (val)
            beginTextRb->setChecked(false);
      beginSymbol->setEnabled(val);
      beginSymbolX->setEnabled(val);
      beginSymbolY->setEnabled(val);
      }

//---------------------------------------------------------
//   continueTextToggled
//---------------------------------------------------------

void LineProperties::continueTextToggled(bool val)
      {
      if (val)
            continueSymbolRb->setChecked(false);
      continueText->setEnabled(val);
      continueTextPlace->setEnabled(val);
      continueTextTb->setEnabled(val);
      }

//---------------------------------------------------------
//   continueSymbolToggled
//---------------------------------------------------------

void LineProperties::continueSymbolToggled(bool val)
      {
      if (val)
            continueTextRb->setChecked(false);
      continueSymbol->setEnabled(val);
      continueSymbolX->setEnabled(val);
      continueSymbolY->setEnabled(val);
      }

//---------------------------------------------------------
//   beginTextProperties
//---------------------------------------------------------

void LineProperties::beginTextProperties()
      {
      if (!_beginText) {
            _beginText = new Text(tl->score());
            _beginText->setTextStyleType(TEXT_STYLE_TEXTLINE);
            }
      _beginText->setText(beginText->text());
      TextProperties t(_beginText, this);
      if (t.exec()) {
            // TODO: delay to ok
            foreach(SpannerSegment* ls, tl->spannerSegments()) {
                  if (ls->spannerSegmentType() != SEGMENT_SINGLE && ls->spannerSegmentType() != SEGMENT_BEGIN)
                        continue;
                  TextLineSegment* tls = static_cast<TextLineSegment*>(ls);
                  if (!tls->text())
                        continue;
                  Text* t = tls->text();
                  t->setColor(tl->beginText()->color());
                  }
            }
      }

//---------------------------------------------------------
//   continueTextProperties
//---------------------------------------------------------

void LineProperties::continueTextProperties()
      {
      if (!_continueText) {
            _continueText = new Text(tl->score());
            _continueText->setTextStyleType(TEXT_STYLE_TEXTLINE);
            }
      _continueText->setText(continueText->text());
      TextProperties t(_continueText, this);
      if (t.exec()) {
            // TODO: delay to ok
            foreach(SpannerSegment* ls, tl->spannerSegments()) {
                  if (ls->spannerSegmentType() != SEGMENT_MIDDLE)
                        continue;
                  TextLineSegment* tls = static_cast<TextLineSegment*>(ls);
                  if (!tls->text())
                        continue;
                  Text* t = tls->text();
                  t->setColor(tl->continueText()->color());
                  }
            }
      }
}

