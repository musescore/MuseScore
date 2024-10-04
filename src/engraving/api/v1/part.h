/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_APIV1_PART_H
#define MU_ENGRAVING_APIV1_PART_H

#include <QQmlListProperty>

#include "engraving/dom/part.h"

// api
#include "scoreelement.h"

namespace mu::engraving::apiv1 {
class Instrument;
class Part;

//---------------------------------------------------------
//   InstrumentListProperty
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class InstrumentListProperty : public QQmlListProperty<Instrument>
{
public:
    InstrumentListProperty(Part* p);

    static qsizetype count(QQmlListProperty<Instrument>* l);
    static Instrument* at(QQmlListProperty<Instrument>* l, qsizetype i);
};

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part : public ScoreElement
{
    Q_OBJECT
    Q_PROPERTY(int startTrack READ startTrack)
    Q_PROPERTY(int endTrack READ endTrack)
    /**
     * The string identifier
     * ([MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/))
     * for the first instrument in this part.
     * \see \ref mu::plugins::api::Instrument::instrumentId "Instrument.instrumentId"
     * \since MuseScore 3.2
     */
    Q_PROPERTY(QString instrumentId READ instrumentId)
    /// The number of Chord Symbols. \since MuseScore 3.2.1
    Q_PROPERTY(int harmonyCount READ harmonyCount)
    /// Whether it is a percussion staff. \since MuseScore 3.2.1
    Q_PROPERTY(bool hasDrumStaff READ hasDrumStaff)
    /// Whether it is a 'normal' staff with notes. \since MuseScore 3.2.1
    Q_PROPERTY(bool hasPitchedStaff READ hasPitchedStaff)
    /// Whether it is a tablature staff. \since MuseScore 3.2.1
    Q_PROPERTY(bool hasTabStaff READ hasTabStaff)
    /// The number of lyrics syllables. \since MuseScore 3.2.1
    Q_PROPERTY(int lyricCount READ lyricCount)
    /// One of 16 music channels that can be assigned an instrument. \since MuseScore 3.2.1
    Q_PROPERTY(int midiChannel READ midiChannel)
    /// One of the 128 different instruments in General MIDI. \since MuseScore 3.2.1
    Q_PROPERTY(int midiProgram READ midiProgram)
    /// The long name for the current instrument.
    /// Note that this property was writeable in MuseScore v2.x
    /// \since MuseScore 3.2.1
    Q_PROPERTY(QString longName READ longName)
    /// The short name for the current instrument.
    /// Note that this property was writeable in MuseScore v2.x
    /// \since MuseScore 3.2.1
    Q_PROPERTY(QString shortName READ shortName)
    /// The name of the current part of music.
    /// It is shown in Mixer.
    ///
    /// Note that this property was writeable in MuseScore v2.x
    /// \since MuseScore 3.2.1
    Q_PROPERTY(QString partName READ partName)
    /// Whether part is shown or hidden.
    /// This property is writeable since MuseScore 3.6 (and was writable in MuseScore 2.x)
    /// \since MuseScore 3.2.1
    Q_PROPERTY(bool show READ show WRITE setShow)

    /**
     * List of instruments in this part.
     * \since MuseScore 3.5
     */
    Q_PROPERTY(QQmlListProperty<apiv1::Instrument> instruments READ instruments);

public:
    /// \cond MS_INTERNAL
    Part(mu::engraving::Part* p = nullptr, Ownership o = Ownership::SCORE)
        : ScoreElement(p, o) {}

    mu::engraving::Part* part() { return toPart(e); }
    const mu::engraving::Part* part() const { return toPart(e); }

    int startTrack() const { return static_cast<int>(part()->startTrack()); }
    int endTrack()   const { return static_cast<int>(part()->endTrack()); }
    QString instrumentId() const { return part()->instrument()->musicXmlId(); }
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
    void setShow(bool val) { set(engraving::Pid::VISIBLE, val); }

    InstrumentListProperty instruments();
    /// \endcond

    /**
     * Finds an instrument that is active in this part at the given \p tick.
     * \since MuseScore 3.5
     */
    Q_INVOKABLE apiv1::Instrument* instrumentAtTick(int tick);
};
}

#endif
