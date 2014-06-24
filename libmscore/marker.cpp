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
      _markerType = MarkerType::FINE;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::REPEAT);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(MarkerType t)
      {
      _markerType = t;
      switch (t) {
            case MarkerType::SEGNO:
                  setText("<sym>segno</sym>");
                  setLabel("segno");
                  break;

            case MarkerType::VARSEGNO:
                  setText("<sym>segnoSerpent1</sym>");
                  setLabel("varsegno");
                  break;

            case MarkerType::CODA:
                  setText("<sym>coda</sym>");
                  setLabel("codab");
                  break;

            case MarkerType::VARCODA:
                  setText("<sym>codaSquare</sym>");
                  setLabel("varcoda");
                  break;

            case MarkerType::CODETTA:
                  setText("<sym>coda</sym><sym>coda</sym>");
                  setLabel("codetta");
                  break;

            case MarkerType::FINE:
                  setText("Fine");
                  setLabel("fine");
                  break;

            case MarkerType::TOCODA:
                  setText("To Coda");
                  setLabel("coda");
                  break;

            case MarkerType::USER:
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

MarkerType Marker::markerType(const QString& s) const
      {
      if (s == "segno")
            return MarkerType::SEGNO;
      else if (s == "varsegno")
            return MarkerType::VARSEGNO;
      else if (s == "codab")
            return MarkerType::CODA;
      else if (s == "varcoda")
            return MarkerType::VARCODA;
      else if (s == "codetta")
            return MarkerType::CODETTA;
      else if (s == "fine")
            return MarkerType::FINE;
      else if (s == "coda")
            return MarkerType::TOCODA;
      else
            return MarkerType::USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(XmlReader& e)
      {
      MarkerType mt = MarkerType::SEGNO;

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

void Marker::undoSetMarkerType(MarkerType t)
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
                  setMarkerType(MarkerType(v.toInt()));
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
                  return int(MarkerType::FINE);

            default:
                  break;
            }
      return Text::propertyDefault(propertyId);
      }



}

