//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorHairpin.h"
#include "musescore.h"
#include "libmscore/hairpin.h"
#include "libmscore/score.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

InspectorHairpin::InspectorHairpin(QWidget* parent)
   : InspectorElementBase(parent)
      {
      l.setupUi(addWidget());
      setupLineStyle(l.lineStyle);
      h.setupUi(addWidget());

      h.hairpinType->clear();
      h.hairpinType->addItem(tr("Crescendo Hairpin"),   int(HairpinType::CRESC_HAIRPIN));
      h.hairpinType->addItem(tr("Decrescendo Hairpin"), int(HairpinType::DECRESC_HAIRPIN) );
      h.hairpinType->addItem(tr("Crescendo Line"),      int(HairpinType::CRESC_LINE));
      h.hairpinType->addItem(tr("Decrescendo Line"),    int(HairpinType::DECRESC_LINE));

      const std::vector<InspectorItem> il = {
            { Pid::LINE_VISIBLE,         0, l.lineVisible,         l.resetLineVisible       },
            { Pid::DIAGONAL,             0, l.diagonal,            l.resetDiagonal          },
            { Pid::LINE_COLOR,           0, l.lineColor,           l.resetLineColor         },
            { Pid::LINE_WIDTH,           0, l.lineWidth,           l.resetLineWidth         },
            { Pid::LINE_STYLE,           0, l.lineStyle,           l.resetLineStyle         },
            { Pid::DASH_LINE_LEN,        0, l.dashLineLength,      l.resetDashLineLength    },
            { Pid::DASH_GAP_LEN,         0, l.dashGapLength,       l.resetDashGapLength     },
            { Pid::HAIRPIN_CIRCLEDTIP,   0, h.hairpinCircledTip,   h.resetHairpinCircledTip },
            { Pid::HAIRPIN_TYPE,         0, h.hairpinType,         0                        },
            { Pid::PLACEMENT,            0, h.placement,           h.resetPlacement         },
            { Pid::DYNAMIC_RANGE,        0, h.dynRange,            h.resetDynRange          },
            { Pid::VELO_CHANGE,          0, h.veloChange,          h.resetVeloChange        },
            { Pid::HAIRPIN_HEIGHT,       0, h.hairpinHeight,       h.resetHairpinHeight     },
            { Pid::HAIRPIN_CONT_HEIGHT,  0, h.hairpinContHeight,   h.resetHairpinContHeight },
            { Pid::BEGIN_FONT_FACE,      0, h.fontFace,            h.resetFontFace          },
            { Pid::BEGIN_FONT_SIZE,      0, h.fontSize,            h.resetFontSize          },
            { Pid::BEGIN_FONT_BOLD,      0, h.fontBold,            h.resetFontBold          },
            { Pid::BEGIN_FONT_ITALIC,    0, h.fontItalic,          h.resetFontItalic        },
            { Pid::BEGIN_FONT_UNDERLINE, 0, h.fontUnderline,       h.resetFontUnderline     }
            };
      const std::vector<InspectorPanel> ppList = {
            { l.title, l.panel },
            { h.title, h.panel }
            };
      h.fontBold->setIcon(*icons[int(Icons::textBold_ICON)]);
      h.fontItalic->setIcon(*icons[int(Icons::textItalic_ICON)]);
      h.fontUnderline->setIcon(*icons[int(Icons::textUnderline_ICON)]);
      mapSignals(il, ppList);
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

