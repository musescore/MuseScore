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
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   markerStyle
//---------------------------------------------------------

static const ElementStyle markerStyle {
      { Sid::repeatLeftPlacement, Pid::PLACEMENT },
      { Sid::repeatMinDistance,   Pid::MIN_DISTANCE },
      };

//must be in sync with Marker::Type enum
const MarkerTypeItem markerTypeTable[] = {
      { Marker::Type::SEGNO    , QT_TRANSLATE_NOOP("markerType", "Segno")           },
      { Marker::Type::VARSEGNO , QT_TRANSLATE_NOOP("markerType", "Segno variation") },
      { Marker::Type::CODA     , QT_TRANSLATE_NOOP("markerType", "Coda")            },
      { Marker::Type::VARCODA  , QT_TRANSLATE_NOOP("markerType", "Varied coda")     },
      { Marker::Type::CODETTA  , QT_TRANSLATE_NOOP("markerType", "Codetta")         },
      { Marker::Type::FINE     , QT_TRANSLATE_NOOP("markerType", "Fine")            },
      { Marker::Type::TOCODA   , QT_TRANSLATE_NOOP("markerType", "To Coda")         },
      { Marker::Type::TOCODASYM, QT_TRANSLATE_NOOP("markerType", "To Coda (Symbol)")},
      { Marker::Type::USER     , QT_TRANSLATE_NOOP("markerType", "Custom")          }
      };

int markerTypeTableSize()
      {
      return sizeof(markerTypeTable)/sizeof(MarkerTypeItem) - 1; //-1 for the user defined
      }

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Marker(s, Tid::REPEAT_LEFT)
      {
      }

Marker::Marker(Score* s, Tid tid)
   : TextBase(s, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
      {
      initElementStyle(&markerStyle);
      _markerType = Type::FINE;
      setLayoutToParentWidth(true);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(Type t)
      {
      _markerType = t;
      const char* txt = 0;
      switch (t) {
            case Type::SEGNO:
                  txt = "<sym>segno</sym>";
                  setLabel("segno");
                  break;

            case Type::VARSEGNO:
                  txt = "<sym>segnoSerpent1</sym>";
                  setLabel("varsegno");
                  break;

            case Type::CODA:
                  txt = "<sym>coda</sym>";
                  setLabel("codab");
                  break;

            case Type::VARCODA:
                  txt = "<sym>codaSquare</sym>";
                  setLabel("varcoda");
                  break;

            case Type::CODETTA:
                  txt = "<sym>coda</sym><sym>coda</sym>";
                  setLabel("codetta");
                  break;

            case Type::FINE:
                  txt = "Fine";
                  initTid(Tid::REPEAT_RIGHT, true);
                  setLabel("fine");
                  break;

            case Type::TOCODA:
                  txt = "To Coda";
                  initTid(Tid::REPEAT_RIGHT, true);
                  setLabel("coda");
                  break;

            case Type::TOCODASYM:
                  txt = "To <font size=\"20\"/><sym>coda</sym>";
                  initTid(Tid::REPEAT_RIGHT, true);
                  setLabel("coda");
                  break;

            case Type::USER:
                  break;

            default:
                  qDebug("unknown marker type %d", int(t));
                  break;
            }
      if (empty() && txt)
            setXmlText(txt);
      }

//---------------------------------------------------------
//   markerTypeUserName
//---------------------------------------------------------

QString Marker::markerTypeUserName() const
      {
      return qApp->translate("markerType", markerTypeTable[static_cast<int>(_markerType)].name.toUtf8().constData());
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
      {
      setMarkerType(_markerType);
      TextBase::styleChanged();
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
//   layout
//---------------------------------------------------------

void Marker::layout()
      {
      TextBase::layout();

      // although normally laid out to parent (measure) width,
      // force to center over barline if left-aligned

      if (layoutToParentWidth() && !(align() & (Align::RIGHT | Align::HCENTER)))
            rxpos() -= width() * 0.5;

      autoplaceMeasureElement();
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
                  }
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(XmlWriter& xml) const
      {
      xml.stag(this);
      TextBase::writeProperties(xml);
      xml.tag("label", _label);
      xml.etag();
      }

//---------------------------------------------------------
//   undoSetLabel
//---------------------------------------------------------

void Marker::undoSetLabel(const QString& s)
      {
      undoChangeProperty(Pid::LABEL, s);
      }

//---------------------------------------------------------
//   undoSetMarkerType
//---------------------------------------------------------

void Marker::undoSetMarkerType(Type t)
      {
      undoChangeProperty(Pid::MARKER_TYPE, int(t));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Marker::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LABEL:
                  return label();
            case Pid::MARKER_TYPE:
                  return int(markerType());
            default:
                  break;
            }
      return TextBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Marker::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::LABEL:
                  setLabel(v.toString());
                  break;
            case Pid::MARKER_TYPE:
                  setMarkerType(Type(v.toInt()));
                  break;
            default:
                  if (!TextBase::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Marker::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LABEL:
                  return QString();
            case Pid::MARKER_TYPE:
                  return int(Type::FINE);
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  break;
            }
      return TextBase::propertyDefault(propertyId);
      }


//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Marker::nextSegmentElement()
      {
      Segment* seg;
      if (markerType() == Marker::Type::FINE) {
            seg = measure()->last();
            return seg->firstElement(staffIdx());
            }
      Measure* prevMeasure = measure()->prevMeasureMM();
      if (prevMeasure) {
            seg = prevMeasure->last();
            return seg->firstElement(staffIdx());
            }
      return Element::nextSegmentElement();
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Marker::prevSegmentElement()
      {
      //it's the same barline
      return nextSegmentElement();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Marker::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo(), markerTypeUserName());
      }

}

