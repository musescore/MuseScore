//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tempotext.cpp -1   $
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/tempotext.h"
#include "tempoproperties.h"
#include "libmscore/tempo.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"

//---------------------------------------------------------
//   TempoProperties
//---------------------------------------------------------

TempoProperties::TempoProperties(TempoText* tt, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tempoText = tt;
      tempo->setValue(tempoText->tempo() * 60.0);
      followText->setChecked(tempoText->followText());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void TempoProperties::saveValues()
      {
      Score* score    = tempoText->score();
      double newTempo = tempo->value() / 60.0;
      if ((newTempo != tempoText->tempo()) || (followText->isChecked() != tempoText->followText())) {
            TempoText* ntt = new TempoText(*tempoText);
            ntt->setTempo(newTempo);
            ntt->setFollowText(followText->isChecked());
            score->undoChangeElement(tempoText, ntt);
            }
      }

