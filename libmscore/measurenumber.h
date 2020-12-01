//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef __MEASURENUMBER_H__
#define __MEASURENUMBER_H__

#include "measurenumberbase.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

class MeasureNumber : public MeasureNumberBase {

   public:
      MeasureNumber(Score* = nullptr, Tid tid = Tid::MEASURE_NUMBER);
      MeasureNumber(const MeasureNumber& other);

      virtual ElementType type() const override       { return ElementType::MEASURE_NUMBER; }
      virtual MeasureNumber* clone() const override   { return new MeasureNumber(*this); }

      virtual QVariant propertyDefault(Pid id) const override;
      };

}     // namespace Ms

#endif
