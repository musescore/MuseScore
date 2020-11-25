//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorGlissando.h"
#include "libmscore/glissando.h"
#include "libmscore/note.h"
#include "libmscore/rendermidi.h"   // For counting the number of pitches in a glissando

namespace Ms {

//---------------------------------------------------------
//   InspectorGlissando
//---------------------------------------------------------

InspectorGlissando::InspectorGlissando(QWidget* parent)
   : InspectorElementBase(parent)
      {
      g.setupUi(addWidget());

      const std::vector<InspectorItem> iiList = {
            { Pid::GLISS_TYPE,      0, g.type,           g.resetType           },
            { Pid::GLISS_TEXT,      0, g.text,           g.resetText           },
            { Pid::GLISS_SHOW_TEXT, 0, g.showText,       g.resetShowText       },
            { Pid::GLISS_STYLE,     0, g.glissandoStyle, g.resetGlissandoStyle },
            { Pid::GLISS_EASEIN,    0, g.easeInSpin,     g.resetEaseIn         },
            { Pid::GLISS_EASEOUT,   0, g.easeOutSpin,    g.resetEaseOut        },
            { Pid::PLAY,            0, g.playGlissando,  g.resetPlayGlissando  },
            { Pid::FONT_FACE,       0, g.fontFace,       g.resetFontFace       },
            { Pid::FONT_SIZE,       0, g.fontSize,       g.resetFontSize       },
            { Pid::FONT_STYLE,      0, g.fontStyle,      g.resetFontStyle      },
            };
      const std::vector<InspectorPanel> ppList = {
            { g.title, g.panel }
            };
      mapSignals(iiList, ppList);
}

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorGlissando::setElement()
      {
      InspectorElementBase::setElement();
      if (!g.playGlissando->isChecked()) {
            g.labelGlissandoStyle->setEnabled(false);
            g.glissandoStyle->setEnabled(false);
            g.resetGlissandoStyle->setEnabled(false);
            }
      if (!g.showText->isChecked())
            g.textWidget->setVisible(false);
      g.easeInSlider->setSliderPosition(g.easeInSpin->value());
      g.easeOutSlider->setSliderPosition(g.easeOutSpin->value());
      g.easeInOutCanvas->setEaseInOut(g.easeInSpin->value(), g.easeOutSpin->value());

      updateEvents();
}

//---------------------------------------------------------
//   canvasChanged
//---------------------------------------------------------

void InspectorGlissando::valueChanged(int n)
{
      InspectorElementBase::valueChanged(n);
      if (iList[n].t == Pid::GLISS_EASEIN || iList[n].t == Pid::GLISS_EASEOUT)
            g.easeInOutCanvas->setEaseInOut(g.easeInSpin->value(), g.easeOutSpin->value());
      else if (iList[n].t == Pid::GLISS_STYLE)
            updateEvents();
      update();
}

//---------------------------------------------------------
//   updateNbEvents computes the number of pitch events in the glissanso given the glissando style.
//   For example, all the half notes, only the white notes, only the black notes, etc
//---------------------------------------------------------

void InspectorGlissando::updateEvents() {
      GlissandoSegment* gs = inspector->element()->isGlissandoSegment() ? toGlissandoSegment(inspector->element()) : nullptr;
      std::vector<int> body;
      int nEvents = 25;
      int pitchStart = toNote(gs->spanner()->startElement())->ppitch();
      int pitchEnd = toNote(gs->spanner()->endElement())->ppitch();
      if (gs && glissandoPitchOffsets(gs->spanner(), body))
            nEvents = body.size();
      g.easeInOutCanvas->setNbEvents(nEvents);
      g.easeInOutCanvas->setBottomPitch(pitchEnd > pitchStart ? pitchStart : pitchEnd);
      g.easeInOutCanvas->setPitchDelta(std::abs(pitchEnd - pitchStart) + 1);
      g.easeInOutCanvas->setEvents(body);
}

}

