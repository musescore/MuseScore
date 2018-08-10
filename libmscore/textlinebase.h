//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTLINEBASE_H__
#define __TEXTLINEBASE_H__

#include "line.h"
#include "style.h"
#include "property.h"

namespace Ms {

enum class Align : char;
class TextLineBase;
class Element;
class Text;

//---------------------------------------------------------
//   @@ TextLineBaseSegment
//---------------------------------------------------------

class TextLineBaseSegment : public LineSegment {
   protected:
      Text* _text;
      Text* _endText;
      QPointF points[4];
      int npoints;
      bool twoLines { false };

   public:
      TextLineBaseSegment(Score* s, ElementFlags f = ElementFlag::NOTHING);
      TextLineBaseSegment(const TextLineBaseSegment&);
      ~TextLineBaseSegment();

      TextLineBase* textLineBase() const            { return (TextLineBase*)spanner(); }
      virtual void draw(QPainter*) const override;

      virtual void layout() override;
      virtual void setSelected(bool f);

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual Element* propertyDelegate(Pid) override;

      virtual Shape shape() const override;
      };

//---------------------------------------------------------
//   HookType
//---------------------------------------------------------

enum class HookType : char {
      NONE, HOOK_90, HOOK_45, HOOK_90T
      };

//---------------------------------------------------------
//   @@ TextLineBase
//---------------------------------------------------------

class TextLineBase : public SLine {
      enum class LineType : char { CRESCENDO, DECRESCENDO };

      M_PROPERTY(bool,      lineVisible,           setLineVisible)
      M_PROPERTY2(HookType, beginHookType,         setBeginHookType,          HookType::NONE)
      M_PROPERTY2(HookType, endHookType,           setEndHookType,            HookType::NONE)
      M_PROPERTY(Spatium,   beginHookHeight,       setBeginHookHeight)
      M_PROPERTY(Spatium,   endHookHeight,         setEndHookHeight)

      M_PROPERTY(PlaceText, beginTextPlace,        setBeginTextPlace)
      M_PROPERTY(QString,   beginText,             setBeginText)
      M_PROPERTY(Align,     beginTextAlign,        setBeginTextAlign)
      M_PROPERTY(QString,   beginFontFamily,       setBeginFontFamily)
      M_PROPERTY(qreal,     beginFontSize,         setBeginFontSize)
      M_PROPERTY(bool,      beginFontBold,         setBeginFontBold)
      M_PROPERTY(bool,      beginFontItalic,       setBeginFontItalic)
      M_PROPERTY(bool,      beginFontUnderline,    setBeginFontUnderline)
      M_PROPERTY(QPointF,   beginTextOffset,       setBeginTextOffset)

      M_PROPERTY(PlaceText, continueTextPlace,     setContinueTextPlace)
      M_PROPERTY(QString,   continueText,          setContinueText)
      M_PROPERTY(Align,     continueTextAlign,     setContinueTextAlign)
      M_PROPERTY(QString,   continueFontFamily,    setContinueFontFamily)
      M_PROPERTY(qreal,     continueFontSize,      setContinueFontSize)
      M_PROPERTY(bool,      continueFontBold,      setContinueFontBold)
      M_PROPERTY(bool,      continueFontItalic,    setContinueFontItalic)
      M_PROPERTY(bool,      continueFontUnderline, setContinueFontUnderline)
      M_PROPERTY(QPointF,   continueTextOffset,    setContinueTextOffset)

      M_PROPERTY(PlaceText, endTextPlace,          setEndTextPlace)
      M_PROPERTY(QString,   endText,               setEndText)
      M_PROPERTY(Align,     endTextAlign,          setEndTextAlign)
      M_PROPERTY(QString,   endFontFamily,         setEndFontFamily)
      M_PROPERTY(qreal,     endFontSize,           setEndFontSize)
      M_PROPERTY(bool,      endFontBold,           setEndFontBold)
      M_PROPERTY(bool,      endFontItalic,         setEndFontItalic)
      M_PROPERTY(bool,      endFontUnderline,      setEndFontUnderline)
      M_PROPERTY(QPointF,   endTextOffset,         setEndTextOffset)

   protected:
      friend class TextLineBaseSegment;

   public:
      TextLineBase(Score* s, ElementFlags = ElementFlag::NOTHING);

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void read300(XmlReader&) override;

      virtual void writeProperties(XmlWriter& xml) const override;
      virtual bool readProperties(XmlReader& node) override;
      virtual bool readProperties300(XmlReader&) override;

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual QVariant getProperty(Pid id) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      };

}     // namespace Ms
Q_DECLARE_METATYPE(Ms::HookType);

#endif

