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

#ifndef __PART_H__
#define __PART_H__

#include "mscore.h"
#include "instrument.h"
#include "text.h"

namespace Ms {

class XmlWriter;
class Staff;
class Score;
class InstrumentTemplate;

//---------------------------------------------------------
//   PreferSharpFlat
//---------------------------------------------------------

enum class PreferSharpFlat : char {
      DEFAULT, SHARPS, FLATS
      };

//---------------------------------------------------------
//   @@ Part
//   @P endTrack        int         (read only)
//   @P harmonyCount    int         (read only)
//   @P hasDrumStaff    bool        (read only)
//   @P hasPitchedStaff bool        (read only)
//   @P hasTabStaff     bool        (read only)
//   @P instrumentId    string      (read only)
//   @P longName        string
//   @P lyricCount      int         (read only)
//   @P midiChannel     int         (read only)
//   @P midiProgram     int         (read only)
//   @P mute            bool
//   @P partName        string      name of the part, used in the mixer
//   @P shortName       string
//   @P show            bool        check/set whether or not a part is shown
//   @P startTrack      int         (read only)
//   @P volume          int
//---------------------------------------------------------

class Part final : public ScoreElement {
      QString _partName;            ///< used in tracklist (mixer)
      InstrumentList _instruments;
      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true
      bool _soloist;                ///< used in score ordering

      static const int DEFAULT_COLOR = 0x3399ff;
      int _color;                   ///User specified color for helping to label parts

      PreferSharpFlat _preferSharpFlat;

   public:
      Part(Score* = 0);
      void initFromInstrTemplate(const InstrumentTemplate*);

      ElementType type() const override { return ElementType::PART; }

      void read(XmlReader&);
      bool readProperties(XmlReader&);
      void write(XmlWriter& xml) const;

      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }

      int startTrack() const;
      int endTrack() const;

      QString longName(const Fraction& tick = { -1, 1 } ) const;
      QString shortName(const Fraction& tick = { -1, 1 } ) const;
      QString instrumentName(const Fraction& tick = { -1, 1 } ) const;
      QString instrumentId(const Fraction& tick = { -1, 1 } ) const;

      const QList<StaffName>& longNames(const  Fraction& tick = { -1, 1 } ) const { return instrument(tick)->longNames();  }
      const QList<StaffName>& shortNames(const Fraction& tick = { -1, 1 } ) const { return instrument(tick)->shortNames(); }

      const QColor namesColor(const  Fraction& tick = { -1, 1 }) const { return instrument(tick)->getNameColor(); }
      void setLongNames(QList<StaffName>& s,  const Fraction& tick = { -1, 1 } );
      void setShortNames(QList<StaffName>& s, const Fraction& tick = { -1, 1 } );

      void setLongName(const QString& s);
      void setShortName(const QString& s);
      void setLongNameAll(const QString& s);  // For all instruments in _instruments
      void setShortNameAll(const QString& s); // For all instruments in _instruments

      void setPlainLongName(const QString& s);
      void setPlainShortName(const QString& s);
      void setPlainLongNameAll(const QString& s);
      void setPlainShortNameAll(const QString& s);

      void setStaves(int);

      int midiProgram() const;
      void setMidiProgram(int, int bank = 0);

      int midiChannel() const;
      int midiPort() const;
      void setMidiChannel(int ch, int port = -1, const Fraction& tick = {-1,1});  // tick != -1 for InstrumentChange

      void insertStaff(Staff*, int idx);
      void removeStaff(Staff*);
      bool show() const                        { return _show;     }
      void setShow(bool val)                   { _show = val;      }
      bool soloist() const                     { return _soloist;  }
      void setSoloist(bool val)                { _soloist = val;   }

      Instrument* instrument(Fraction = { -1, 1 } );
      const Instrument* instrument(Fraction = { -1, 1 }) const;
      void setInstrument(Instrument*, Fraction = { -1, 1} );       // transfer ownership
      void setInstrument(const Instrument&&, Fraction = { -1, 1 });
      void setInstrument(const Instrument&, Fraction = { -1, 1 });
      void removeInstrument(const Fraction&);
      const InstrumentList* instruments() const;

      void insertTime(const Fraction& tick, const Fraction& len);

      QString partName() const                 { return _partName; }
      void setPartName(const QString& s)       { _partName = s; }
      int color() const { return _color; }
      void setColor(int value) { _color = value; }

      QVariant getProperty(Pid) const override;
      bool setProperty(Pid, const QVariant&) override;

      int lyricCount() const;
      int harmonyCount() const;
      bool hasPitchedStaff() const;
      bool hasTabStaff() const;
      bool hasDrumStaff() const;

      void updateHarmonyChannels(bool isDoOnInstrumentChanged, bool checkRemoval = false);
      const Channel* harmonyChannel() const;

      const Part* masterPart() const;
      Part* masterPart();

      PreferSharpFlat preferSharpFlat() const     { return _preferSharpFlat; }
      void setPreferSharpFlat(PreferSharpFlat v)  { _preferSharpFlat = v;    }

      // Allows not reading the same instrument twice on importing 2.X scores.
      // TODO: do we need instruments info in parts at all?
      friend void readPart206(Part*, XmlReader&);
      };

}     // namespace Ms
#endif

