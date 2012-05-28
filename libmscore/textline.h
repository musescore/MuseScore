//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: textline.h 5500 2012-03-28 16:28:26Z wschweer $
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
//   TextLineSegment
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
      virtual void setSelected(bool f);

      Text* text() const { return _text; }
      void clearText();

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      };

enum HookType { HOOK_90, HOOK_45 };

//---------------------------------------------------------
//   TextLine
//    brackets
//---------------------------------------------------------

class TextLine : public SLine {
      Q_OBJECT

      Spatium _lineWidth;
      QColor _lineColor;
      Qt::PenStyle _lineStyle;
      Placement _beginTextPlace, _continueTextPlace;

      bool _beginHook, _endHook;
      HookType _beginHookType, _endHookType;
      Spatium _beginHookHeight, _endHookHeight;

      int _beginSymbol, _continueSymbol, _endSymbol;  // -1: no symbol
      QPointF _beginSymbolOffset, _continueSymbolOffset, _endSymbolOffset;

      int _mxmlOff2;
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
      virtual void read(const QDomElement&);
      void writeProperties(Xml& xml, const TextLine* proto = 0) const;
      bool readProperties(const QDomElement& node);

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
      Placement beginTextPlace() const        { return _beginTextPlace;       }
      void setBeginTextPlace(Placement p)     { _beginTextPlace = p;          }
      Placement continueTextPlace() const     { return _continueTextPlace;    }
      void setContinueTextPlace(Placement p)  { _continueTextPlace = p;       }

      void setBeginSymbol(int v)              { _beginSymbol = v;             }
      void setContinueSymbol(int v)           { _continueSymbol = v;          }
      void setEndSymbol(int v)                { _endSymbol = v;               }

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
      int beginSymbol() const                 { return _beginSymbol;          }
      int continueSymbol() const              { return _continueSymbol;       }
      int endSymbol() const                   { return _endSymbol;            }
      QPointF beginSymbolOffset() const       { return _beginSymbolOffset;    }
      QPointF continueSymbolOffset() const    { return _continueSymbolOffset; }
      QPointF endSymbolOffset() const         { return _endSymbolOffset;      }
      void setBeginSymbolOffset(QPointF v)    { _beginSymbolOffset = v;       }
      void setContinueSymbolOffset(QPointF v) { _continueSymbolOffset = v;    }
      void setEndSymbolOffset(QPointF v)      { _endSymbolOffset = v;         }
      void setMxmlOff2(int v)                 { _mxmlOff2 = v;                }
      int mxmlOff2() const                    { return _mxmlOff2;             }
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      };

#endif

