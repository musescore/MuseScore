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

#ifndef __MMRESTRANGE_H__
#define __MMRESTRANGE_H__

#include "measurenumber.h"
#include "property.h"

namespace Ms {

//---------------------------------------------------------
//   MMRestRange
//---------------------------------------------------------

class MMRestRange : public MeasureNumber {

      /// Bracketing: [18-24], (18-24) or 18-24
      M_PROPERTY (MMRestRangeBracketType, bracketType, setBracketType)

   public:
      MMRestRange(Score* s = nullptr);
      MMRestRange(const MMRestRange& other);

      virtual ElementType type()   const override   { return ElementType::MMREST_RANGE; }
      virtual MMRestRange* clone() const override   { return new MMRestRange(*this); }

      virtual QVariant getProperty(Pid id) const override;
      virtual bool setProperty(Pid id, const QVariant& val) override;
      virtual QVariant propertyDefault(Pid id) const override;

      virtual bool readProperties(XmlReader&) override;

      virtual void setXmlText(const QString&) override;
      };

} // namespace Ms

#endif
