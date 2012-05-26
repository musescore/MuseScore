//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: jumpproperties.h 1840 2009-05-20 11:57:51Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __JUMPPROPERTIES_H__
#define __JUMPPROPERTIES_H__

#include "ui_jumpproperties.h"

class Jump;

//---------------------------------------------------------
//   JumpProperties
//---------------------------------------------------------

class JumpProperties : public QDialog, public Ui::JumpPropertyBase {
      Q_OBJECT

      Jump* jump;

   private slots:
      void saveValues();

   public:
      JumpProperties(Jump*, QWidget* parent = 0);
      };

#endif
