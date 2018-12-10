//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorTextLine.h"
#include "inspectorHairpin.h"
#include "musescore.h"
#include "libmscore/hairpin.h"
#include "libmscore/score.h"
// #include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

InspectorHairpin::InspectorHairpin(QWidget* parent)
//   : InspectorElementBase(parent)
   : InspectorTextLineBase(parent)
      {
      h.setupUi(addWidget());

      h.hairpinType->clear();
      h.hairpinType->addItem(tr("Crescendo Hairpin"),   int(HairpinType::CRESC_HAIRPIN));
      h.hairpinType->addItem(tr("Decrescendo Hairpin"), int(HairpinType::DECRESC_HAIRPIN) );
      h.hairpinType->addItem(tr("Crescendo Line"),      int(HairpinType::CRESC_LINE));
      h.hairpinType->addItem(tr("Decrescendo Line"),    int(HairpinType::DECRESC_LINE));

      const std::vector<InspectorItem> il = {
            { Pid::HAIRPIN_CIRCLEDTIP,   0, h.hairpinCircledTip,   h.resetHairpinCircledTip },
            { Pid::HAIRPIN_TYPE,         0, h.hairpinType,         0                        },
            { Pid::PLACEMENT,            0, h.placement,           h.resetPlacement         },
            { Pid::DYNAMIC_RANGE,        0, h.dynRange,            h.resetDynRange          },
            { Pid::VELO_CHANGE,          0, h.veloChange,          h.resetVeloChange        },
            { Pid::HAIRPIN_HEIGHT,       0, h.hairpinHeight,       h.resetHairpinHeight     },
            { Pid::HAIRPIN_CONT_HEIGHT,  0, h.hairpinContHeight,   h.resetHairpinContHeight },
            };
      const std::vector<InspectorPanel> ppList = {
            { h.title, h.panel }
            };
      mapSignals(il, ppList);
      }

//---------------------------------------------------------
//   updateLineType
//---------------------------------------------------------

void InspectorHairpin::updateLineType()
      {
      HairpinSegment* hs = toHairpinSegment(inspector->element());
      Hairpin* hp = hs->hairpin();
      bool userDash = hp->lineStyle() == Qt::CustomDashLine;

      l.dashLineLength->setVisible(userDash);
      l.dashGapLength->setVisible(userDash);
      l.resetDashLineLength->setVisible(userDash);
      l.resetDashGapLength->setVisible(userDash);
      l.dashLineLengthLabel->setVisible(userDash);
      l.dashGapLengthLabel->setVisible(userDash);
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorHairpin::valueChanged(int idx)
      {
      InspectorTextLineBase::valueChanged(idx);
      if (iList[idx].t == Pid::LINE_STYLE)
            updateLineType();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorHairpin::setElement()
      {
      InspectorTextLineBase::setElement();
      updateLineType();
      }

//---------------------------------------------------------
//   postInit
//---------------------------------------------------------

void InspectorHairpin::postInit()
      {
      bool useTextLine = h.hairpinType->currentIndex() == int(HairpinType::CRESC_LINE)
         || h.hairpinType->currentIndex() == int(HairpinType::DECRESC_LINE);
      l.lineVisible->setEnabled(useTextLine);
      h.hairpinCircledTip->setDisabled(useTextLine);
      h.hairpinHeight->setDisabled(useTextLine);
      h.hairpinContHeight->setDisabled(useTextLine);
      }

}

