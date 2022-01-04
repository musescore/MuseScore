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

#include "style/style.h"
#include "types/types.h"

#include "line.h"
#include "property.h"

namespace Ms {
class TextLineBase;
class EngravingItem;
class Text;

//---------------------------------------------------------
//   @@ TextLineBaseSegment
//---------------------------------------------------------

class TextLineBaseSegment : public LineSegment
{
protected:
    Text* _text;
    Text* _endText;
    mu::PointF points[6];
    int npoints;
    qreal lineLength;
    bool twoLines { false };

public:
    TextLineBaseSegment(const ElementType& type, Spanner*, System* parent, ElementFlags f = ElementFlag::NOTHING);
    TextLineBaseSegment(const TextLineBaseSegment&);
    ~TextLineBaseSegment();

    TextLineBase* textLineBase() const { return (TextLineBase*)spanner(); }
    void draw(mu::draw::Painter*) const override;

    void layout() override;
    void setSelected(bool f) override;

    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

    EngravingItem* propertyDelegate(Pid) override;

    Shape shape() const override;

    bool setProperty(Pid id, const mu::engraving::PropertyValue& v) override;
};

//---------------------------------------------------------
//   @@ TextLineBase
//---------------------------------------------------------

class TextLineBase : public SLine
{
    M_PROPERTY(bool,      lineVisible,           setLineVisible)
    M_PROPERTY2(HookType, beginHookType,         setBeginHookType,          HookType::NONE)
    M_PROPERTY2(HookType, endHookType,           setEndHookType,            HookType::NONE)
    M_PROPERTY(Spatium,   beginHookHeight,       setBeginHookHeight)
    M_PROPERTY(Spatium,   endHookHeight,         setEndHookHeight)

    M_PROPERTY(TextPlace, beginTextPlace,        setBeginTextPlace)
    M_PROPERTY(QString,   beginText,             setBeginText)
    M_PROPERTY(Align,     beginTextAlign,        setBeginTextAlign)
    M_PROPERTY(QString,   beginFontFamily,       setBeginFontFamily)
    M_PROPERTY(qreal,     beginFontSize,         setBeginFontSize)
    M_PROPERTY(FontStyle, beginFontStyle,        setBeginFontStyle)
    M_PROPERTY(mu::PointF,   beginTextOffset,       setBeginTextOffset)

    M_PROPERTY(TextPlace, continueTextPlace,     setContinueTextPlace)
    M_PROPERTY(QString,   continueText,          setContinueText)
    M_PROPERTY(Align,     continueTextAlign,     setContinueTextAlign)
    M_PROPERTY(QString,   continueFontFamily,    setContinueFontFamily)
    M_PROPERTY(qreal,     continueFontSize,      setContinueFontSize)
    M_PROPERTY(FontStyle, continueFontStyle,     setContinueFontStyle)
    M_PROPERTY(mu::PointF,   continueTextOffset,    setContinueTextOffset)

    M_PROPERTY(TextPlace, endTextPlace,          setEndTextPlace)
    M_PROPERTY(QString,   endText,               setEndText)
    M_PROPERTY(Align,     endTextAlign,          setEndTextAlign)
    M_PROPERTY(QString,   endFontFamily,         setEndFontFamily)
    M_PROPERTY(qreal,     endFontSize,           setEndFontSize)
    M_PROPERTY(FontStyle, endFontStyle,          setEndFontStyle)
    M_PROPERTY(mu::PointF,   endTextOffset,         setEndTextOffset)

protected:
    friend class TextLineBaseSegment;

public:
    TextLineBase(const ElementType& type, EngravingItem* parent, ElementFlags = ElementFlag::NOTHING);

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;

    void writeProperties(XmlWriter& xml) const override;
    bool readProperties(XmlReader& node) override;

    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

    mu::engraving::PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    Pid propertyId(const QStringRef& xmlName) const override;
};
}     // namespace Ms

#endif
