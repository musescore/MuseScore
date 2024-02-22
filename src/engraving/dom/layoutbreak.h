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

#ifndef MU_ENGRAVING_LAYOUTBREAK_H
#define MU_ENGRAVING_LAYOUTBREAK_H

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
    int subtype() const override { return static_cast<int>(m_layoutBreakType); }

    void setLayoutBreakType(LayoutBreakType);
    LayoutBreakType layoutBreakType() const { return m_layoutBreakType; }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    MeasureBase* measure() const { return (MeasureBase*)explicitParent(); }
    double pause() const { return m_pause; }
    void setPause(double v) { m_pause = v; }
    bool startWithLongNames() const { return m_startWithLongNames; }
    void setStartWithLongNames(bool v) { m_startWithLongNames = v; }
    bool startWithMeasureOne() const { return m_startWithMeasureOne; }
    void setStartWithMeasureOne(bool v) { m_startWithMeasureOne = v; }
    bool firstSystemIndentation() const { return m_firstSystemIndentation; }
    void setFirstSystemIndentation(bool v) { m_firstSystemIndentation = v; }

    bool isPageBreak() const { return m_layoutBreakType == LayoutBreakType::PAGE; }
    bool isLineBreak() const { return m_layoutBreakType == LayoutBreakType::LINE; }
    bool isSectionBreak() const { return m_layoutBreakType == LayoutBreakType::SECTION; }
    bool isNoBreak() const { return m_layoutBreakType == LayoutBreakType::NOBREAK; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void init();

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

    double m_lw = 0.0;
    mu::RectF m_iconBorderRect;
    mu::draw::PainterPath m_iconPath;
    double m_pause = 0.0;
    bool m_startWithLongNames = false;
    bool m_startWithMeasureOne = false;
    bool m_firstSystemIndentation = false;
    LayoutBreakType m_layoutBreakType = LayoutBreakType::NOBREAK;
};
} // namespace mu::engraving

#endif
