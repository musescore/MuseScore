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
      // init subStyle Fingering
      _subStyleId = SubStyleId::FINGERING;
      _propertyFlagsList = new PropertyFlags[subStyle(_subStyleId).size()];
      for (const StyledProperty* spp = styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp)
            resetProperty(spp->propertyIdx);

      setFlag(ElementFlag::HAS_TAG, true);      // this is a layered element
      }

Fingering::Fingering(SubStyleId ssid, Score* s)
   : TextBase(s)
      {
      setSubStyleId(ssid);
      for (const StyledProperty* spp = styledProperties();spp->styleIdx != StyleIdx::NOSTYLE; ++spp)
            resetProperty(spp->propertyIdx);
      setFlag(ElementFlag::HAS_TAG, true);      // this is a layered element
      }

Fingering::~Fingering()
      {
      }

//---------------------------------------------------------
//   styledProperties
//---------------------------------------------------------

const StyledProperty* Fingering::styledProperties() const
      {
      return subStyle(_subStyleId).data();
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
      if (_subStyleId == SubStyleId::STRING_NUMBER) {
            rez += " " + QObject::tr("String number");
            }
      return QString("%1: %2").arg(rez).arg(plainText());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Fingering::getProperty(P_ID propertyId) const
      {
      return TextBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fingering::setProperty(P_ID propertyId, const QVariant& v)
      {
      return TextBase::setProperty(propertyId, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyleId::FINGERING);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Fingering::subtypeName() const
      {
      return subStyleName(_subStyleId);
      }

}

