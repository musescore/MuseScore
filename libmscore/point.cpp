//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "point.h"

#include "mscore.h"
#include "xml.h"

namespace Ms {

static constexpr PointInfo absDefaults = PointInfo::absolute();
static constexpr PointInfo relDefaults = PointInfo::relative();

//---------------------------------------------------------
//   PointInfo::track
//---------------------------------------------------------

int PointInfo::track() const
      {
      if ((_staff == absDefaults._staff) || (_voice == absDefaults._voice))
            return INT_MIN;
      return VOICES * _staff + _voice;
      }

//---------------------------------------------------------
//   PointInfo::setTrack
//---------------------------------------------------------

void PointInfo::setTrack(int track)
      {
      _staff = track / VOICES;
      _voice = track % VOICES;
      }

//---------------------------------------------------------
//   PointInfo::write
//---------------------------------------------------------

void PointInfo::write(XmlWriter& xml) const
      {
      if (isRelative()) {
            xml.stag("move");
            xml.tag("staves", _staff, relDefaults._staff);
            xml.tag("voices", _voice, relDefaults._voice);
            xml.tag("measures", _measure, relDefaults._measure);
            xml.tag("fractions", _fpos.reduced(), relDefaults._fpos);
            xml.tag("grace", _graceIndex, relDefaults._graceIndex);
            xml.tag("notes", _note, relDefaults._note);
            xml.etag();
            }
      else {
            xml.stag("move_abs");
            xml.tag("staff", _staff, absDefaults._staff);
            xml.tag("voice", _voice, absDefaults._voice);
            xml.tag("measure", _measure, absDefaults._measure);
            xml.tag("fraction", _fpos.reduced(), absDefaults._fpos);
            xml.tag("grace", _graceIndex, absDefaults._graceIndex);
            xml.tag("note", _note, absDefaults._note);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   PointInfo::read
//---------------------------------------------------------

void PointInfo::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if ((tag == "staves") || (tag == "staff"))
                  _staff = e.readInt();
            else if ((tag == "voices") || (tag == "voice"))
                  _voice = e.readInt();
            else if ((tag == "measures") || (tag == "measure"))
                  _measure = e.readInt();
            else if ((tag == "fractions") || (tag == "fraction"))
                  _fpos = e.readFraction();
            else if (tag == "grace")
                  _graceIndex = e.readInt();
            else if ((tag == "notes") || (tag == "note"))
                  _note = e.readInt();
            }
      }

//---------------------------------------------------------
//   PointInfo::toAbsolute
//---------------------------------------------------------

void PointInfo::toAbsolute(const PointInfo& ref)
      {
      if (isAbsolute())
            return;
      _staff += ref._staff;
      _voice += ref._voice;
      _measure += ref._measure;
      _fpos += ref._fpos;
      _note += ref._note;
      _rel = false;
      }

//---------------------------------------------------------
//   PointInfo::toRelative
//---------------------------------------------------------

void PointInfo::toRelative(const PointInfo& ref)
      {
      if (isRelative())
            return;
      _staff -= ref._staff;
      _voice -= ref._voice;
      _measure -= ref._measure;
      _fpos -= ref._fpos;
      _note -= ref._note;
      _rel = true;
      }

//---------------------------------------------------------
//   PointInfo::operator==
//---------------------------------------------------------

bool PointInfo::operator==(const PointInfo& pi2) const {
      const PointInfo& pi1 = *this;
      return ((pi1._fpos == pi2._fpos)
             && (pi1._measure == pi2._measure)
             && (pi1._voice == pi2._voice)
             && (pi1._staff == pi2._staff)
             && (pi1._graceIndex == pi2._graceIndex)
             && (pi1._note == pi2._note)
             );
      }
}

