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

#ifndef __MMRESTRANGE_H__
#define __MMRESTRANGE_H__

#include "measurenumberbase.h"
#include "property.h"

namespace Ms {
//---------------------------------------------------------
//   MMRestRange
//---------------------------------------------------------

class MMRestRange : public MeasureNumberBase
{
    /// Bracketing: [18-24], (18-24) or 18-24
    M_PROPERTY(MMRestRangeBracketType, bracketType, setBracketType)

public:
    MMRestRange(Score* s = nullptr);
    MMRestRange(const MMRestRange& other);

    virtual ElementType type()   const override { return ElementType::MMREST_RANGE; }
    virtual MMRestRange* clone() const override { return new MMRestRange(*this); }

    virtual QVariant getProperty(Pid id) const override;
    virtual bool setProperty(Pid id, const QVariant& val) override;
    virtual QVariant propertyDefault(Pid id) const override;

    virtual bool readProperties(XmlReader&) override;

    virtual void setXmlText(const QString&) override;
};
} // namespace Ms

#endif
