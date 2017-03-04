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

enum class SubStyle;
enum class Align : char;
class TextLineBase;
class Element;
class Text;

//---------------------------------------------------------
//   @@ TextLineBaseSegment
//---------------------------------------------------------

class TextLineBaseSegment : public LineSegment {
      Q_GADGET

   protected:
      Text* _text;
      Text* _endText;
      QPointF points[4];
      int npoints;
      bool twoLines { false };

   public:
      TextLineBaseSegment(Score* s);
      TextLineBaseSegment(const TextLineBaseSegment&);
      ~TextLineBaseSegment();

      TextLineBase* textLineBase() const            { return (TextLineBase*)spanner(); }
      virtual void draw(QPainter*) const override;

      virtual void layout() override;
      virtual void setSelected(bool f);

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual Shape shape() const override;
      };

//---------------------------------------------------------
//   HookType
//---------------------------------------------------------

enum class HookType : char {
      NONE, HOOK_90, HOOK_45
      };

//---------------------------------------------------------
//   @@ TextLineBase
//---------------------------------------------------------

class TextLineBase : public SLine {
      Q_GADGET

      enum class LineType : char { CRESCENDO, DECRESCENDO };

#define PROP(a,b,c)                \
      a _ ## b;                           \
      PropertyFlags _ ## b ## Style { PropertyFlags::STYLED }; \
      public:                        \
      const a& b() const   { return _ ## b;    } \
      void c(const a& val) { _ ## b = val; }     \
      private:

      PROP(bool,      lineVisible,           setLineVisible)
      PROP(HookType,  beginHookType,         setBeginHookType)
      PROP(HookType,  endHookType,           setEndHookType)
      PROP(Spatium,   beginHookHeight,       setBeginHookHeight)
      PROP(Spatium,   endHookHeight,         setEndHookHeight)

      PROP(PlaceText, beginTextPlace,        setBeginTextPlace)
      PROP(QString,   beginText,             setBeginText)
      PROP(Align,     beginTextAlign,        setBeginTextAlign)
      PROP(QString,   beginFontFamily,       setBeginFontFamily)
      PROP(qreal,     beginFontSize,         setBeginFontSize)
      PROP(bool,      beginFontBold,         setBeginFontBold)
      PROP(bool,      beginFontItalic,       setBeginFontItalic)
      PROP(bool,      beginFontUnderline,    setBeginFontUnderline)
      PROP(QPointF,   beginTextOffset,       setBeginTextOffset)

      PROP(PlaceText, continueTextPlace,     setContinueTextPlace)
      PROP(QString,   continueText,          setContinueText)
      PROP(Align,     continueTextAlign,     setContinueTextAlign)
      PROP(QString,   continueFontFamily,    setContinueFontFamily)
      PROP(qreal,     continueFontSize,      setContinueFontSize)
      PROP(bool,      continueFontBold,      setContinueFontBold)
      PROP(bool,      continueFontItalic,    setContinueFontItalic)
      PROP(bool,      continueFontUnderline, setContinueFontUnderline)
      PROP(QPointF,   continueTextOffset,    setContinueTextOffset)

      PROP(PlaceText, endTextPlace,          setEndTextPlace)
      PROP(QString,   endText,               setEndText)
      PROP(Align,     endTextAlign,          setEndTextAlign)
      PROP(QString,   endFontFamily,         setEndFontFamily)
      PROP(qreal,     endFontSize,           setEndFontSize)
      PROP(bool,      endFontBold,           setEndFontBold)
      PROP(bool,      endFontItalic,         setEndFontItalic)
      PROP(bool,      endFontUnderline,      setEndFontUnderline)
      PROP(QPointF,   endTextOffset,         setEndTextOffset)
#undef PROP

   protected:
      friend class TextLineBaseSegment;

   public:
      TextLineBase(Score* s);
      TextLineBase(const TextLineBase&);

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;

      virtual void writeProperties(XmlWriter& xml) const override;
      virtual bool readProperties(XmlReader& node) override;

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };

}     // namespace Ms
Q_DECLARE_METATYPE(Ms::HookType);

#endif

