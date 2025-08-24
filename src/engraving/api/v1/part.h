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
class EngravingItem;
class FractionWrapper;
class Instrument;
class Part;
class Staff;

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
    /// The first track in this part.
    Q_PROPERTY(int startTrack READ startTrack)
    /// The last track of the next part + 1.
    Q_PROPERTY(int endTrack READ endTrack)
    /// The MuseScore string identifier
    /// for the first instrument in this part.
    /// \see \ref mu::plugins::api::Instrument::instrumentId "Instrument.instrumentId"
    /// \since MuseScore 4.6
    Q_PROPERTY(QString instrumentId READ instrumentId)
    /// The string identifier
    /// ([MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/))
    /// for the first instrument in this part.
    /// Was called using \ref instrumentId prior to 4.6
    /// \see \ref mu::plugins::api::Instrument::musicXmlId "Instrument.musicXmlId"
    /// \since MuseScore 3.2
    Q_PROPERTY(QString musicXmlId READ musicXmlId)
    /// The number of Chord Symbols. \since MuseScore 3.2.1
    Q_PROPERTY(int harmonyCount READ harmonyCount)
    /// Whether this part has chord symbols.
    /// \since MuseScore 4.6
    Q_PROPERTY(bool hasChordSymbol READ hasChordSymbol)
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

    /// List of instruments in this part.
    /// \since MuseScore 3.5
    Q_PROPERTY(QQmlListProperty<apiv1::Instrument> instruments READ instruments);

    /// List of staves belonging to this part.
    /// \since MuseScore 4.6
    Q_PROPERTY(QQmlListProperty<apiv1::Staff> staves READ staves);
    /// The part object of this part in the main score.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Part * masterPart READ masterPart);

public:
    /// \cond MS_INTERNAL
    Part(mu::engraving::Part* p = nullptr, Ownership o = Ownership::SCORE)
        : ScoreElement(p, o) {}

    mu::engraving::Part* part() { return toPart(e); }
    const mu::engraving::Part* part() const { return toPart(e); }

    int startTrack() const { return static_cast<int>(part()->startTrack()); }
    int endTrack()   const { return static_cast<int>(part()->endTrack()); }
    QString instrumentId() const { return part()->instrument()->id(); }
    QString musicXmlId() const { return part()->instrument()->musicXmlId(); }
    int harmonyCount() const { return part()->harmonyCount(); }
    bool hasChordSymbol() { return part()->hasChordSymbol(); }
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
    apiv1::Part* masterPart() { return wrap<apiv1::Part>(part()->masterPart()); }

    InstrumentListProperty instruments();
    QQmlListProperty<apiv1::Staff> staves();
    /// \endcond

    /// The instrument of the part at the given tick in the score.
    /// \param tick Tick location in the score, as an integer.
    /// \since MuseScore 3.5
    Q_INVOKABLE apiv1::Instrument* instrumentAtTick(int tick);

    /// The instrument of the part at the given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::Instrument* instrumentAtTick(apiv1::FractionWrapper* tick);

    /// The long name of the part at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString longNameAtTick(apiv1::FractionWrapper* tick);
    /// The short name of the part at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString shortNameAtTick(apiv1::FractionWrapper* tick);
    /// The name of the part's instrument at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString instrumentNameAtTick(apiv1::FractionWrapper* tick);
    /// The ID of the part's instrument at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString instrumentIdAtTick(apiv1::FractionWrapper* tick);
    /// The currently active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* currentHarpDiagramAtTick(apiv1::FractionWrapper* tick);
    /// The next active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* nextHarpDiagramFromTick(apiv1::FractionWrapper* tick);
    /// The previous active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem* prevHarpDiagramFromTick(apiv1::FractionWrapper* tick);
    /// The tick of the currently active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::FractionWrapper* tickOfCurrentHarpDiagram(apiv1::FractionWrapper* tick);
};
}

#endif
