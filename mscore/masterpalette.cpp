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
#include "keyedit.h"
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

#include "timedialog.h"

namespace Ms {

extern void populateIconPalette(Palette* p, const IconAction* a);
extern Palette* newKeySigPalette();
extern Palette* newBarLinePalette();
extern Palette* newLinesPalette();
extern Palette* newAccidentalsPalette();

//---------------------------------------------------------
//   showMasterPalette
//---------------------------------------------------------

void MuseScore::showMasterPalette(const QString& s)
      {
      QAction* a = getAction("masterpalette");

      if (masterPalette == 0) {
            masterPalette = new MasterPalette(0);
            connect(masterPalette, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      masterPalette->setVisible(a->isChecked());
      if (!s.isEmpty())
            masterPalette->selectItem(s);
      masterPalette->show();
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
//   selectItem
//---------------------------------------------------------

void MasterPalette::selectItem(const QString& s)
      {
      for (int idx = 0; idx < listWidget->count(); ++idx) {
            if (listWidget->item(idx)->text() == s) {
                  listWidget->setCurrentItem(listWidget->item(idx));
                  break;
                  }
            }
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
      stack->addWidget(new KeyEditor);

      timeDialog = new TimeDialog;
      stack->addWidget(timeDialog);

      addPalette(MuseScore::newBarLinePalette());
      addPalette(MuseScore::newLinesPalette());
      addPalette(MuseScore::newArpeggioPalette());
      addPalette(MuseScore::newBreathPalette());
      addPalette(MuseScore::newBracketsPalette());
      addPalette(MuseScore::newArticulationsPalette());

      addPalette(MuseScore::newAccidentalsPalette());

      addPalette(MuseScore::newDynamicsPalette());
      addPalette(MuseScore::newFingeringPalette());
      addPalette(MuseScore::newNoteHeadsPalette());
      addPalette(MuseScore::newTremoloPalette());
      addPalette(MuseScore::newRepeatsPalette());
      addPalette(MuseScore::newTempoPalette());
      addPalette(MuseScore::newTextPalette());
      addPalette(MuseScore::newBreaksPalette());
      addPalette(MuseScore::newBagpipeEmbellishmentPalette());
      addPalette(MuseScore::newBeamPalette());
      addPalette(MuseScore::newFramePalette());

      stack->addWidget(new SymbolDialog);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::closeEvent(QCloseEvent* ev)
      {
      if (timeDialog->dirty())
            timeDialog->save();
      emit closed(false);
      QWidget::closeEvent(ev);
      }

}

