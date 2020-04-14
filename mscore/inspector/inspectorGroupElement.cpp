//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "libmscore/chord.h"
#include "inspector.h"
#include "inspectorGroupElement.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorGroupElement
//---------------------------------------------------------

InspectorGroupElement::InspectorGroupElement(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      ge.setupUi(w);
      _layout->insertWidget(_layout->count()-1, w);
      ge.color->setColor(Qt::black);
      connect(ge.setColor, SIGNAL(clicked()), SLOT(setColor()));
      connect(ge.setVisible, SIGNAL(clicked()), SLOT(setVisible()));
      connect(ge.setInvisible, SIGNAL(clicked()), SLOT(setInvisible()));
      connect(ge.enableAutoplace, SIGNAL(clicked()), SLOT(enableAutoplace()));
      connect(ge.disableAutoplace, SIGNAL(clicked()), SLOT(disableAutoplace()));

      //
      // Select
      //
      QLabel* l = new QLabel;
      l->setText(tr("Select"));
      QFont font(l->font());
      font.setBold(true);
      l->setFont(font);
      l->setAlignment(Qt::AlignHCenter);
      _layout->addWidget(l);

      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->setSpacing(3);
      hbox->setContentsMargins(3,3,3,3);

      notes = new QPushButton(this);
      notes->setText(tr("Notes"));
      notes->setEnabled(true);
      notes->setObjectName("notes");
      hbox->addWidget(notes);

      graceNotes = new QPushButton(this);
      graceNotes->setText(tr("Grace Notes"));
      graceNotes->setEnabled(true);
      graceNotes->setObjectName("graceNotes");
      hbox->addWidget(graceNotes);

      rests = new QPushButton(this);
      rests->setText(tr("Rests"));
      rests->setEnabled(true);
      rests->setObjectName("rests");
      hbox->addWidget(rests);

      _layout->addLayout(hbox);
      connect(notes, SIGNAL(clicked()), SLOT(notesClicked()));
      connect(graceNotes, SIGNAL(clicked()), SLOT(graceNotesClicked()));
      connect(rests, SIGNAL(clicked()), SLOT(restsClicked()));
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void InspectorGroupElement::setColor()
      {
      if (inspector->el()->isEmpty())
            return;
      Score* score = inspector->el()->front()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {
            if (e->getProperty(Pid::COLOR) != QVariant(ge.color->color()))
                  e->undoChangeProperty(Pid::COLOR, ge.color->color());
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void InspectorGroupElement::setVisible()
      {
      if (inspector->el()->isEmpty())
            return;
      Score* score = inspector->el()->front()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {
            if (!e->getProperty(Pid::VISIBLE).toBool())
                  e->undoChangeProperty(Pid::VISIBLE, true);
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   setInvisible
//---------------------------------------------------------

void InspectorGroupElement::setInvisible()
      {
      if (inspector->el()->isEmpty())
            return;
      Score* score = inspector->el()->front()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {
            if (e->getProperty(Pid::VISIBLE).toBool())
                  e->undoChangeProperty(Pid::VISIBLE, false);
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   enableAutoplace
//---------------------------------------------------------

void InspectorGroupElement::enableAutoplace()
      {
      if (inspector->el()->isEmpty())
            return;
      Score* score = inspector->el()->front()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {
            if (!e->getProperty(Pid::AUTOPLACE).toBool())
                  e->undoChangeProperty(Pid::AUTOPLACE, true);
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   disableAutoplace
//---------------------------------------------------------

void InspectorGroupElement::disableAutoplace()
      {
      if (inspector->el()->isEmpty())
            return;
      Score* score = inspector->el()->front()->score();
      score->startCmd();
      for (Element* e : *inspector->el()) {
            if (e->getProperty(Pid::AUTOPLACE).toBool())
                  e->undoChangeProperty(Pid::AUTOPLACE, false);
            }
      score->endCmd();
      }

//---------------------------------------------------------
//   notesClicked
//---------------------------------------------------------

void InspectorGroupElement::notesClicked()
      {
      Score* score = inspector->el()->front()->score();
      QList<Element*> el = score->selection().elements();
      QList<Element*> nel;
      score->deselectAll();
      for (Element* e : el) {
            if (e->isNote()) {
                  Note* note = toNote(e);
                  //if note is not grace note, then add to selection
                  if (!note->chord()->isGrace()) {
                        nel.append(note);
                        score->selection().add(note);
                        }
                  }
            }
      score->update();
      inspector->update();
      }

//---------------------------------------------------------
//   graceNotesClicked
//---------------------------------------------------------

void InspectorGroupElement::graceNotesClicked()
      {
      Score* score = inspector->el()->front()->score();
      QList<Element*> el = score->selection().elements();
      QList<Element*> nel;
      score->deselectAll();
      for (Element* e : el) {
            if (e->isNote()) {
                  Note* note = toNote(e);
                  //if note is grace note, then add to selection
                  if (note->chord()->isGrace()) {
                        nel.append(note);
                        score->selection().add(note);
                        }
                  }
            }
      score->update();
      inspector->update();
      }

//---------------------------------------------------------
//   restsClicked
//---------------------------------------------------------

void InspectorGroupElement::restsClicked()
      {
      Score* score = inspector->el()->front()->score();
      QList<Element*> el = score->selection().elements();
      QList<Element*> nel;
      score->deselectAll();
      for (Element* e : el) {
            if (e->isRest()) {
                  nel.append(e);
                  score->selection().add(e);
                  }
            }
      score->update();
      inspector->update();
      }

}

