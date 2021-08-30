/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __MEASURENUMBERBASE_H__
#define __MEASURENUMBERBASE_H__

#include "textbase.h"

namespace Ms {
//---------------------------------------------------------
//   MeasureNumberBase
///   The basic element making measure numbers.
///   Reimplemented by MMRestRange
//---------------------------------------------------------

class MeasureNumberBase : public TextBase
{
    M_PROPERTY(HPlacement, hPlacement, setHPlacement)    // Horizontal Placement

public:
    MeasureNumberBase(const ElementType& type, Measure* parent = nullptr, Tid = Tid::DEFAULT);
    MeasureNumberBase(const MeasureNumberBase& other);

    virtual QVariant getProperty(Pid id) const override;
    virtual bool setProperty(Pid id, const QVariant& val) override;
    virtual QVariant propertyDefault(Pid id) const override;

    virtual bool readProperties(XmlReader&) override;

    virtual void layout() override;
    Measure* measure() const { return toMeasure(parent()); }

    virtual bool isEditable() const override { return false; }   // The measure numbers' text should not be editable
};
}     // namespace Ms

#endif
