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

#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "staff.h"
#include "xml.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : TextBase(s)
      {
      init(SubStyle::STAFF);
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setPlacement(Placement::ABOVE);     // default
      setSwingParameters(MScore::division / 2, 60);
      }

StaffText::StaffText(SubStyle ss, Score* s)
   : TextBase(s)
      {
      init(ss);
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setPlacement(Placement::ABOVE);     // default
      setSwingParameters(MScore::division / 2, 60);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void StaffText::writeProperties(XmlWriter& xml) const
      {
      for (ChannelActions s : _channelActions) {
            int channel = s.channel;
            for (QString name : s.midiActionNames)
                  xml.tagE(QString("MidiAction channel=\"%1\" name=\"%2\"").arg(channel).arg(name));
            }
      for (int voice = 0; voice < VOICES; ++voice) {
            if (!_channelNames[voice].isEmpty())
                  xml.tagE(QString("channelSwitch voice=\"%1\" name=\"%2\"").arg(voice).arg(_channelNames[voice]));
            }
      if (_setAeolusStops) {
            for (int i = 0; i < 4; ++i)
                  xml.tag(QString("aeolus group=\"%1\"").arg(i), aeolusStops[i]);
            }
      if (swing()) {
            QString swingUnit;
            if (swingParameters()->swingUnit == MScore::division / 2)
                  swingUnit = TDuration(TDuration::DurationType::V_EIGHTH).name();
            else if (swingParameters()->swingUnit == MScore::division / 4)
                  swingUnit = TDuration(TDuration::DurationType::V_16TH).name();
            else
                  swingUnit = TDuration(TDuration::DurationType::V_ZERO).name();
            int swingRatio = swingParameters()->swingRatio;
            xml.tagE(QString("swing unit=\"%1\" ratio= \"%2\"").arg(swingUnit).arg(swingRatio));
            }
      TextBase::writeProperties(xml);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("StaffText");
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(XmlReader& e)
      {
      for (int voice = 0; voice < VOICES; ++voice)
            _channelNames[voice].clear();
      clearAeolusStops();
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffText::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "MidiAction") {
            int channel = e.intAttribute("channel", 0);
            QString name = e.attribute("name");
            bool found = false;
            int n = _channelActions.size();
            for (int i = 0; i < n; ++i) {
                  ChannelActions* a = &_channelActions[i];
                  if (a->channel == channel) {
                        a->midiActionNames.append(name);
                        found = true;
                        break;
                        }
                  }
            if (!found) {
                  ChannelActions a;
                  a.channel = channel;
                  a.midiActionNames.append(name);
                  _channelActions.append(a);
                  }
            e.readNext();
            }
      else if (tag == "channelSwitch" || tag == "articulationChange") {
            int voice = e.intAttribute("voice", -1);
            if (voice >= 0 && voice < VOICES)
                  _channelNames[voice] = e.attribute("name");
            else if (voice == -1) {
                  // no voice applies channel to all voices for
                  // compatibility
                  for (int i = 0; i < VOICES; ++i)
                        _channelNames[i] = e.attribute("name");
                  }
            e.readNext();
            }
      else if (tag == "aeolus") {
            int group = e.intAttribute("group", -1);
            if (group >= 0 && group < 4)
                  aeolusStops[group] = e.readInt();
            else
                  e.readNext();
            _setAeolusStops = true;
            }
      else if (tag == "swing") {
            QString swingUnit = e.attribute("unit","");
            int unit = 0;
            if (swingUnit == TDuration(TDuration::DurationType::V_EIGHTH).name())
                  unit = MScore::division / 2;
            else if (swingUnit == TDuration(TDuration::DurationType::V_16TH).name())
                  unit = MScore:: division / 4;
            else if (swingUnit == TDuration(TDuration::DurationType::V_ZERO).name())
                  unit = 0;
            int ratio = e.intAttribute("ratio", 60);
            setSwing(true);
            setSwingParameters(unit, ratio);
            e.readNext();
            }
      else if (!TextBase::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffText::clearAeolusStops()
      {
      for (int i = 0; i < 4; ++i)
            aeolusStops[i] = 0;
      }

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffText::setAeolusStop(int group, int idx, bool val)
      {
      if (val)
            aeolusStops[group] |= (1 << idx);
      else
            aeolusStops[group] &= ~(1 << idx);
      }

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffText::getAeolusStop(int group, int idx) const
      {
      return aeolusStops[group] & (1 << idx);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffText::layout()
      {
      if (autoplace())
            setUserOff(QPointF());

      // TODO: add above/below offset properties
      QPointF p(offset() * (offsetType() == OffsetType::SPATIUM ? spatium() : DPI));
      if (placement() == Placement::BELOW)
            p.ry() =  - p.ry() + lineHeight();
      setPos(p);
      TextBase::layout1();
      if (!parent()) // palette & clone trick
          return;

      if (autoplace() && segment()) {
            qreal minDistance = score()->styleP(StyleIdx::dynamicsMinDistance);  // TODO
            const Shape& s1 = segment()->measure()->staffShape(staffIdx());
            Shape s2        = shape().translated(segment()->pos() + pos());

            if (placeAbove()) {
                  qreal d = s2.minVerticalDistance(s1);
                  if (d > -minDistance)
                        rUserYoffset() = -d - minDistance;
                  }
            else {
                  qreal d = s1.minVerticalDistance(s2);
                  if (d > -minDistance)
                        rUserYoffset() = d + minDistance;
                  }
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* StaffText::segment() const
      {
      if (!parent()->isSegment()) {
            qDebug("StaffText parent %s\n", parent()->name());
            return 0;
            }
      Segment* s = toSegment(parent());
      return s;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant StaffText::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::STAFF);
            case P_ID::PLACEMENT:
                  return int(Placement::ABOVE);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

