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

class TextLine;
class Element;
class Text;

//---------------------------------------------------------
//   @@ TextLineSegment
//---------------------------------------------------------

class TextLineSegment : public LineSegment {
      Q_OBJECT

      Text* _text;

   public:
      TextLineSegment(Score* s);
      TextLineSegment(const TextLineSegment&);
      virtual TextLineSegment* clone() const { return new TextLineSegment(*this); }
      virtual ElementType type() const       { return TEXTLINE_SEGMENT; }
      TextLine* textLine() const             { return (TextLine*)spanner(); }
      virtual void draw(QPainter*) const;

      virtual void layout();
      void layout1();
      virtual void setSelected(bool f);

      Text* text() const { return _text; }
      void clearText();

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      };

enum HookType { HOOK_90, HOOK_45 };

//---------------------------------------------------------
//   @@ TextLine
//---------------------------------------------------------

class TextLine : public SLine {
      Q_OBJECT

      Spatium _lineWidth;
      QColor _lineColor;
      Qt::PenStyle _lineStyle;
      PlaceText _beginTextPlace, _continueTextPlace;

      bool _beginHook, _endHook;
      HookType _beginHookType, _endHookType;
      Spatium _beginHookHeight, _endHookHeight;

      SymId _beginSymbol, _continueSymbol, _endSymbol;
      QPointF _beginSymbolOffset, _continueSymbolOffset, _endSymbolOffset;

      qreal _sp;       // cached value from last spatiumChanged() call

   protected:
      Text* _beginText;
      Text* _continueText;
      friend class TextLineSegment;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      ~TextLine() {}

      virtual TextLine* clone() const           { return new TextLine(*this); }
      virtual ElementType type() const          { return TEXTLINE; }
      virtual LineSegment* createLineSegment();
      virtual void layout();

      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      void writeProperties(Xml& xml, const TextLine* proto = 0) const;
      bool readProperties(XmlReader& node);

      bool beginHook() const                  { return _beginHook;            }
      bool endHook() const                    { return _endHook;              }
      void setBeginHook(bool v)               { _beginHook = v;               }
      void setEndHook(bool v)                 { _endHook = v;                 }
      HookType beginHookType() const          { return _beginHookType;        }
      HookType endHookType() const            { return _endHookType;          }
      void setBeginHookType(HookType val)     { _beginHookType = val;         }
      void setEndHookType(HookType val)       { _endHookType = val;           }

      void setBeginText(const QString& s, const TextStyle& textStyle);
      void setContinueText(const QString& s, const TextStyle& textStyle);
      Text* beginText() const                { return _beginText;            }
      void setBeginText(Text* v);
      Text* continueText() const             { return _continueText;         }
      void setContinueText(Text* v);
      PlaceText beginTextPlace() const        { return _beginTextPlace;       }
      void setBeginTextPlace(PlaceText p)     { _beginTextPlace = p;          }
      PlaceText continueTextPlace() const     { return _continueTextPlace;    }
      void setContinueTextPlace(PlaceText p)  { _continueTextPlace = p;       }

      void setBeginSymbol(SymId v)            { _beginSymbol = v;             }
      void setContinueSymbol(SymId v)         { _continueSymbol = v;          }
      void setEndSymbol(SymId v)              { _endSymbol = v;               }

      void setBeginHookHeight(Spatium v)      { _beginHookHeight = v;         }
      void setEndHookHeight(Spatium v)        { _endHookHeight = v;           }
      Spatium beginHookHeight() const         { return _beginHookHeight;      }
      Spatium endHookHeight() const           { return _endHookHeight;        }

      Spatium lineWidth() const               { return _lineWidth;            }
      QColor lineColor() const                { return _lineColor;            }
      Qt::PenStyle lineStyle() const          { return _lineStyle;            }
      void setLineWidth(const Spatium& v)     { _lineWidth = v;               }
      void setLineColor(const QColor& v)      { _lineColor = v;               }
      void setLineStyle(Qt::PenStyle v)       { _lineStyle = v;               }
      SymId beginSymbol() const               { return _beginSymbol;          }
      SymId continueSymbol() const            { return _continueSymbol;       }
      SymId endSymbol() const                 { return _endSymbol;            }
      QPointF beginSymbolOffset() const       { return _beginSymbolOffset;    }
      QPointF continueSymbolOffset() const    { return _continueSymbolOffset; }
      QPointF endSymbolOffset() const         { return _endSymbolOffset;      }
      void setBeginSymbolOffset(QPointF v)    { _beginSymbolOffset = v;       }
      void setContinueSymbolOffset(QPointF v) { _continueSymbolOffset = v;    }
      void setEndSymbolOffset(QPointF v)      { _endSymbolOffset = v;         }
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      };

#endif

