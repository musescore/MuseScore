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

#ifndef __MEASURENUMBERBASE_H__
#define __MEASURENUMBERBASE_H__

#include "textbase.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureNumberBase
///   The basic element making measure numbers.
///   Reimplemented by MMRestRange
//---------------------------------------------------------

class MeasureNumberBase : public TextBase {

      M_PROPERTY (HPlacement, hPlacement, setHPlacement) // Horizontal Placement

   public:
      MeasureNumberBase(Score* = nullptr, Tid = Tid::DEFAULT);
      MeasureNumberBase(const MeasureNumberBase& other);

      virtual QVariant getProperty(Pid id) const override;
      virtual bool setProperty(Pid id, const QVariant& val) override;
      virtual QVariant propertyDefault(Pid id) const override;

      virtual bool readProperties(XmlReader&) override;

      virtual void layout() override;
      Measure* measure() const { return toMeasure(parent()); }

      virtual bool isEditable() const override { return false; } // The measure numbers' text should not be editable
      };

}     // namespace Ms

#endif
