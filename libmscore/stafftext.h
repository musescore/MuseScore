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

#ifndef __STAFFTEXT_H__
#define __STAFFTEXT_H__

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
//   @@ StaffText
//---------------------------------------------------------

class StaffText final : public TextBase  {
      QString _channelNames[4];
      QList<ChannelActions> _channelActions;
      SwingParameters _swingParameters;
      bool _setAeolusStops { false };
      int aeolusStops[4]   { 0, 0, 0, 0 };
      bool _swing          { false };

   protected:
      virtual void writeProperties(XmlWriter& xml) const;

   public:
      StaffText(Score* = 0);
      StaffText(SubStyleId, Score* = 0);
      virtual StaffText* clone() const                    { return new StaffText(*this);    }
      virtual ElementType type() const                    { return ElementType::STAFF_TEXT; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual bool readProperties(XmlReader&) override;
      virtual int subtype() const                         { return (int) subStyleId(); }
      virtual void layout() override;
      virtual QString subtypeName() const                 { return "??"; }
      virtual QVariant propertyDefault(Pid id) const override;

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
