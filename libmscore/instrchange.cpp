//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "instrchange.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "undo.h"
#include "mscore.h"
#include "xml.h"
#include "measure.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   instrumentChangeStyle
//---------------------------------------------------------

static const ElementStyle instrumentChangeStyle {
/*      { Sid::instrumentChangeFontFace,           Pid::FONT_FACE              },
      { Sid::instrumentChangeFontSize,           Pid::FONT_SIZE              },
      { Sid::instrumentChangeFontBold,           Pid::FONT_BOLD              },
      { Sid::instrumentChangeFontItalic,         Pid::FONT_ITALIC            },
      { Sid::instrumentChangeFontUnderline,      Pid::FONT_UNDERLINE         },
      { Sid::instrumentChangeAlign,              Pid::ALIGN                  },
      { Sid::instrumentChangeOffset,             Pid::OFFSET                 },
*/
      { Sid::instrumentChangePlacement,          Pid::PLACEMENT              },
      };

//---------------------------------------------------------
//   InstrumentChange
//---------------------------------------------------------

InstrumentChange::InstrumentChange(Score* s)
   : TextBase(s, Tid::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&instrumentChangeStyle);
      _instrument = new Instrument();
      }

InstrumentChange::InstrumentChange(const Instrument& i, Score* s)
   : TextBase(s, Tid::INSTRUMENT_CHANGE, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&instrumentChangeStyle);
      _instrument = new Instrument(i);
      }

InstrumentChange::InstrumentChange(const InstrumentChange& is)
   : TextBase(is)
      {
      _instrument = new Instrument(*is._instrument);
      }

InstrumentChange::~InstrumentChange()
      {
      delete _instrument;
      }

void InstrumentChange::setInstrument(const Instrument& i)
      {
      *_instrument = i;
      //delete _instrument;
      //_instrument = new Instrument(i);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentChange::write(XmlWriter& xml) const
      {
      xml.stag(name());
      _instrument->write(xml, part());
      TextBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Instrument")
                  _instrument->read(e, part());
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      if (score()->mscVersion() <= 206) {
            // previous versions did not honor transposition of instrument change
            // except in ways that it should not have
            // notes entered before the instrument change was added would not be altered,
            // so original transposition remained in effect
            // notes added afterwards would be transposed by both intervals, resulting in tpc corruption
            // here we set the instrument change to inherit the staff transposition to emulate previous versions
            // in Note::read(), we attempt to fix the tpc corruption

            Interval v = staff() ? staff()->part()->instrument()->transpose() : 0;
            _instrument->setTranspose(v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant InstrumentChange::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::SUB_STYLE:
                  return int(Tid::INSTRUMENT_CHANGE);
            default:
                  return TextBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void InstrumentChange::layout()
      {
      layout2(Sid::instrumentChangePosAbove, Sid::instrumentChangePosBelow);
      autoplaceSegmentElement(styleP(Sid::instrumentChangeMinDistance));
      }

}

