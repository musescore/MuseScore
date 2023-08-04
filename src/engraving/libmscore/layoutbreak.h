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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "engravingitem.h"
#include "draw/types/painterpath.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ LayoutBreak
///    symbols for line break, page break etc.
//---------------------------------------------------------
class LayoutBreak final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, LayoutBreak)
    DECLARE_CLASSOF(ElementType::LAYOUT_BREAK)

public:

    void setParent(MeasureBase* parent);

    LayoutBreak* clone() const override { return new LayoutBreak(*this); }
    int subtype() const override { return static_cast<int>(_layoutBreakType); }

    void setLayoutBreakType(LayoutBreakType);
    LayoutBreakType layoutBreakType() const { return _layoutBreakType; }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    MeasureBase* measure() const { return (MeasureBase*)explicitParent(); }
    double pause() const { return _pause; }
    void setPause(double v) { _pause = v; }
    bool startWithLongNames() const { return _startWithLongNames; }
    void setStartWithLongNames(bool v) { _startWithLongNames = v; }
    bool startWithMeasureOne() const { return _startWithMeasureOne; }
    void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v; }
    bool firstSystemIndentation() const { return _firstSystemIndentation; }
    void setFirstSystemIndentation(bool v) { _firstSystemIndentation = v; }

    bool isPageBreak() const { return _layoutBreakType == LayoutBreakType::PAGE; }
    bool isLineBreak() const { return _layoutBreakType == LayoutBreakType::LINE; }
    bool isSectionBreak() const { return _layoutBreakType == LayoutBreakType::SECTION; }
    bool isNoBreak() const { return _layoutBreakType == LayoutBreakType::NOBREAK; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void layout0();

    double lineWidth() const { return m_lw; }
    const RectF& iconBorderRect() const { return m_iconBorderRect; }
    const draw::PainterPath& iconPath() const { return m_iconPath; }

protected:
    void added() override;
    void removed() override;

private:

    friend class Factory;
    LayoutBreak(MeasureBase* parent = 0);
    LayoutBreak(const LayoutBreak&);

    void spatiumChanged(double oldValue, double newValue) override;

    double m_lw;
    mu::RectF m_iconBorderRect;
    mu::draw::PainterPath m_iconPath;
    double _pause;
    bool _startWithLongNames;
    bool _startWithMeasureOne;
    bool _firstSystemIndentation;
    LayoutBreakType _layoutBreakType;
};
} // namespace mu::engraving

#endif
