//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "masterpalette.h"
#include "symboldialog.h"
#include "palette.h"
#include "libmscore/score.h"
#include "libmscore/clef.h"
#include "libmscore/fingering.h"
#include "libmscore/spacer.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/dynamic.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/repeat.h"
#include "libmscore/breath.h"
#include "libmscore/harmony.h"
#include "libmscore/text.h"
#include "libmscore/tempotext.h"
#include "libmscore/instrchange.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/stafftext.h"
#include "libmscore/note.h"
#include "libmscore/tremolo.h"
#include "libmscore/chordline.h"

extern void populateIconPalette(Palette* p, const IconAction* a);
extern Palette* newKeySigPalette();
extern Palette* newBarLinePalette();
extern Palette* newLinesPalette();
extern Palette* newAccidentalsPalette();

//---------------------------------------------------------
//   showMasterPalette
//---------------------------------------------------------

void MuseScore::showMasterPalette()
      {
      QAction* a = getAction("masterpalette");

      if (masterPalette == 0) {
            masterPalette = new MasterPalette(0);
            connect(masterPalette, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      masterPalette->setShown(a->isChecked());
      }

//---------------------------------------------------------
//   createPalette
//---------------------------------------------------------

Palette* MasterPalette::createPalette(int w, int h, bool grid, double mag)
      {
      Palette* sp = new Palette;
      PaletteScrollArea* psa = new PaletteScrollArea(sp);
      psa->setRestrictHeight(false);
      sp->setMag(mag);
      sp->setGrid(w, h);
      sp->setDrawGrid(grid);
      sp->setReadOnly(true);
      stack->addWidget(psa);
      return sp;
      }

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void MasterPalette::addPalette(Palette* sp)
      {
      sp->setReadOnly(true);
      PaletteScrollArea* psa = new PaletteScrollArea(sp);
      psa->setRestrictHeight(false);
      stack->addWidget(psa);
      }

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      addPalette(MuseScore::newGraceNotePalette());
      addPalette(MuseScore::newClefsPalette());
      addPalette(MuseScore::newKeySigPalette());

      stack->addWidget(new TimeDialog);

      addPalette(MuseScore::newBarLinePalette());
      addPalette(MuseScore::newKeySigPalette());
      addPalette(MuseScore::newArpeggioPalette());
      addPalette(MuseScore::newBreathPalette());
      addPalette(MuseScore::newBracketsPalette());
      addPalette(MuseScore::newArticulationsPalette());
      addPalette(MuseScore::newAccidentalsPalette());
      addPalette(MuseScore::newDynamicsPalette());
      addPalette(MuseScore::newFingeringPalette());
      addPalette(MuseScore::newNoteHeadsPalette());
      addPalette(MuseScore::newTremoloPalette());
      addPalette(MuseScore::newFallDoitPalette());
      addPalette(MuseScore::newRepeatsPalette());
      addPalette(MuseScore::newTextPalette());
      addPalette(MuseScore::newBreaksPalette());
      addPalette(MuseScore::newBeamPalette());
      addPalette(MuseScore::newFramePalette());

      stack->addWidget(new SymbolDialog);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }


