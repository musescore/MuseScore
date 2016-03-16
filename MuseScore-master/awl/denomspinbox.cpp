//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2009 by Werner Schweer and others
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

#include "denomspinbox.h"

namespace Awl {

//---------------------------------------------------------
//   DenominatorSpinBox
//---------------------------------------------------------

DenominatorSpinBox::DenominatorSpinBox(QWidget* parent)
   : QSpinBox(parent)
      {
      setValue(4);
      setRange(1, 256);
      }

//---------------------------------------------------------
//   stepBy
//---------------------------------------------------------

void DenominatorSpinBox::stepBy(int steps)
      {
      int v = value();
      if (steps < 0)
            v = v / 2;
      else
            v = v * 2;
      if (v < 1)
            v = 1;
      if (v > 256)
            v = 256;
      setValue(v);
      }

//---------------------------------------------------------
//   validate
//---------------------------------------------------------

QValidator::State DenominatorSpinBox::validate(QString& input, int& /*pos*/) const
      {
      if (input.isEmpty())
            return QValidator::Intermediate;
      bool ok;
      int val = input.toInt(&ok);
      if (!ok)
            return QValidator::Invalid;
      if (val && (val - 1) != 0)
            return QValidator::Intermediate;
      return QValidator::Acceptable;
      }

//---------------------------------------------------------
//   fixup
//---------------------------------------------------------

void DenominatorSpinBox::fixup(QString& input) const
      {
      bool ok;
      int val = input.toInt(&ok);
      if (!ok) {
            input = QString("4");
            return;
            }
      if (val == 1)
            return;
      if (val < 1 || val > 256) {
            input = QString("4");
            return;
            }
      double v = round(log2(double(val)));
      val = int(exp2(v));
      input = QString("%1").arg(val);
      }

}

