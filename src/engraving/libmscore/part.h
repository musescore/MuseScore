/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __PART_H__
#define __PART_H__

#include <vector>

#include "mscore.h"
#include "instrument.h"
#include "text.h"
#include "types/types.h"

namespace mu::engraving::compat {
class Read206;
}

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

class Part final : public EngravingObject
{
    QString _partName;              ///< used in tracklist (mixer)
    InstrumentList _instruments;
    std::vector<Staff*> _staves;
    ID _id = INVALID_ID;             ///< used for MusicXml import
    bool _show = false;              ///< show part in partitur if true
    bool _soloist = false;           ///< used in score ordering
    int _capoFret = 0;

    static const int DEFAULT_COLOR = 0x3399ff;
    int _color = 0;                  ///User specified color for helping to label parts

    PreferSharpFlat _preferSharpFlat = PreferSharpFlat::DEFAULT;

    friend class mu::engraving::compat::Read206;

public:
    Part(Score* score = nullptr);
    void initFromInstrTemplate(const InstrumentTemplate*);

    ID id() const;
    void setId(const ID& id);

    Part* clone() const;

    void read(XmlReader&);
    bool readProperties(XmlReader&);
    void write(XmlWriter& xml) const;

    size_t nstaves() const;
    const std::vector<Staff*>& staves() const;
    void appendStaff(Staff* staff);
    void clearStaves();

    Staff* staff(staff_idx_t idx) const;
    QString familyId() const;

    track_idx_t startTrack() const;
    track_idx_t endTrack() const;

    InstrumentTrackIdSet instrumentTrackIdSet() const;

    QString longName(const Fraction& tick = { -1, 1 }) const;
    QString shortName(const Fraction& tick = { -1, 1 }) const;
    QString instrumentName(const Fraction& tick = { -1, 1 }) const;
    QString instrumentId(const Fraction& tick = { -1, 1 }) const;

    const std::list<StaffName>& longNames(const Fraction& tick = { -1, 1 }) const { return instrument(tick)->longNames(); }
    const std::list<StaffName>& shortNames(const Fraction& tick = { -1, 1 }) const { return instrument(tick)->shortNames(); }

    void setLongNames(std::list<StaffName>& s,  const Fraction& tick = { -1, 1 });
    void setShortNames(std::list<StaffName>& s, const Fraction& tick = { -1, 1 });

    void setLongName(const QString& s);
    void setShortName(const QString& s);

    void setPlainLongName(const QString& s);
    void setPlainShortName(const QString& s);

    void setStaves(int);

    int midiProgram() const;
    void setMidiProgram(int, int bank = 0);

    int capoFret() const;
    void setCapoFret(int capoFret);

    int midiChannel() const;
    int midiPort() const;
    void setMidiChannel(int ch, int port = -1, const Fraction& tick = { -1, 1 });  // tick != -1 for InstrumentChange

    void insertStaff(Staff*, staff_idx_t idx);
    void removeStaff(Staff*);
    bool show() const { return _show; }
    void setShow(bool val) { _show = val; }
    bool soloist() const { return _soloist; }
    void setSoloist(bool val) { _soloist = val; }

    Instrument* instrument(Fraction = { -1, 1 });
    const Instrument* instrument(Fraction = { -1, 1 }) const;
    void setInstrument(Instrument*, Fraction = { -1, 1 });         // transfer ownership
    void setInstrument(Instrument*, int tick);
    void setInstrument(const Instrument&&, Fraction = { -1, 1 });
    void setInstrument(const Instrument&, Fraction = { -1, 1 });
    void setInstruments(const InstrumentList& instruments);
    void removeInstrument(const Fraction&);
    void removeInstrument(const QString&);
    const InstrumentList& instruments() const;

    void insertTime(const Fraction& tick, const Fraction& len);

    QString partName() const { return _partName; }
    void setPartName(const QString& s) { _partName = s; }
    int color() const { return _color; }
    void setColor(int value) { _color = value; }

    bool isVisible() const;

    mu::engraving::PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const mu::engraving::PropertyValue&) override;

    int lyricCount() const;
    int harmonyCount() const;
    bool hasPitchedStaff() const;
    bool hasTabStaff() const;
    bool hasDrumStaff() const;

    void updateHarmonyChannels(bool isDoOnInstrumentChanged, bool checkRemoval = false);
    const Channel* harmonyChannel() const;

    const Part* masterPart() const;
    Part* masterPart();

    PreferSharpFlat preferSharpFlat() const { return _preferSharpFlat; }
    void setPreferSharpFlat(PreferSharpFlat v) { _preferSharpFlat = v; }

    // Allows not reading the same instrument twice on importing 2.X scores.
    // TODO: do we need instruments info in parts at all?
    friend void readPart206(Part*, XmlReader&);
};
}     // namespace Ms
#endif
