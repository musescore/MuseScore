//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2009 by Werner Schweer and others
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

#ifndef __AWLPOSLABEL_H__
#define __AWLPOSLABEL_H__

#include "libmscore/pos.h"

namespace Awl {
using namespace Ms;

//---------------------------------------------------------
//   PosLabel
//---------------------------------------------------------

class PosLabel : public QLabel {
      bool _smpte;
      Pos pos;
      Q_OBJECT

      void updateValue();

   protected:
      QSize sizeHint() const;

   public slots:
      void setValue(const Pos&);

   public:
      PosLabel(QWidget* parent = 0);
      PosLabel(TempoMap*, TimeSigMap*, QWidget* parent = 0);
      void setContext(TempoMap*, TimeSigMap*);

      Pos value() const { return pos; }

      void setSmpte(bool);
      bool smpte() const { return _smpte; }
      };

}

#endif

