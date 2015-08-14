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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "mscore.h"
#include "line.h"
#include "style.h"

namespace Ms {

class TextLine;
class Element;
class Text;

//---------------------------------------------------------
//   @@ TextLineSegment
//---------------------------------------------------------

class TextLineSegment : public LineSegment {
      Q_OBJECT

      Text* _text        { 0 };
      Text* _endText     { 0 };

      void setText(Text*);

   public:
      TextLineSegment(Score* s) : LineSegment(s) {}
      TextLineSegment(const TextLineSegment&);
      ~TextLineSegment();

      virtual TextLineSegment* clone() const override { return new TextLineSegment(*this); }
      virtual Element::Type type() const override     { return Element::Type::TEXTLINE_SEGMENT; }
      TextLine* textLine() const             { return (TextLine*)spanner(); }
      virtual void draw(QPainter*) const override;

      virtual void layout() override;
      void layout1();
      virtual void setSelected(bool f);

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };

enum class HookType : char { HOOK_90, HOOK_45 };

//---------------------------------------------------------
//   @@ TextLine
//---------------------------------------------------------

class TextLine : public SLine {
      Q_OBJECT

      PlaceText _beginTextPlace, _continueTextPlace, _endTextPlace;

      bool _lineVisible;
      bool _beginHook, _endHook;
      HookType _beginHookType, _endHookType;
      Spatium _beginHookHeight, _endHookHeight;

   protected:
      Text *_beginText, *_continueText, *_endText;

      friend class TextLineSegment;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      ~TextLine();

      virtual TextLine* clone() const override            { return new TextLine(*this); }
      virtual Element::Type type() const override         { return Element::Type::TEXTLINE; }
      virtual void setScore(Score* s) override;
      virtual LineSegment* createLineSegment() override;

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;

      virtual void writeProperties(Xml& xml) const override;
      virtual bool readProperties(XmlReader& node) override;

      bool lineVisible() const                { return _lineVisible;          }
      void setLineVisible(bool v)             { _lineVisible = v;             }
      bool beginHook() const                  { return _beginHook;            }
      bool endHook() const                    { return _endHook;              }
      void setBeginHook(bool v)               { _beginHook = v;               }
      void setEndHook(bool v)                 { _endHook = v;                 }
      HookType beginHookType() const          { return _beginHookType;        }
      HookType endHookType() const            { return _endHookType;          }
      void setBeginHookType(HookType val)     { _beginHookType = val;         }
      void setEndHookType(HookType val)       { _endHookType = val;           }
      void setBeginHookHeight(Spatium v)      { _beginHookHeight = v;         }
      void setEndHookHeight(Spatium v)        { _endHookHeight = v;           }
      Spatium beginHookHeight() const         { return _beginHookHeight;      }
      Spatium endHookHeight() const           { return _endHookHeight;        }

      Text* beginTextElement() const          { return _beginText; }
      Text* continueTextElement() const       { return _continueText; }
      Text* endTextElement() const            { return _endText; }

      void createBeginTextElement();
      void createContinueTextElement();
      void createEndTextElement();

      void setBeginText(const QString& s, TextStyleType style);
      void setContinueText(const QString& s, TextStyleType style);
      void setEndText(const QString& s, TextStyleType style);

      void setBeginText(const QString&);
      void setContinueText(const QString&);
      void setEndText(const QString&);

      QString beginText() const;
      QString continueText() const;
      QString endText() const;

      PlaceText beginTextPlace() const        { return _beginTextPlace;       }
      void setBeginTextPlace(PlaceText p)     { _beginTextPlace = p;          }
      PlaceText continueTextPlace() const     { return _continueTextPlace;    }
      void setContinueTextPlace(PlaceText p)  { _continueTextPlace = p;       }
      PlaceText endTextPlace() const          { return _endTextPlace;    }
      void setEndTextPlace(PlaceText p)       { _endTextPlace = p;       }

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };


}     // namespace Ms
#endif

