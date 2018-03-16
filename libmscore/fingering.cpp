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

#include "fingering.h"
#include "score.h"
#include "staff.h"
#include "undo.h"
#include "xml.h"
#include "chord.h"
#include "part.h"
#include "measure.h"
#include "stem.h"

namespace Ms {

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
  : TextBase(s)
      {
      init(SubStyle::FINGERING);
      setFlag(ElementFlag::HAS_TAG, true);      // this is a layered element
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(name());
      TextBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fingering::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!TextBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      TextBase::layout();

      if (autoplace() && note()) {
            Chord* chord = note()->chord();
            Staff* staff = chord->staff();
            Part* part   = staff->part();
            int n        = part->nstaves();
            bool voices  = chord->measure()->hasVoices(staff->idx());
            bool below   = voices ? !chord->up() : (n > 1) && (staff->rstaff() == n-1);
            bool tight   = voices && !chord->beam();

            qreal x = 0.0;
            qreal y = 0.0;
            qreal headWidth = note()->headWidth();
            qreal headHeight = note()->headHeight();
            qreal fh = headHeight;        // TODO: fingering number height

            if (chord->notes().size() == 1) {
                  x = headWidth * .5;
                  if (below) {
                        // place fingering below note
                        y = fh + spatium() * .4;
                        if (tight) {
                              y += 0.5 * spatium();
                              if (chord->stem())
                                    x += 0.5 * spatium();
                              }
                        else if (chord->stem() && !chord->up()) {
                              // on stem side
                              y += chord->stem()->height();
                              x -= spatium() * .4;
                              }
                        }
                  else {
                        // place fingering above note
                        y = -headHeight - spatium() * .4;
                        if (tight) {
                              y -= 0.5 * spatium();
                              if (chord->stem())
                                    x -= 0.5 * spatium();
                              }
                        else if (chord->stem() && chord->up()) {
                              // on stem side
                              y -= chord->stem()->height();
                              x += spatium() * .4;
                              }
                        }
                  }
            else {
                  x -= spatium();
                  }
            setUserOff(QPointF(x, y));
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Fingering::draw(QPainter* painter) const
      {
      TextBase::draw(painter);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fingering::accessibleInfo() const
      {
      QString rez = Element::accessibleInfo();
      if (subStyle() == SubStyle::STRING_NUMBER) {
            rez += " " + QObject::tr("String number");
            }
      return QString("%1: %2").arg(rez).arg(plainText());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Fingering::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return TextBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fingering::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            default:
                  return TextBase::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::FINGERING);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags& Fingering::propertyFlags(P_ID id)
      {
      return TextBase::propertyFlags(id);
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fingering::resetProperty(P_ID id)
      {
      switch (id) {
            default:
                  return TextBase::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Fingering::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            default:
                  return TextBase::getPropertyStyle(id);
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Fingering::styleChanged()
      {
      TextBase::styleChanged();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Fingering::reset()
      {
      TextBase::reset();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Fingering::subtypeName() const
      {
      return subStyleName(subStyle());
      }

}

