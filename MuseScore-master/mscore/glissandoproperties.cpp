//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: glissando.cpp -1   $
//
//  Copyright (C) 2008-2011 Werner Schweer and others
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

#include "libmscore/glissando.h"
#include "glissandoproperties.h"

namespace Ms {

//---------------------------------------------------------
//   GlissandoProperties
//---------------------------------------------------------

GlissandoProperties::GlissandoProperties(Glissando* g, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      glissando = g;
      showText->setChecked(glissando->showText());
      text->setText(glissando->text());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void GlissandoProperties::accept()
      {
      glissando->setShowText(showText->isChecked());
      glissando->setText(text->text());
      QDialog::accept();
      }

}

