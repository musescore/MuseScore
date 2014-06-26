//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "marker.h"
#include "score.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Text(s)
      {
      _markerType = Type::FINE;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::REPEAT);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(Type t)
      {
      _markerType = t;
      switch (t) {
            case Type::SEGNO:
                  setText("<sym>segno</sym>");
                  setLabel("segno");
                  break;

            case Type::VARSEGNO:
                  setText("<sym>segnoSerpent1</sym>");
                  setLabel("varsegno");
                  break;

            case Type::CODA:
                  setText("<sym>coda</sym>");
                  setLabel("codab");
                  break;

            case Type::VARCODA:
                  setText("<sym>codaSquare</sym>");
                  setLabel("varcoda");
                  break;

            case Type::CODETTA:
                  setText("<sym>coda</sym><sym>coda</sym>");
                  setLabel("codetta");
                  break;

            case Type::FINE:
                  setText("Fine");
                  setLabel("fine");
                  break;

            case Type::TOCODA:
                  setText("To Coda");
                  setLabel("coda");
                  break;

            case Type::USER:
                  break;

            default:
                  qDebug("unknown marker type %hhd", t);
                  break;
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
      {
      setMarkerType(_markerType);
      }

//---------------------------------------------------------
//   adjustReadPos
//---------------------------------------------------------

void Marker::adjustReadPos()
      {
      if (!readPos().isNull()) {
            QPointF uo;
            if (score()->mscVersion() <= 114) {
                  // rebase from Measure to Segment
                  uo = userOff();
                  uo.rx() -= segment()->pos().x();
                  // 1.2 is always HCENTER aligned
                  if ((textStyle().align() & AlignmentFlags::HMASK) == 0)    // AlignmentFlags::LEFT
                        uo.rx() -= bbox().width() * .5;
                  }
            else
                  uo = readPos() - ipos();
            setUserOff(uo);
            setReadPos(QPointF());
            }
      }

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

Marker::Type Marker::markerType(const QString& s) const
      {
      if (s == "segno")
            return Type::SEGNO;
      else if (s == "varsegno")
            return Type::VARSEGNO;
      else if (s == "codab")
            return Type::CODA;
      else if (s == "varcoda")
            return Type::VARCODA;
      else if (s == "codetta")
            return Type::CODETTA;
      else if (s == "fine")
            return Type::FINE;
      else if (s == "coda")
            return Type::TOCODA;
      else
            return Type::USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(XmlReader& e)
      {
      Type mt = Type::SEGNO;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "label") {
                  QString s(e.readElementText());
                  setLabel(s);
                  mt = markerType(s);
                  qDebug("Marker::read type %d <%s>", int(mt), qPrintable(s));
                  }
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("label", _label);
      xml.etag();
      }

//---------------------------------------------------------
//   undoSetLabel
//---------------------------------------------------------

void Marker::undoSetLabel(const QString& s)
      {
      score()->undoChangeProperty(this, P_ID::LABEL, s);
      }

//---------------------------------------------------------
//   undoSetMarkerType
//---------------------------------------------------------

void Marker::undoSetMarkerType(Type t)
      {
      score()->undoChangeProperty(this, P_ID::MARKER_TYPE, int(t));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Marker::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LABEL:
                  return label();
            case P_ID::MARKER_TYPE:
                  return int(markerType());
            default:
                  break;
            }
      return Text::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Marker::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::LABEL:
                  setLabel(v.toString());
                  break;
            case P_ID::MARKER_TYPE:
                  setMarkerType(Type(v.toInt()));
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Marker::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LABEL:
                  return QString();

            case P_ID::MARKER_TYPE:
                  return int(Type::FINE);

            default:
                  break;
            }
      return Text::propertyDefault(propertyId);
      }



}

