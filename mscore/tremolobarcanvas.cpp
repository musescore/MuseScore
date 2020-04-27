//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2019 Werner Schweer and others
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

#include "tremolobarcanvas.h"

namespace Ms {

//---------------------------------------------------------
//   TremoloBarCanvas
//---------------------------------------------------------

TremoloBarCanvas::TremoloBarCanvas(QWidget* parent)
   : GridCanvas(parent)
      {
      setRows(33);
      setColumns(13);

      setPrimaryColumnsInterval(3);
      setPrimaryRowsInterval(8);

      setShowNegativeRows(true);
      }


} // namespace Ms
