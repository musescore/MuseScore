//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorJump.h"
#include "musescore.h"
#include "libmscore/jump.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorJump
//---------------------------------------------------------

InspectorJump::InspectorJump(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iJump.setupUi(w);
      layout->addWidget(w);
      connect(iJump.jumpTo,     SIGNAL(textChanged(const QString&)), SLOT(apply()));
      connect(iJump.playUntil,  SIGNAL(textChanged(const QString&)), SLOT(apply()));
      connect(iJump.continueAt, SIGNAL(textChanged(const QString&)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorJump::setElement(Element* e)
      {
      Jump* jump = static_cast<Jump*>(e);
      iElement->setElement(jump);

      iJump.jumpTo->blockSignals(true);
      iJump.playUntil->blockSignals(true);
      iJump.continueAt->blockSignals(true);

      iJump.jumpTo->setText(jump->jumpTo());
      iJump.playUntil->setText(jump->playUntil());
      iJump.continueAt->setText(jump->continueAt());

      iJump.jumpTo->blockSignals(false);
      iJump.playUntil->blockSignals(false);
      iJump.continueAt->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorJump::apply()
      {
      Jump* jump = static_cast<Jump*>(inspector->element());

      if (iJump.jumpTo->text() != jump->jumpTo()
         || iJump.playUntil->text() != jump->playUntil()
         || iJump.continueAt->text() != jump->continueAt()) {
            Score* score = jump->score();
            score->startCmd();

            if (iJump.jumpTo->text() != jump->jumpTo())
                  score->undoChangeProperty(jump, P_JUMP_TO, iJump.jumpTo->text());
            if (iJump.playUntil->text() != jump->playUntil())
                  score->undoChangeProperty(jump, P_PLAY_UNTIL, iJump.playUntil->text());
            if (iJump.continueAt->text() != jump->continueAt())
                  score->undoChangeProperty(jump, P_CONTINUE_AT, iJump.continueAt->text());

            score->endCmd();
            mscore->endCmd();
            }
      }

