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
#include "infrastructure/draw/painterpath.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
//---------------------------------------------------------
//   @@ LayoutBreak
///    symbols for line break, page break etc.
//---------------------------------------------------------
class LayoutBreak final : public EngravingItem
{
public:

    void setParent(MeasureBase* parent);

    LayoutBreak* clone() const override { return new LayoutBreak(*this); }
    int subtype() const override { return static_cast<int>(_layoutBreakType); }

    void setLayoutBreakType(LayoutBreakType);
    LayoutBreakType layoutBreakType() const { return _layoutBreakType; }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    MeasureBase* measure() const { return (MeasureBase*)explicitParent(); }
    qreal pause() const { return _pause; }
    void setPause(qreal v) { _pause = v; }
    bool startWithLongNames() const { return _startWithLongNames; }
    void setStartWithLongNames(bool v) { _startWithLongNames = v; }
    bool startWithMeasureOne() const { return _startWithMeasureOne; }
    void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v; }
    bool firstSystemIdentation() const { return _firstSystemIdentation; }
    void setFirstSystemIdentation(bool v) { _firstSystemIdentation = v; }

    bool isPageBreak() const { return _layoutBreakType == LayoutBreakType::PAGE; }
    bool isLineBreak() const { return _layoutBreakType == LayoutBreakType::LINE; }
    bool isSectionBreak() const { return _layoutBreakType == LayoutBreakType::SECTION; }
    bool isNoBreak() const { return _layoutBreakType == LayoutBreakType::NOBREAK; }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

protected:
    void added() override;
    void removed() override;

private:

    friend class mu::engraving::Factory;
    LayoutBreak(MeasureBase* parent = 0);
    LayoutBreak(const LayoutBreak&);

    void draw(mu::draw::Painter*) const override;
    void layout0();
    void spatiumChanged(qreal oldValue, qreal newValue) override;

    qreal lw;
    mu::RectF m_iconBorderRect;
    mu::PainterPath m_iconPath;
    qreal _pause;
    bool _startWithLongNames;
    bool _startWithMeasureOne;
    bool _firstSystemIdentation;
    LayoutBreakType _layoutBreakType;
};
}     // namespace Ms

#endif
