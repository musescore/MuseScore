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

#ifndef MU_ENGRAVING_APIV1_INSTRUMENT_H
#define MU_ENGRAVING_APIV1_INSTRUMENT_H

#include "engraving/dom/instrument.h"

#include <QQmlListProperty>

// api
#include "scoreelement.h"

#include "log.h"

namespace mu::engraving {
class Instrument;
}

namespace mu::engraving::apiv1 {
class Instrument;

//---------------------------------------------------------
//   Channel
///   Provides an access to channel properties. Similar to
///   Mixer, changes to some of the playback-related
///   properties are not recorded to undo stack and are not
///   revertable with standard user-visible "undo" action.
///   Changing MIDI patch though is undoable in normal way
///   ("dock" type plugins may need to call
///   \ref Score.startCmd / \ref Score.endCmd for that to
///   work properly).
///
///   Iterating over all channels in the current score can
///   be done as follows:
///   \code
///   var parts = curScore.parts;
///   for (var i = 0; i < parts.length; ++i) {
///       var part = parts[i];
///       var instrs = part.instruments;
///       for (var j = 0; j < instrs.length; ++j) {
///           var instr = instrs[j];
///           var channels = instr.channels;
///           for (var k = 0; k < channels.length; ++k) {
///               var channel = channels[k];
///               channel.volume = 64; // just for example, changing the channel's volume
///           }
///       }
///   }
///   \endcode
///   \since MuseScore 3.5
//---------------------------------------------------------

class Channel : public QObject
{
    Q_OBJECT

    mu::engraving::InstrChannel* _channel;
    mu::engraving::Part* _part;

    /** Name of this channel */
    Q_PROPERTY(QString name READ name)

    /**
     * Channel volume, from 0 to 127.
     * \note Changing this property is **not** revertable with a standard "undo"
     * action. Plugins may need to handle reverting this property change if
     * necessary.
     */
    Q_PROPERTY(int volume READ volume WRITE setVolume)
    /**
     * Channel pan, from 0 to 127.
     * \note Changing this property is **not** revertable with a standard "undo"
     * action. Plugins may need to handle reverting this property change if
     * necessary.
     */
    Q_PROPERTY(int pan READ pan WRITE setPan)
    /**
     * Channel chorus, from 0 to 127.
     * \note Changing this property is **not** revertable with a standard "undo"
     * action. Plugins may need to handle reverting this property change if
     * necessary.
     */
    Q_PROPERTY(int chorus READ chorus WRITE setChorus)
    /**
     * Channel reverb, from 0 to 127.
     * \note Changing this property is **not** revertable with a standard "undo"
     * action. Plugins may need to handle reverting this property change if
     * necessary.
     */
    Q_PROPERTY(int reverb READ reverb WRITE setReverb)
    /**
     * Whether this channel is muted.
     * \note Changing this property is **not** revertable with a standard "undo"
     * action. Plugins may need to handle reverting this property change if
     * necessary.
     */
    Q_PROPERTY(bool mute READ mute WRITE setMute)

    /**
     * MIDI program number, from 0 to 127. Changing this property is recorded
     * to the program's undo stack and can be reverted with a standard "undo"
     * action.
     */
    Q_PROPERTY(int midiProgram READ midiProgram WRITE setMidiProgram)
    /**
     * MIDI patch bank number. Changing this property is recorded
     * to the program's undo stack and can be reverted with a standard "undo"
     * action.
     */
    Q_PROPERTY(int midiBank READ midiBank WRITE setMidiBank)

    mu::engraving::InstrChannel* activeChannel();

    void setMidiBankAndProgram(int bank, int program, bool setUserBankController);

public:
    /// \cond MS_INTERNAL
    Channel(mu::engraving::InstrChannel* ch, mu::engraving::Part* p, QObject* parent = nullptr)
        : QObject(parent), _channel(ch), _part(p) {}

    QString name() const { return _channel->name(); }

    int volume() const { return _channel->volume(); }
    void setVolume(int val) { activeChannel()->setVolume(qBound(0, val, 127)); }
    int pan() const { return _channel->pan(); }
    void setPan(int val) { activeChannel()->setPan(qBound(0, val, 127)); }
    int chorus() const { return _channel->chorus(); }
    void setChorus(int val) { activeChannel()->setChorus(qBound(0, val, 127)); }
    int reverb() const { return _channel->reverb(); }
    void setReverb(int val) { activeChannel()->setReverb(qBound(0, val, 127)); }

    bool mute() const
    {
        DEPRECATED;
        //!@NOTE since MuseScore 4.0 mixer doen/t work via Channel
        return false;
    }

    void setMute(bool)
    {
        DEPRECATED;
        //!@NOTE since MuseScore 4.0 mixer doen/t work via Channel
    }

    int midiProgram() const { return _channel->program(); }
    void setMidiProgram(int prog);
    int midiBank() const { return _channel->bank(); }
    void setMidiBank(int bank);
    /// \endcond
};

//---------------------------------------------------------
//   StringData
///   \since MuseScore 3.5
//---------------------------------------------------------

class StringData : public QObject
{
    Q_OBJECT

    /**
     * List of strings in this instrument.
     * \returns A list of objects representing strings. Each
     * object has the following fields:
     * - \p pitch - pitch of this string on fret 0 (integer).
     * - \p open - if \p true, this string is not fretted and
     *             always open. If \p false, the string **is**
     *             fretted. For example, for classical guitar
     *             all strings are fretted, and for them
     *             \p open value will always be \p false.
     */
    Q_PROPERTY(QVariantList strings READ stringList)

    /** Number of frets in this instrument */
    Q_PROPERTY(int frets READ frets)

    mu::engraving::StringData _data;

public:
    /// \cond MS_INTERNAL
    StringData(const mu::engraving::StringData* d, QObject* parent = nullptr)
        : QObject(parent), _data(*d) {}

    QVariantList stringList() const;
    int frets() const { return _data.frets(); }
    /// \endcond
};

//---------------------------------------------------------
//   ChannelListProperty
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class ChannelListProperty : public QQmlListProperty<Channel>
{
public:
    ChannelListProperty(Instrument* i);

    static qsizetype count(QQmlListProperty<Channel>* l);
    static Channel* at(QQmlListProperty<Channel>* l, qsizetype i);
};

//---------------------------------------------------------
//   Instrument
///   \since MuseScore 3.5
//---------------------------------------------------------

class Instrument : public QObject
{
    Q_OBJECT

    /**
     * The string identifier
     * ([MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/))
     * for this instrument.
     * \see \ref apiv1::Part::instrumentId "Part.instrumentId"
     */
    Q_PROPERTY(QString instrumentId READ instrumentId)
    // mu::engraving::Instrument supports multiple short/long names (for aeolus instruments?)
    // but in practice only one is actually used. If this gets changed this API could
    // be expanded.
    /** The long name for this instrument. */
    Q_PROPERTY(QString longName READ longName)
    /** The short name for this instrument. */
    Q_PROPERTY(QString shortName READ shortName)

    /**
     * For fretted instruments, an information about this
     * instrument's strings.
     */
    Q_PROPERTY(apiv1::StringData * stringData READ stringData)

    // TODO: a property for drumset?

    Q_PROPERTY(QQmlListProperty<apiv1::Channel> channels READ channels)

    mu::engraving::Instrument* _instrument;
    mu::engraving::Part* _part;

public:
    /// \cond MS_INTERNAL
    Instrument(mu::engraving::Instrument* i, mu::engraving::Part* p)
        : QObject(), _instrument(i), _part(p) {}

    mu::engraving::Instrument* instrument() { return _instrument; }
    const mu::engraving::Instrument* instrument() const { return _instrument; }

    mu::engraving::Part* part() { return _part; }

    QString instrumentId() const { return instrument()->id(); }
    QString longName() const;
    QString shortName() const;

    apiv1::StringData* stringData() { return customWrap<StringData>(instrument()->stringData()); }

    ChannelListProperty channels();
    /// \endcond

    /** Checks whether two variables represent the same object. */
    Q_INVOKABLE bool is(apiv1::Instrument* other) { return other && instrument() == other->instrument(); }
};
}

#endif
