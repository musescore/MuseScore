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
      TextLineSegment(Score* s);
      TextLineSegment(const TextLineSegment&);
      ~TextLineSegment();

      virtual TextLineSegment* clone() const { return new TextLineSegment(*this); }
      virtual Element::Type type() const     { return Element::Type::TEXTLINE_SEGMENT; }
      TextLine* textLine() const             { return (TextLine*)spanner(); }
      virtual void draw(QPainter*) const;

      virtual void layout();
      void layout1();
      virtual void setSelected(bool f);

//      Text* text() const { return _text; }

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;
      };

enum class HookType : char { HOOK_90, HOOK_45 };

//---------------------------------------------------------
//   @@ TextLine
//---------------------------------------------------------

class TextLine : public SLine {
      Q_OBJECT

      PlaceText _beginTextPlace, _continueTextPlace, _endTextPlace;

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

      virtual TextLine* clone() const           { return new TextLine(*this); }
      virtual Element::Type type() const         { return Element::Type::TEXTLINE; }
      virtual LineSegment* createLineSegment();
      virtual void layout();

      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      void writeProperties(Xml& xml) const;
      bool readProperties(XmlReader& node);

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

      void setBeginText(const QString& s, int style);
      void setContinueText(const QString& s, int style);
      void setEndText(const QString& s, int style);

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

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID id) const;
      };


}     // namespace Ms
#endif

