//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "alignSelect.h"
#include "libmscore/elementlayout.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   AlignSelect
//---------------------------------------------------------

AlignSelect::AlignSelect(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      g1 = new QButtonGroup(this);
      g1->addButton(alignLeft);
      g1->addButton(alignHCenter);
      g1->addButton(alignRight);

      g2 = new QButtonGroup(this);
      g2->addButton(alignTop);
      g2->addButton(alignVCenter);
      g2->addButton(alignBaseline);
      g2->addButton(alignBottom);

      alignLeft->setIcon(*icons[int(Icons::textLeft_ICON)]);
      alignRight->setIcon(*icons[int(Icons::textRight_ICON)]);
      alignHCenter->setIcon(*icons[int(Icons::textCenter_ICON)]);
      alignVCenter->setIcon(*icons[int(Icons::textVCenter_ICON)]);
      alignTop->setIcon(*icons[int(Icons::textTop_ICON)]);
      alignBaseline->setIcon(*icons[int(Icons::textBaseline_ICON)]);
      alignBottom->setIcon(*icons[int(Icons::textBottom_ICON)]);

      connect(g1, SIGNAL(buttonToggled(int,bool)), SLOT(_alignChanged()));
      connect(g2, SIGNAL(buttonToggled(int,bool)), SLOT(_alignChanged()));
      }

//---------------------------------------------------------
//   _alignChanged
//---------------------------------------------------------

void AlignSelect::_alignChanged()
      {
      emit alignChanged(align());
      }

//---------------------------------------------------------
//   align
//---------------------------------------------------------

Align AlignSelect::align() const
      {
      Align a = Align::LEFT;
      if (alignHCenter->isChecked())
            a = a | Align::HCENTER;
      else if (alignRight->isChecked())
            a = a | Align::RIGHT;
      if (alignVCenter->isChecked())
            a = a | Align::VCENTER;
      else if (alignBottom->isChecked())
            a = a | Align::BOTTOM;
      else if (alignBaseline->isChecked())
            a = a | Align::BASELINE;
      return a;
      }

//---------------------------------------------------------
//   blockAlign
//---------------------------------------------------------

void AlignSelect::blockAlign(bool val)
      {
      g1->blockSignals(val);
      g2->blockSignals(val);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void AlignSelect::setAlign(Align a)
      {
      blockAlign(true);
      if (a & Align::HCENTER)
            alignHCenter->setChecked(true);
      else if (a & Align::RIGHT)
            alignRight->setChecked(true);
      else
            alignLeft->setChecked(true);
      if (a & Align::VCENTER)
            alignVCenter->setChecked(true);
      else if (a & Align::BOTTOM)
            alignBottom->setChecked(true);
      else if (a & Align::BASELINE)
            alignBaseline->setChecked(true);
      else
            alignTop->setChecked(true);
      blockAlign(false);
      }

}

