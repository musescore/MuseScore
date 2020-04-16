//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "editraster.h"
#include "libmscore/mscore.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   EditRaster
//---------------------------------------------------------

EditRaster::EditRaster(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("EditRaster");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      hraster->setValue(MScore::hRaster());
      vraster->setValue(MScore::vRaster());

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditRaster::accept()
      {
      MScore::setHRaster(hraster->value());
      MScore::setVRaster(vraster->value());
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditRaster::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }

}

