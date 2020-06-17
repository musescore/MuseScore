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
#include "libmscore/types.h"
#include "inspectoriconloader.h"

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

      alignLeft->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_JUSTIFY_LEFT_ICON));
      alignRight->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_JUSTIFY_RIGHT_ICON));
      alignHCenter->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_JUSTIFY_CENTER_ICON));
      alignVCenter->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_ALIGN_V_CENTER_ICON));
      alignTop->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_ALIGN_V_TOP_ICON));
      alignBaseline->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_ALIGN_V_BASELINE_ICON));
      alignBottom->setIcon(*InspectorIconLoader::icon(InspectorIconLoader::TEXT_ALIGN_V_BOTTOM_ICON));

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

