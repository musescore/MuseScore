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

#ifndef __TEXTLINEBASE_H__
#define __TEXTLINEBASE_H__

#include "types/types.h"

#include "line.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
class EngravingItem;
class Text;
class TextLineBase;

class XmlReader;
class XmlWriter;

//---------------------------------------------------------
//   @@ TextLineBaseSegment
//---------------------------------------------------------

class TextLineBaseSegment : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, TextLineBaseSegment)
protected:
    Text* _text;
    Text* _endText;
    mu::PointF points[6];
    int npoints = 0;
    double lineLength = 0;
    bool twoLines = false;

public:
    TextLineBaseSegment(const ElementType& type, Spanner*, System* parent, ElementFlags f = ElementFlag::NOTHING);
    TextLineBaseSegment(const TextLineBaseSegment&);
    ~TextLineBaseSegment();

    TextLineBase* textLineBase() const { return (TextLineBase*)spanner(); }
    void draw(mu::draw::Painter*) const override;

    void layout() override;
    void setSelected(bool f) override;

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    EngravingItem* propertyDelegate(Pid) override;

    Shape shape() const override;

    bool setProperty(Pid id, const PropertyValue& v) override;
};

//---------------------------------------------------------
//   @@ TextLineBase
//---------------------------------------------------------

class TextLineBase : public SLine
{
    OBJECT_ALLOCATOR(engraving, TextLineBase)

    M_PROPERTY(bool,       lineVisible,           setLineVisible)
    M_PROPERTY2(HookType,  beginHookType,         setBeginHookType,     HookType::NONE)
    M_PROPERTY2(HookType,  endHookType,           setEndHookType,       HookType::NONE)
    M_PROPERTY(Spatium,    beginHookHeight,       setBeginHookHeight)
    M_PROPERTY(Spatium,    endHookHeight,         setEndHookHeight)

    M_PROPERTY2(TextPlace, beginTextPlace,        setBeginTextPlace,    TextPlace::AUTO)
    M_PROPERTY(String,     beginText,             setBeginText)
    M_PROPERTY(Align,      beginTextAlign,        setBeginTextAlign)
    M_PROPERTY(String,     beginFontFamily,       setBeginFontFamily)
    M_PROPERTY(double,     beginFontSize,         setBeginFontSize)
    M_PROPERTY(FontStyle,  beginFontStyle,        setBeginFontStyle)
    M_PROPERTY(PointF,     beginTextOffset,       setBeginTextOffset)

    M_PROPERTY2(TextPlace, continueTextPlace,     setContinueTextPlace, TextPlace::AUTO)
    M_PROPERTY(String,     continueText,          setContinueText)
    M_PROPERTY(Align,      continueTextAlign,     setContinueTextAlign)
    M_PROPERTY(String,     continueFontFamily,    setContinueFontFamily)
    M_PROPERTY(double,     continueFontSize,      setContinueFontSize)
    M_PROPERTY(FontStyle,  continueFontStyle,     setContinueFontStyle)
    M_PROPERTY(PointF,     continueTextOffset,    setContinueTextOffset)

    M_PROPERTY2(TextPlace, endTextPlace,          setEndTextPlace,      TextPlace::AUTO)
    M_PROPERTY(String,     endText,               setEndText)
    M_PROPERTY(Align,      endTextAlign,          setEndTextAlign)
    M_PROPERTY(String,     endFontFamily,         setEndFontFamily)
    M_PROPERTY(double,     endFontSize,           setEndFontSize)
    M_PROPERTY(FontStyle,  endFontStyle,          setEndFontStyle)
    M_PROPERTY(PointF,     endTextOffset,         setEndTextOffset)
    M_PROPERTY(bool,       textSizeSpatiumDependent, setTextSizeSpatiumDependent)

public:
    TextLineBase(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);

    void write(XmlWriter& xml) const override;

    void writeProperties(XmlWriter& xml) const override;

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;

    static const std::array<Pid, 26>& textLineBasePropertyIds();

protected:
    friend class TextLineBaseSegment;
};

inline bool isSystemTextLine(const EngravingItem* element)
{
    return element && element->isTextLineBase() && element->systemFlag();
}
} // namespace mu::engraving

#endif
