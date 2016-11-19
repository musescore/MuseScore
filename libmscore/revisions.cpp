//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "revisions.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

Revision::Revision()
      {
      _parent = 0;
      _dateTime = QDateTime::currentDateTime();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revision::write(XmlWriter& xml) const
      {
      xml.stag("Revision");
      xml.tag("id",   _id);
      xml.tag("date", _dateTime.toString());
      xml.tag("diff", _diff);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Revision::read(XmlReader& e)
      {
      _dateTime = QDateTime::currentDateTime();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "id")
                  _id = e.readElementText();
            else if (tag == "diff")
                  _diff = e.readElementText();
            else if (tag == "date")
                  _dateTime = QDateTime::fromString(e.readElementText());
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   Revisions
//---------------------------------------------------------

Revisions::Revisions()
      {
      _trunk = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revisions::write(XmlWriter& xml) const
      {
      for (Revision* r = _trunk; r; r = r->parent())
            write(xml, r);
      }

void Revisions::write(XmlWriter& xml, const Revision* r) const
      {
      r->write(xml);
      foreach(const Revision* rr, r->branches())
            write(xml, rr);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Revisions::add(Revision* r)
      {
      if (_trunk == 0) {
            _trunk = r;
            _trunk->setParent(0);
            return;
            }
      }

//---------------------------------------------------------
//   getRevision
//---------------------------------------------------------

QString Revisions::getRevision(QString /*id*/)
      {
      return QString();
      }

}

