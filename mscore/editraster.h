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

#ifndef __EDITRASTER_H__
#define __EDITRASTER_H__

#include "ui_editraster.h"

namespace Ms {

//---------------------------------------------------------
//   EditRaster
//---------------------------------------------------------

class EditRaster : public QDialog, public Ui::EditRaster {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);

   private slots:
      virtual void accept();

   public:
      EditRaster(QWidget* parent = 0);
      };


} // namespace Ms
#endif

