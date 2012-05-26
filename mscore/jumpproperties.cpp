//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: repeat.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "libmscore/repeat.h"
#include "libmscore/sym.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "globals.h"
#include "jumpproperties.h"
#include "markerproperties.h"

//---------------------------------------------------------
//   JumpProperties
//---------------------------------------------------------

JumpProperties::JumpProperties(Jump* jp, QWidget* parent)
   : QDialog(parent)
      {
      jump = jp;
      setupUi(this);
      jumpTo->setText(jump->jumpTo());
      playUntil->setText(jump->playUntil());
      continueAt->setText(jump->continueAt());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void JumpProperties::saveValues()
      {
      jump->setJumpTo(jumpTo->text());
      jump->setPlayUntil(playUntil->text());
      jump->setContinueAt(continueAt->text());
      }

//---------------------------------------------------------
//   MarkerProperties
//---------------------------------------------------------

MarkerProperties::MarkerProperties(Marker* mk, QWidget* parent)
   : QDialog(parent)
      {
      marker = mk;
      setupUi(this);
      label->setText(marker->label());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void MarkerProperties::saveValues()
      {
      marker->setLabel(label->text());
      }

