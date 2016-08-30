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
            case PlaceText::ABOVE: idx = 0; break;
            case PlaceText::BELOW: idx = 1; break;
            case PlaceText::LEFT:  idx = 2; break;
            default:
                  qDebug("illegal text placement");
            }
      cb->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLineBase* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      tl  = l;

      beginText->setText(Text::unEscape(tl->beginText()));
      continueText->setText(Text::unEscape(tl->continueText()));
      endText->setText(Text::unEscape(tl->endText()));

      setTextPlace(tl->beginTextPlace(),    beginTextPlace);
      setTextPlace(tl->continueTextPlace(), continueTextPlace);
      setTextPlace(tl->endTextPlace(),      endTextPlace);

      beginHook->setChecked(tl->beginHook());
      endHook->setChecked(tl->endHook());
      beginHookHeight->setValue(tl->beginHookHeight().val());
      endHookHeight->setValue(tl->endHookHeight().val());
      beginHookType90->setChecked(tl->beginHookType() == HookType::HOOK_90);
      beginHookType45->setChecked(tl->beginHookType() == HookType::HOOK_45);
      endHookType90->setChecked(tl->endHookType() == HookType::HOOK_90);
      endHookType45->setChecked(tl->endHookType() == HookType::HOOK_45);

      connect(beginTextTb, SIGNAL(clicked()),    SLOT(beginTextProperties()));
      connect(continueTextTb, SIGNAL(clicked()), SLOT(continueTextProperties()));
      connect(endTextTb, SIGNAL(clicked()),      SLOT(endTextProperties()));
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::~LineProperties()
      {
      }

//---------------------------------------------------------
//   getPlaceText
//---------------------------------------------------------

static PlaceText getPlaceText(QComboBox* cb)
      {
      PlaceText p = PlaceText::ABOVE;
      switch(cb->currentIndex()) {
            case 0: p = PlaceText::ABOVE; break;
            case 1: p = PlaceText::BELOW; break;
            case 2: p = PlaceText::LEFT; break;
            }
      return p;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      if (beginHook->isChecked() != tl->beginHook())
            tl->undoChangeProperty(P_ID::BEGIN_HOOK, beginHook->isChecked());
      if (endHook->isChecked() != tl->endHook())
            tl->undoChangeProperty(P_ID::END_HOOK, endHook->isChecked());

      HookType ht = beginHookType90->isChecked() ? HookType::HOOK_90 : HookType::HOOK_45;
      if (ht != tl->beginHookType())
            tl->undoChangeProperty(P_ID::BEGIN_HOOK_TYPE, int(ht));
      ht = endHookType90->isChecked() ? HookType::HOOK_90 : HookType::HOOK_45;
      if (ht != tl->endHookType())
            tl->undoChangeProperty(P_ID::END_HOOK_TYPE, int(ht));

      Spatium val = Spatium(beginHookHeight->value());
      if (val != tl->beginHookHeight())
            tl->undoChangeProperty(P_ID::BEGIN_HOOK_HEIGHT, QVariant(double(val.val())));
      val = Spatium(endHookHeight->value());
      if (val != tl->endHookHeight())
            tl->undoChangeProperty(P_ID::END_HOOK_HEIGHT, QVariant(double(val.val())));

      PlaceText pt = getPlaceText(beginTextPlace);
      if (pt != tl->beginTextPlace()) {
            qDebug("change ottava, links %p", tl->links());
            tl->undoChangeProperty(P_ID::BEGIN_TEXT_PLACE, int(pt));
            }
      pt = getPlaceText(continueTextPlace);
      if (pt != tl->continueTextPlace())
            tl->undoChangeProperty(P_ID::CONTINUE_TEXT_PLACE, int(pt));
      pt = getPlaceText(endTextPlace);
      if (pt != tl->endTextPlace())
            tl->undoChangeProperty(P_ID::END_TEXT_PLACE, int(pt));

      if (beginText->text() != tl->beginText())
            tl->undoChangeProperty(P_ID::BEGIN_TEXT, Text::tagEscape(beginText->text()));
      else if (tl->beginText().isEmpty())
            tl->setBeginText("");
      if (continueText->text() != tl->continueText())
            tl->undoChangeProperty(P_ID::CONTINUE_TEXT, Text::tagEscape(continueText->text()));
      else if (tl->continueText().isEmpty())
            tl->setContinueText("");
      if (endText->text() != tl->endText())
            tl->undoChangeProperty(P_ID::END_TEXT, Text::tagEscape(endText->text()));
      else if (tl->endText().isEmpty())
            tl->setEndText("");

      if (tl->beginTextElement()) {
            Text* t  = tl->beginTextElement();
            Text* ot = tl->beginTextElement();
            if (t) {
                  ot->undoChangeProperty(P_ID::TEXT_STYLE_TYPE, int(t->textStyleType()));
                  ot->undoChangeProperty(P_ID::TEXT_STYLE, QVariant::fromValue(t->textStyle()));
                  }
            }

      if (tl->continueTextElement()) {
            Text* t  = tl->continueTextElement();
            Text* ot = tl->continueTextElement();
            if (t) {
                  ot->undoChangeProperty(P_ID::TEXT_STYLE_TYPE, int(t->textStyleType()));
                  ot->undoChangeProperty(P_ID::TEXT_STYLE, QVariant::fromValue(t->textStyle()));
                  }
            }

      if (tl->endTextElement()) {
            Text* t  = tl->endTextElement();
            Text* ot = tl->endTextElement();
            if (t) {
                  ot->undoChangeProperty(P_ID::TEXT_STYLE_TYPE, int(t->textStyleType()));
                  ot->undoChangeProperty(P_ID::TEXT_STYLE, QVariant::fromValue(t->textStyle()));
                  }
            }

      // ...
      QDialog::accept();
      }

//---------------------------------------------------------
//   beginTextProperties
//---------------------------------------------------------

void LineProperties::beginTextProperties()
      {
      tl->createBeginTextElement();
      TextProperties t(tl->beginTextElement(), this);
      t.exec();
      }

//---------------------------------------------------------
//   continueTextProperties
//---------------------------------------------------------

void LineProperties::continueTextProperties()
      {
      tl->createContinueTextElement();
      TextProperties t(tl->continueTextElement(), this);
      t.exec();
      }

//---------------------------------------------------------
//   endTextProperties
//---------------------------------------------------------

void LineProperties::endTextProperties()
      {
      tl->createEndTextElement();
      TextProperties t(tl->endTextElement(), this);
      t.exec();
      }
}

