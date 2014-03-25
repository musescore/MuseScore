//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
//   setTextPlace
//---------------------------------------------------------

static void setTextPlace(PlaceText place, QComboBox* cb)
      {
      int idx = 0;
      switch (place) {
            case PLACE_ABOVE: idx = 0; break;
            case PLACE_BELOW: idx = 1; break;
            case PLACE_LEFT:  idx = 2; break;
            default:
                  qDebug("illegal text placement");
            }
      cb->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLine* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      otl = l;
      tl  = l->clone();

      beginText->setText(otl->beginText());
      continueText->setText(otl->continueText());
      endText->setText(otl->endText());

      setTextPlace(otl->beginTextPlace(),    beginTextPlace);
      setTextPlace(otl->continueTextPlace(), continueTextPlace);
      setTextPlace(otl->endTextPlace(),      endTextPlace);

      beginHook->setChecked(otl->beginHook());
      endHook->setChecked(otl->endHook());
      beginHookHeight->setValue(otl->beginHookHeight().val());
      endHookHeight->setValue(otl->endHookHeight().val());
      beginHookType90->setChecked(otl->beginHookType() == HOOK_90);
      beginHookType45->setChecked(otl->beginHookType() == HOOK_45);
      endHookType90->setChecked(otl->endHookType() == HOOK_90);
      endHookType45->setChecked(otl->endHookType() == HOOK_45);

      connect(beginTextTb, SIGNAL(clicked()),    SLOT(beginTextProperties()));
      connect(continueTextTb, SIGNAL(clicked()), SLOT(continueTextProperties()));
      connect(endTextTb, SIGNAL(clicked()),      SLOT(endTextProperties()));
      }
//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::~LineProperties()
      {
      delete tl;
      }

//---------------------------------------------------------
//   getPlaceText
//---------------------------------------------------------

static PlaceText getPlaceText(QComboBox* cb)
      {
      PlaceText p = PLACE_ABOVE;
      switch(cb->currentIndex()) {
            case 0: p = PLACE_ABOVE; break;
            case 1: p = PLACE_BELOW; break;
            case 2: p = PLACE_LEFT; break;
            }
      return p;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      if (beginHook->isChecked() != otl->beginHook())
            otl->undoChangeProperty(P_BEGIN_HOOK, beginHook->isChecked());
      if (endHook->isChecked() != otl->endHook())
            otl->undoChangeProperty(P_END_HOOK, endHook->isChecked());

      HookType ht = beginHookType90->isChecked() ? HOOK_90 : HOOK_45;
      if (ht != otl->beginHookType())
            otl->undoChangeProperty(P_BEGIN_HOOK_TYPE, ht);
      ht = endHookType90->isChecked() ? HOOK_90 : HOOK_45;
      if (ht != otl->endHookType())
            otl->undoChangeProperty(P_END_HOOK_TYPE, ht);

      Spatium val = Spatium(beginHookHeight->value());
      if (val != otl->beginHookHeight())
            otl->undoChangeProperty(P_BEGIN_HOOK_HEIGHT, QVariant(double(val.val())));
      val = Spatium(endHookHeight->value());
      if (val != otl->endHookHeight())
            otl->undoChangeProperty(P_END_HOOK_HEIGHT, QVariant(double(val.val())));

      PlaceText pt = getPlaceText(beginTextPlace);
      if (pt != otl->beginTextPlace()) {
            qDebug("change ottava, links %p", otl->links());
            otl->undoChangeProperty(P_BEGIN_TEXT_PLACE, pt);
            }
      pt = getPlaceText(continueTextPlace);
      if (pt != otl->continueTextPlace())
            otl->undoChangeProperty(P_CONTINUE_TEXT_PLACE, pt);
      pt = getPlaceText(endTextPlace);
      if (pt != otl->endTextPlace())
            otl->undoChangeProperty(P_END_TEXT_PLACE, pt);

      if ((tl->beginTextElement() && !tl->beginTextElement()) || (beginText->text() != otl->beginText()))
            otl->undoChangeProperty(P_BEGIN_TEXT, beginText->text());
      if ((tl->continueTextElement() && !tl->continueTextElement()) || (continueText->text() != otl->continueText()))
            otl->undoChangeProperty(P_CONTINUE_TEXT, continueText->text());
      if ((tl->endTextElement() && !tl->endTextElement()) || (endText->text() != otl->endText()))
            otl->undoChangeProperty(P_END_TEXT, endText->text());

      // TODO: apply text style changes

      Text* t = tl->beginTextElement();
      if (t) {
            Text* ot = otl->beginTextElement();
            if (t->textStyleType() != ot->textStyleType())
                  ot->undoChangeProperty(P_TEXT_STYLE_TYPE, t->textStyleType());
            // TODO: only save style difference
            if (t->textStyle() != ot->textStyle())
                  ot->undoChangeProperty(P_TEXT_STYLE, QVariant::fromValue(t->textStyle()));
            }

      // ...

      QDialog::accept();
      }

//---------------------------------------------------------
//   beginTextProperties
//---------------------------------------------------------

void LineProperties::beginTextProperties()
      {
      if (!tl->beginTextElement())
            tl->setBeginText("");         // create text element
      TextProperties t(tl->beginTextElement(), this);
      t.exec();
      }

//---------------------------------------------------------
//   continueTextProperties
//---------------------------------------------------------

void LineProperties::continueTextProperties()
      {
      if (!tl->continueTextElement())
            tl->setContinueText("");      // create Text element
      TextProperties t(tl->continueTextElement(), this);
      t.exec();
      }

//---------------------------------------------------------
//   endTextProperties
//---------------------------------------------------------

void LineProperties::endTextProperties()
      {
      if (!tl->endTextElement())
            tl->setEndText("");     // create Text element
      TextProperties t(tl->endTextElement(), this);
      t.exec();
      }
}

