//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#ifndef __AWLPITCHLABEL_H__
#define __AWLPITCHLABEL_H__

namespace Awl {

//---------------------------------------------------------
//   PitchLabel
//---------------------------------------------------------

class PitchLabel : public QLabel {
      bool _pitchMode;
      int _value;
      Q_OBJECT

   protected:
      QSize sizeHint() const;

   public slots:
      void setValue(int);
      void setInt(int);
      void setPitch(int);

   public:
      PitchLabel(QWidget* parent = 0);
      int value() const { return _value; }
      void setPitchMode(bool val);
      bool pitchMode() const { return _pitchMode; }
      };
}

#endif
