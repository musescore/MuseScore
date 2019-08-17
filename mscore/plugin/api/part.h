//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_PART_H__
#define __PLUGIN_API_PART_H__

#include "scoreelement.h"
#include "libmscore/part.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part : public Ms::PluginAPI::ScoreElement {
      Q_OBJECT
      Q_PROPERTY(int                            startTrack           READ startTrack)
      Q_PROPERTY(int                            endTrack             READ endTrack)
      /// The string identifier for the current instrument. \since MuseScore 3.2
      Q_PROPERTY(QString                        instrumentId         READ instrumentId)
      /// The number of Chord Symbols. \since MuseScore 3.2.1
      Q_PROPERTY(int                            harmonyCount         READ harmonyCount)
      /// Whether it is a percussion staff. \since MuseScore 3.2.1
      Q_PROPERTY(bool                           hasDrumStaff         READ hasDrumStaff)
      /// Whether it is a 'normal' staff with notes. \since MuseScore 3.2.1
      Q_PROPERTY(bool                           hasPitchedStaff      READ hasPitchedStaff)
      /// Whether it is a tablature staff. \since MuseScore 3.2.1
      Q_PROPERTY(bool                           hasTabStaff          READ hasTabStaff)
      /// The number of lyrics syllables. \since MuseScore 3.2.1
      Q_PROPERTY(int                            lyricCount           READ lyricCount)
      /// One of 16 music channels that can be assigned an instrument. \since MuseScore 3.2.1
      Q_PROPERTY(int                            midiChannel          READ midiChannel)
      /// One of the 128 different instruments in General MIDI. \since MuseScore 3.2.1
      Q_PROPERTY(int                            midiProgram          READ midiProgram)
      /// The long name for the current instrument.
      /// Note that this property was writeable in MuseScore v2.x
      /// \since MuseScore 3.2.1
      Q_PROPERTY(QString                        longName             READ longName)
      /// The short name for the current instrument.
      /// Note that this property was writeable in MuseScore v2.x
      /// \since MuseScore 3.2.1
      Q_PROPERTY(QString                        shortName            READ shortName)
      /// The name of the current part of music.
      /// It is shown in Mixer.
      ///
      /// Note that this property was writeable in MuseScore v2.x
      /// \since MuseScore 3.2.1
      Q_PROPERTY(QString                        partName             READ partName)
      /// Whether part is shown or hidden.
      /// Note that this property was writeable in MuseScore v2.x
      /// \since MuseScore 3.2.1
      Q_PROPERTY(bool                           show                 READ show)

   public:
      /// \cond MS_INTERNAL
      Part(Ms::Part* p = nullptr, Ownership o = Ownership::SCORE)
         : ScoreElement(p, o) {}

      Ms::Part* part() { return toPart(e); }
      const Ms::Part* part() const { return toPart(e); }

      int startTrack() const { return part()->startTrack(); }
      int endTrack()   const { return part()->endTrack(); }
      QString instrumentId() const { return part()->instrument()->instrumentId(); }
      int harmonyCount() const { return part()->harmonyCount(); }
      bool hasPitchedStaff() const { return part()->hasPitchedStaff(); }
      bool hasTabStaff() const { return part()->hasTabStaff(); }
      bool hasDrumStaff() const { return part()->hasDrumStaff(); }
      int lyricCount() const { return part()->lyricCount(); }
      int midiChannel() const { return part()->midiChannel(); }
      int midiProgram() const { return part()->midiProgram(); }
      QString longName() const { return part()->longName(); }
      QString shortName() const { return part()->shortName(); }
      QString partName() const { return part()->partName(); }
      bool show() const { return part()->show(); }
      /// \endcond
      };
} // namespace PluginAPI
} // namespace Ms
#endif
