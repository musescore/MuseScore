//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "iname.h"
#include "staff.h"
#include "part.h"
#include "undo.h"

namespace Ms {

static const ElementStyle longInstrumentStyle {
      { Sid::longInstrumentFontFace,             Pid::FONT_FACE              },
      { Sid::longInstrumentFontSize,             Pid::FONT_SIZE              },
      { Sid::longInstrumentFontBold,             Pid::FONT_BOLD              },
      { Sid::longInstrumentFontItalic,           Pid::FONT_ITALIC            },
      { Sid::longInstrumentFontUnderline,        Pid::FONT_UNDERLINE         },
      { Sid::longInstrumentAlign,                Pid::ALIGN                  },
      };

static const ElementStyle shortInstrumentStyle {
      { Sid::shortInstrumentFontFace,            Pid::FONT_FACE              },
      { Sid::shortInstrumentFontSize,            Pid::FONT_SIZE              },
      { Sid::shortInstrumentFontBold,            Pid::FONT_BOLD              },
      { Sid::shortInstrumentFontItalic,          Pid::FONT_ITALIC            },
      { Sid::shortInstrumentFontUnderline,       Pid::FONT_UNDERLINE         },
      { Sid::shortInstrumentAlign,               Pid::ALIGN                  },
      };

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : TextBase(s, Tid::INSTRUMENT_LONG, ElementFlag::NOTHING | ElementFlag::NOT_SELECTABLE)
      {
      setInstrumentNameType(InstrumentNameType::SHORT);
      }

//---------------------------------------------------------
//   instrumentNameTypeName
//---------------------------------------------------------

QString InstrumentName::instrumentNameTypeName() const
      {
      return instrumentNameType() == InstrumentNameType::SHORT ? "short" : "long";
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(const QString& s)
      {
      if (s == "short")
            setInstrumentNameType(InstrumentNameType::SHORT);
      else if (s == "long")
            setInstrumentNameType(InstrumentNameType::LONG);
      else
            qDebug("InstrumentName::setSubtype: unknown <%s>", qPrintable(s));
      }

//---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
      {
      _instrumentNameType = st;
      setTid(st == InstrumentNameType::SHORT ? Tid::INSTRUMENT_SHORT : Tid::INSTRUMENT_LONG);
      initElementStyle(st == InstrumentNameType::SHORT ? &shortInstrumentStyle : &longInstrumentStyle);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant InstrumentName::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::INAME_LAYOUT_POSITION:
                  return _layoutPos;
            default:
                  return TextBase::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool InstrumentName::setProperty(Pid id, const QVariant& v)
      {
      bool rv = true;
      switch (id) {
            case Pid::INAME_LAYOUT_POSITION:
                  _layoutPos = v.toInt();
                  break;
            default:
                  rv = TextBase::setProperty(id, v);
                  break;
            }
#if 0
      Sid sidx = getPropertyStyle(id);
      if (sidx != Sid::NOSTYLE) {
            score()->undoChangeStyleVal(sidx, getProperty(id));
            }
      score()->setLayoutAll();
#endif
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentName::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::INAME_LAYOUT_POSITION:
                  return 0;
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

