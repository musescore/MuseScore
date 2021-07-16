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

#include "element.h"
#include "draw/painterpath.h"

namespace Ms {
//---------------------------------------------------------
//   @@ LayoutBreak
///    symbols for line break, page break etc.
//---------------------------------------------------------
class LayoutBreak final : public Element
{
    Q_GADGET
public:
    enum class Type {
        ///.\{
        PAGE, LINE, SECTION, NOBREAK
        ///\}
    };
private:
    Q_ENUM(Type);

    qreal lw;
    mu::RectF m_iconBorderRect;
    mu::PainterPath m_iconPath;
    qreal _pause;
    bool _startWithLongNames;
    bool _startWithMeasureOne;
    bool _firstSystemIdentation;
    Type _layoutBreakType;

    void draw(mu::draw::Painter*) const override;
    void layout0();
    void spatiumChanged(qreal oldValue, qreal newValue) override;

public:
    LayoutBreak(Score* = 0);
    LayoutBreak(const LayoutBreak&);

    LayoutBreak* clone() const override { return new LayoutBreak(*this); }
    ElementType type() const override { return ElementType::LAYOUT_BREAK; }
    int subtype() const override { return static_cast<int>(_layoutBreakType); }

    void setLayoutBreakType(Type);
    Type layoutBreakType() const { return _layoutBreakType; }

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;
    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    MeasureBase* measure() const { return (MeasureBase*)parent(); }
    qreal pause() const { return _pause; }
    void setPause(qreal v) { _pause = v; }
    bool startWithLongNames() const { return _startWithLongNames; }
    void setStartWithLongNames(bool v) { _startWithLongNames = v; }
    bool startWithMeasureOne() const { return _startWithMeasureOne; }
    void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v; }
    bool firstSystemIdentation() const { return _firstSystemIdentation; }
    void setFirstSystemIdentation(bool v) { _firstSystemIdentation = v; }

    bool isPageBreak() const { return _layoutBreakType == Type::PAGE; }
    bool isLineBreak() const { return _layoutBreakType == Type::LINE; }
    bool isSectionBreak() const { return _layoutBreakType == Type::SECTION; }
    bool isNoBreak() const { return _layoutBreakType == Type::NOBREAK; }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
};
}     // namespace Ms

#endif
