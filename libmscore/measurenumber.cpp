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

#include "score.h"
#include "measurenumber.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   measureNumberStyle
//---------------------------------------------------------

static const ElementStyle measureNumberStyle {
      { Sid::measureNumberVPlacement, Pid::PLACEMENT },
      { Sid::measureNumberHPlacement, Pid::HPLACEMENT },
      };

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

MeasureNumber::MeasureNumber(Score* s, Tid tid)
      : MeasureNumberBase(s, tid)
      {
      initElementStyle(&measureNumberStyle);

      setHPlacement(score()->styleV(Sid::measureNumberHPlacement).value<HPlacement>());
      }

//---------------------------------------------------------
//   MeasureNumber
//     Copy constructor
//---------------------------------------------------------

MeasureNumber::MeasureNumber(const MeasureNumber& other): MeasureNumberBase(other)
      {
      initElementStyle(&measureNumberStyle);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MeasureNumber::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::MEASURE_NUMBER);
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::measureNumberVPlacement);
            case Pid::HPLACEMENT:
                  return score()->styleV(Sid::measureNumberHPlacement);
            default:
                  return MeasureNumberBase::propertyDefault(id);
            }
      }

} // namespace MS
