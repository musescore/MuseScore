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

#ifndef __AWLPITCHEDIT_H__
#define __AWLPITCHEDIT_H__

namespace Awl {

//---------------------------------------------------------
//   PitchEdit
//---------------------------------------------------------

class PitchEdit : public QSpinBox {
      Q_OBJECT

      bool deltaMode;

   protected:
      virtual QString textFromValue(int v) const;
      virtual int valueFromText(const QString& text) const;
      virtual void keyPressEvent(QKeyEvent*);

   signals:
      void returnPressed();
      void escapePressed();

   public:
      PitchEdit(QWidget* parent = 0);
      void setDeltaMode(bool);
      };
}

#endif
