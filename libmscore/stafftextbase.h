//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFFTEXTBASE_H__
#define __STAFFTEXTBASE_H__

#include "text.h"
#include "part.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   ChannelActions
//---------------------------------------------------------

struct ChannelActions {
      int channel;
      QStringList midiActionNames;
      };

//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

class StaffTextBase : public TextBase  {
      QString _channelNames[4];
      QList<ChannelActions> _channelActions;
      SwingParameters _swingParameters;
      bool _setAeolusStops { false };
      int aeolusStops[4]   { 0, 0, 0, 0 };
      bool _swing          { false };

   public:
      StaffTextBase(Score*, Tid tid, ElementFlags = ElementFlag::NOTHING);

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual bool readProperties(XmlReader&) override;
      virtual int subtype() const                         { return 0; }       // TODO::ws
      virtual QString subtypeName() const                 { return "??"; }

      Segment* segment() const;
      QString channelName(int voice) const                { return _channelNames[voice]; }
      void setChannelName(int v, const QString& s)        { _channelNames[v] = s;        }
      void setSwingParameters(int unit, int ratio)        {  _swingParameters.swingUnit = unit; _swingParameters.swingRatio = ratio; }
      const QList<ChannelActions>* channelActions() const { return &_channelActions;    }
      QList<ChannelActions>* channelActions()             { return &_channelActions;    }
      const SwingParameters* swingParameters() const      { return &_swingParameters;   }
      void clearAeolusStops();
      void setAeolusStop(int group, int idx, bool val);
      bool getAeolusStop(int group, int idx) const;
      void setSetAeolusStops(bool val)                    { _setAeolusStops = val; }
      void setSwing(bool checked)                         { _swing = checked; }
      bool setAeolusStops() const                         { return _setAeolusStops; }
      bool swing() const                                  { return _swing; }
      };

}     // namespace Ms
#endif
