//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importove.cpp 3763 2010-12-15 15:52:09Z vanferry $
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

#ifndef __IMPORTOVE_H__
#define __IMPORTOVE_H__

#include "libmscore/mscore.h"
#include "libmscore/score.h"
Ms::Score::FileError importOve(Ms::Score*, const QString& name);

#endif
