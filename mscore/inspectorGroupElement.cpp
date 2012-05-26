//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "inspector.h"
#include "inspectorGroupElement.h"

//---------------------------------------------------------
//   InspectorGroupElement
//---------------------------------------------------------

InspectorGroupElement::InspectorGroupElement(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      ge.setupUi(w);
      layout->addWidget(w);
      ge.color->setColor(Qt::black);
      connect(ge.setColor, SIGNAL(clicked()), SLOT(setColor()));
      connect(ge.setVisible, SIGNAL(clicked()), SLOT(setVisible()));
      connect(ge.setInvisible, SIGNAL(clicked()), SLOT(setInvisible()));
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void InspectorGroupElement::setColor()
      {
      if (inspector->el().isEmpty())
            return;
      Score* score = inspector->el().front()->score();
      score->startCmd();
      foreach(Element* e, inspector->el()) {
            if (e->getProperty(P_COLOR) != QVariant(ge.color->color()))
                  score->undoChangeProperty(e, P_COLOR, ge.color->color());
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void InspectorGroupElement::setVisible()
      {
      if (inspector->el().isEmpty())
            return;
      Score* score = inspector->el().front()->score();
      score->startCmd();
      foreach(Element* e, inspector->el()) {
            if (!e->getProperty(P_VISIBLE).toBool())
                  e->score()->undoChangeProperty(e, P_VISIBLE, true);
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   setInvisible
//---------------------------------------------------------

void InspectorGroupElement::setInvisible()
      {
      if (inspector->el().isEmpty())
            return;
      Score* score = inspector->el().front()->score();
      score->startCmd();
      foreach(Element* e, inspector->el()) {
            if (e->getProperty(P_VISIBLE).toBool())
                  e->score()->undoChangeProperty(e, P_VISIBLE, false);
            }
      score->endCmd();
      }


