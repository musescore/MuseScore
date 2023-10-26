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
#include "measure.h"
#include "staff.h"
#include "system.h"
#include "part.h"
#include "undo.h"

namespace Ms {

//---------------------------------------------------------
//   longInstrumentStyle
//---------------------------------------------------------

static const ElementStyle longInstrumentStyle {
      };

//---------------------------------------------------------
//   shortInstrumentStyle
//---------------------------------------------------------

static const ElementStyle shortInstrumentStyle {
      };

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : TextBase(s, Tid::INSTRUMENT_LONG, ElementFlag::NOTHING)
      {
      setFlag(ElementFlag::MOVABLE, false);
      setInstrumentNameType(InstrumentNameType::LONG);
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

qreal InstrumentName::spatium() const
      {
      if (systemFlag() || (parent() && parent()->systemFlag()))
            return Element::spatium();

      // Get spatium for instrument names from largest staff of part,
      // instead of staff it is attached to
      Part* p = part();
      if (!part())
            return Element::spatium();
      qreal largestSpatium = 0;
      for (Staff* s: *p->staves()) {
            double sp = s->spatium(tick());
            if (sp > largestSpatium)
                largestSpatium = sp;
            }
      return largestSpatium;
      }

 //---------------------------------------------------------
//   setInstrumentNameType
//---------------------------------------------------------

void InstrumentName::setInstrumentNameType(InstrumentNameType st)
      {
      _instrumentNameType = st;
      if (st == InstrumentNameType::SHORT) {
            setTid(Tid::INSTRUMENT_SHORT);
            initElementStyle(&shortInstrumentStyle);
            }
      else {
            setTid(Tid::INSTRUMENT_LONG);
            initElementStyle(&longInstrumentStyle);
            }
      }

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction InstrumentName::playTick() const
      {
      // Instrument names always have a tick value of zero, so play from the start of the first measure in the system that the instrument name belongs to.
      const auto sys = system();
      if (sys) {
            const auto firstMeasure = sys->firstMeasure();
            if (firstMeasure)
                  return firstMeasure->tick();
            }

      return tick();
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
            case Pid::VISIBLE:
            case Pid::COLOR:
                  // not supported
                  break;
            default:
                  rv = TextBase::setProperty(id, v);
                  break;
            }
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

