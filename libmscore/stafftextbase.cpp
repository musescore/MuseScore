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
#include "stafftextbase.h"
#include "system.h"
#include "staff.h"
#include "xml.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

StaffTextBase::StaffTextBase(Score* s, Tid tid, ElementFlags flags)
   : TextBase(s, tid, flags)
      {
      setSwingParameters(MScore::division / 2, 60);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTextBase::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);

      for (const ChannelActions &s : _channelActions) {
            int channel = s.channel;
            for (const QString &name : qAsConst(s.midiActionNames))
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
            xml.tagE(QString("swing unit=\"%1\" ratio=\"%2\"").arg(swingUnit).arg(swingRatio));
            }
      if (capo() != 0)
            xml.tagE(QString("capo fretId=\"%1\"").arg(capo()));
      TextBase::writeProperties(xml);

      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTextBase::read(XmlReader& e)
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

bool StaffTextBase::readProperties(XmlReader& e)
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
      else if (tag == "capo") {
            int fretId = e.intAttribute("fretId", 0);
            setCapo(fretId);
            e.readNext();
            }
      else if (!TextBase::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffTextBase::clearAeolusStops()
      {
      for (int i = 0; i < 4; ++i)
            aeolusStops[i] = 0;
      }

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffTextBase::setAeolusStop(int group, int idx, bool val)
      {
      if (val)
            aeolusStops[group] |= (1 << idx);
      else
            aeolusStops[group] &= ~(1 << idx);
      }

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffTextBase::getAeolusStop(int group, int idx) const
      {
      return aeolusStops[group] & (1 << idx);
      }

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* StaffTextBase::segment() const
      {
      if (!parent()->isSegment()) {
            qDebug("parent %s", parent()->name());
            return 0;
            }
      Segment* s = toSegment(parent());
      return s;
      }

}

