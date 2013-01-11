//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: stafftext.h 5500 2012-03-28 16:28:26Z wschweer $
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

class StaffText : public Text  {
      Q_OBJECT

      QString _channelNames[4];
      QList<ChannelActions> _channelActions;
      bool _setAeolusStops;
      int aeolusStops[4];

   public:
      StaffText(Score* = 0);
      virtual StaffText* clone() const { return new StaffText(*this); }
      virtual ElementType type() const { return STAFF_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

      QString channelName(int voice) const                { return _channelNames[voice]; }
      void setChannelName(int v, const QString& s)        { _channelNames[v] = s;        }
      const QList<ChannelActions>* channelActions() const { return &_channelActions;    }
      QList<ChannelActions>* channelActions()             { return &_channelActions;    }
      void clearAeolusStops();
      void setAeolusStop(int group, int idx, bool val);
      bool getAeolusStop(int group, int idx) const;
      void setSetAeolusStops(bool val) { _setAeolusStops = val; }
      bool setAeolusStops() const      { return _setAeolusStops; }
      };

#endif
