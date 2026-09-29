/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "async/asyncable.h"
#include "engraving/dom/part.h"
#include "engraving/iengravingpluginmixerapiv1.h"
#include "modularity/ioc.h"

// api
#include "scoreelement.h"

namespace mu::engraving::apiv1 {
class EngravingItem;
class Fraction;
class Instrument;
class MixerChannel;
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

/** APIDOC
 * Controls playback settings for the part's primary instrument channel.
 * Volume, balance, mute, solo, and sound changes are not undoable. MIDI bank
 * and program changes are undoable.
 * @class MixerChannel
 * @memberof Engraving
 * @hideconstructor
 * @since MuseScore 4.7
 */
class MixerChannel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT

    /** APIDOC
     * Volume in decibels, from -60 to +12.
     * @q_property {Number}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(float volume READ volume WRITE setVolume)
    /** APIDOC
     * Balance from -1 (left) to 1 (right).
     * @q_property {Number}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(float balance READ balance WRITE setBalance)
    /** APIDOC
     * Whether the channel is muted.
     * @q_property {Boolean}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(bool muted READ muted WRITE setMuted)
    /** APIDOC
     * Whether the channel is soloed.
     * @q_property {Boolean}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(bool solo READ solo WRITE setSolo)
    /** APIDOC
     * MIDI bank number, from 0 to 255. Changes are undoable.
     * @q_property {Number}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(int midiBank READ midiBank WRITE setMidiBank)
    /** APIDOC
     * MIDI program number, from 0 to 127. Changes are undoable.
     * @q_property {Number}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(int midiProgram READ midiProgram WRITE setMidiProgram)

    muse::ContextInject<IEngravingPluginMixerApi> mixerApi;
    mu::engraving::InstrumentTrackId m_instrumentTrackId;
    mu::engraving::Part* m_part = nullptr;

public:
    MixerChannel(const mu::engraving::InstrumentTrackId& instrumentTrackId, mu::engraving::Part* part, QObject* parent = nullptr);

    float volume() const;
    void setVolume(float volume);
    float balance() const;
    void setBalance(float balance);
    bool muted() const;
    void setMuted(bool muted);
    bool solo() const;
    void setSolo(bool solo);
    int midiBank() const;
    void setMidiBank(int bank);
    int midiProgram() const;
    void setMidiProgram(int program);

    /** APIDOC
     * Asynchronously return available sound resources to callback(sounds, error).
     * Each sound has an id and name.
     * @method
     * @param {Function} callback Called with the sound list and an error string.
     * @since MuseScore 4.7
     */
    Q_INVOKABLE void availableSounds(QJSValue callback);

    /** APIDOC
     * Asynchronously request a sound resource and call callback(success, error).
     * @method
     * @param {String} soundId ID returned by availableSounds.
     * @param {Function} callback Called when the source change is submitted to
     * playback.
     * @since MuseScore 4.7
     */
    Q_INVOKABLE void setSound(const QString& soundId, QJSValue callback);
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
    /// \see \ref mu::plugins::api::Instrument::instrumentId
    /// "Instrument.instrumentId"
    /// \since MuseScore 4.6
    Q_PROPERTY(QString instrumentId READ instrumentId)
    /// The string identifier
    /// ([MusicXML Sound
    /// ID](https://www.musicxml.com/for-developers/standard-sounds/)) for the
    /// first instrument in this part. Was called using \ref instrumentId prior
    /// to 4.6
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
    /// One of 16 music channels that can be assigned an instrument. \since
    /// MuseScore 3.2.1
    Q_PROPERTY(int midiChannel READ midiChannel)
    /// One of the 128 different instruments in General MIDI. \since
    /// MuseScore 3.2.1
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
    /// This property is writeable since MuseScore 3.6 (and was writable in
    /// MuseScore 2.x)
    /// \since MuseScore 3.2.1
    Q_PROPERTY(bool show READ show WRITE setShow)

    /// List of instruments in this part.
    /// \since MuseScore 3.5
    Q_PROPERTY(QQmlListProperty<apiv1::Instrument> instruments READ instruments);

    /** APIDOC
     * Mixer controls for the primary instrument channel.
     * @readonly
     * @q_property {Engraving.MixerChannel}
     * @since MuseScore 4.7
     */
    Q_PROPERTY(apiv1::MixerChannel * mixerChannel READ mixerChannel)

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

    int startTrack() const
    {
        return static_cast<int>(part()->trackRange().startTrack);
    }

    int endTrack() const
    {
        return static_cast<int>(part()->trackRange().endTrack);
    }

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
    apiv1::MixerChannel* mixerChannel();
    QQmlListProperty<apiv1::Staff> staves();
    /// \endcond

private:
    apiv1::MixerChannel* m_mixerChannel = nullptr;

public:
    /// The instrument of the part at the given tick in the score.
    /// \param tick Tick location in the score, as an integer.
    /// \since MuseScore 3.5
    Q_INVOKABLE apiv1::Instrument* instrumentAtTick(int tick);

    /// The instrument of the part at the given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::Instrument* instrumentAtTick(apiv1::Fraction* tick);

    /// The long name of the part at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString longNameAtTick(apiv1::Fraction* tick);
    /// The short name of the part at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString shortNameAtTick(apiv1::Fraction* tick);
    /// The name of the part's instrument at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString instrumentNameAtTick(apiv1::Fraction* tick);
    /// The ID of the part's instrument at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE QString instrumentIdAtTick(apiv1::Fraction* tick);
    /// The currently active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem*
    currentHarpDiagramAtTick(apiv1::Fraction* tick);
    /// The next active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem*
    nextHarpDiagramFromTick(apiv1::Fraction* tick);
    /// The previous active harp pedal diagram at a given tick in the score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::EngravingItem*
    prevHarpDiagramFromTick(apiv1::Fraction* tick);
    /// The tick of the currently active harp pedal diagram at a given tick in the
    /// score.
    /// \param tick Tick location in the score, as a fraction.
    /// \since MuseScore 4.6
    Q_INVOKABLE apiv1::Fraction* tickOfCurrentHarpDiagram(apiv1::Fraction* tick);
};
} // namespace mu::engraving::apiv1

#endif
