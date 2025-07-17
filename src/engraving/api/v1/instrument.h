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

#pragma once

#include "engraving/dom/drumset.h"
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

    mu::engraving::InstrChannel* m_channel;
    mu::engraving::Part* m_part;

    /// Name of this channel
    Q_PROPERTY(QString name READ name)
    /// Whether this channel controls playback of chord symbols
    /// \since MuseScore 4.6
    Q_PROPERTY(bool isHarmonyChannel READ isHarmonyChannel)

    /// Channel volume, from 0 to 127.
    /// \note Changing this property is **not** revertable with a standard "undo"
    /// action. Plugins may need to handle reverting this property change if
    /// necessary.
    Q_PROPERTY(int volume READ volume WRITE setVolume)
    /// Channel pan, from 0 to 127.
    /// \note Changing this property is **not** revertable with a standard "undo"
    /// action. Plugins may need to handle reverting this property change if
    /// necessary.
    Q_PROPERTY(int pan READ pan WRITE setPan)
    /// Channel chorus, from 0 to 127.
    /// \note Changing this property is **not** revertable with a standard "undo"
    /// action. Plugins may need to handle reverting this property change if
    /// necessary.
    Q_PROPERTY(int chorus READ chorus WRITE setChorus)
    /// Channel reverb, from 0 to 127.
    /// \note Changing this property is **not** revertable with a standard "undo"
    /// action. Plugins may need to handle reverting this property change if
    /// necessary.
    Q_PROPERTY(int reverb READ reverb WRITE setReverb)
    /// Whether this channel is muted.
    /// \note Changing this property is **not** revertable with a standard "undo"
    /// action. Plugins may need to handle reverting this property change if
    /// necessary.
    Q_PROPERTY(bool mute READ mute WRITE setMute)

    /// MIDI program number, from 0 to 127. Changing this property is recorded
    /// to the program's undo stack and can be reverted with a standard "undo"
    /// action.
    Q_PROPERTY(int midiProgram READ midiProgram WRITE setMidiProgram)
    /// MIDI patch bank number. Changing this property is recorded
    /// to the program's undo stack and can be reverted with a standard "undo"
    /// action.
    Q_PROPERTY(int midiBank READ midiBank WRITE setMidiBank)

    mu::engraving::InstrChannel* activeChannel();

    void setMidiBankAndProgram(int bank, int program, bool setUserBankController);

public:
    /// \cond MS_INTERNAL
    Channel(mu::engraving::InstrChannel* ch, mu::engraving::Part* p, QObject* parent = nullptr)
        : QObject(parent), m_channel(ch), m_part(p) {}

    QString name() const { return m_channel->name(); }
    bool isHarmonyChannel() const { return m_channel->isHarmonyChannel(); }

    int volume() const { return m_channel->volume(); }
    void setVolume(int val) { activeChannel()->setVolume(qBound(0, val, 127)); }
    int pan() const { return m_channel->pan(); }
    void setPan(int val) { activeChannel()->setPan(qBound(0, val, 127)); }
    int chorus() const { return m_channel->chorus(); }
    void setChorus(int val) { activeChannel()->setChorus(qBound(0, val, 127)); }
    int reverb() const { return m_channel->reverb(); }
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

    int midiProgram() const { return m_channel->program(); }
    void setMidiProgram(int prog);
    int midiBank() const { return m_channel->bank(); }
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

    /// List of strings in this instrument.
    /// \returns A list of objects representing strings. Each
    /// object has the following fields:
    /// - \p pitch - pitch of this string on fret 0 (integer).
    /// - \p open - if \p true, this string is not fretted and
    ///             always open. If \p false, the string **is**
    ///             fretted. For example, for classical guitar
    ///             all strings are fretted, and for them
    ///             \p open value will always be \p false.
    Q_PROPERTY(QVariantList strings READ stringList)

    /// Number of frets in this instrument
    Q_PROPERTY(int frets READ frets)

    mu::engraving::StringData m_data;

public:
    /// \cond MS_INTERNAL
    StringData(const mu::engraving::StringData* d, QObject* parent = nullptr)
        : QObject(parent), m_data(*d) {}

    QVariantList stringList() const;
    int frets() const { return m_data.frets(); }
    /// \endcond
};

//---------------------------------------------------------
//   Drumset
///   \since MuseScore 4.6
//---------------------------------------------------------

class Drumset : public QObject
{
    Q_OBJECT

    mu::engraving::Drumset* m_drumset;

public:
    /// \cond MS_INTERNAL
    Drumset(mu::engraving::Drumset* d, QObject* parent = nullptr)
        : QObject(parent), m_drumset(d) {}

    mu::engraving::Drumset* drumset() { return m_drumset; }
    const mu::engraving::Drumset* drumset() const { return m_drumset; }
    /// \endcond

    /// Whether the given MIDI pitch corresponds to a note in the drumset.
    /// \param pitch The pitch to test for.
    Q_INVOKABLE bool isValid(int pitch) { return drumset()->isValid(pitch); }

    /// The notehead group for the given pitch, corresponding to one of the
    /// PluginAPI::PluginAPI::NoteHeadGroup values. If the value corresponds to
    /// NoteHeadGroup.HEAD_CUSTOM, the actual notehead must be found using
    /// PluginAPI::Drumset::noteHeads(pitch, NoteHeadType)
    /// \see PluginAPI::PluginAPI::NoteHeadGroup
    /// \see PluginAPI::Drumset::noteHeads
    /// \param pitch The pitch to find the notehead group for.
    Q_INVOKABLE int noteHead(int pitch) { return int(drumset()->noteHead(pitch)); }

    /// The notehead symbol for the given pitch and NoteHeadType, corresponding to
    /// one of the PluginAPI::PluginAPI::SymId values.
    /// \see PluginAPI::PluginAPI::SymId
    /// \see PluginAPI::PluginAPI::NoteHeadType
    /// \param pitch The pitch to find the notehead symbol for.
    /// \param type The NoteHeadType to find the notehead symbol for.
    Q_INVOKABLE int noteHeads(int pitch, int type)
    {
        return int(drumset()->noteHeads(pitch, mu::engraving::NoteHeadType(type)));
    }

    /// The line a note with the given pitch would be displayed on.
    /// \param pitch The pitch of the note to find the line for.
    Q_INVOKABLE int line(int pitch) { return drumset()->line(pitch); }

    /// The voice a note with the given pitch would be added to.
    /// \param pitch The pitch of the note to find the voice for.
    Q_INVOKABLE int voice(int pitch) { return drumset()->voice(pitch); }

    /// The default stem direction a note with the given pitch would have.
    /// One of the PluginAPI::PluginAPI::Direction values
    /// \see PluginAPI::PluginAPI::Direction
    /// \param pitch The pitch to find the stem direction for.
    Q_INVOKABLE int stemDirection(int pitch) { return int(drumset()->stemDirection(pitch)); }

    /// The name (untranslated) of the drumset note for a given pitch.
    /// \param pitch The pitch of the note to find the name for.
    Q_INVOKABLE QString name(int pitch) { return drumset()->name(pitch); }

    /// The translated name of the drumset note for a given pitch.
    /// \param pitch The pitch of the note to find the name for.
    Q_INVOKABLE QString translatedName(int pitch) { return drumset()->translatedName(pitch); }

    /// The shortcut of the drumset note for a given pitch.
    /// \param pitch The pitch of the note to find the shortcut for.
    Q_INVOKABLE QString shortcut(int pitch) { return drumset()->shortcut(pitch); }

    /// List of variants for a given pitch in this drumset.
    /// \returns A list of objects representing variants.
    /// Each object has the following fields:
    /// - \p pitch - pitch of this variant.
    /// - \p tremolo - tremolo type of this variant, one of
    ///             PluginAPI::PLUGIN_API::TremoloType values.
    /// - \p articulationName - the name of the articulation
    ///             for this variant.
    /// \param pitch Pitch to find variants for.
    Q_INVOKABLE QVariantList variants(int pitch);

    /// The row the given pitch appears in in the percussion panel.
    /// \param pitch The pitch of the note to find the row for.
    Q_INVOKABLE int panelRow(int pitch) { return drumset()->panelRow(pitch); }

    /// The column the given pitch appears in in the percussion panel.
    /// \param pitch The pitch of the note to find the column for.
    Q_INVOKABLE int panelColumn(int pitch) { return drumset()->panelColumn(pitch); }

    /// Tries to find the pitch of a normal notehead at "line". If a
    /// normal notehead can't be found it will instead return the
    /// "first valid pitch" (i.e. the lowest used midi note) in the drumset.
    /// \param line The line to find the default pitch on.
    Q_INVOKABLE int defaultPitchForLine(int line) { return drumset()->defaultPitchForLine(line); }

    /// The next used pitch from a given starting point.
    /// \param pitch The pitch from which to find the next used pitch from.
    Q_INVOKABLE int nextPitch(int pitch) { return drumset()->nextPitch(pitch); }

    /// The previous used pitch from a given starting point.
    /// \param pitch The pitch from which to find the previous used pitch from.
    Q_INVOKABLE int prevPitch(int pitch) { return drumset()->prevPitch(pitch); }

    /// Checks whether two drumsets represent the same object.
    Q_INVOKABLE bool is(apiv1::Drumset* other) { return other && drumset() == other->drumset(); }
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

    /// The MuseScore string identifier
    /// for this instrument.
    /// \see \ref apiv1::Part::instrumentId "Part.instrumentId"
    Q_PROPERTY(QString instrumentId READ instrumentId)
    /// The string identifier
    /// ([MusicXML Sound ID](https://www.musicxml.com/for-developers/standard-sounds/))
    /// for this instrument.
    /// \see \ref apiv1::Part::musicXmlId "Part.musicXmlId"
    Q_PROPERTY(QString musicXmlId READ musicXmlId)
    // mu::engraving::Instrument supports multiple short/long names (for aeolus instruments?)
    // but in practice only one is actually used. If this gets changed this API could
    // be expanded.
    /// The long name for this instrument.
    Q_PROPERTY(QString longName READ longName)
    /// The short name for this instrument.
    Q_PROPERTY(QString shortName READ shortName)

    /// For fretted instruments, an information about this
    /// instrument's strings.
    Q_PROPERTY(apiv1::StringData * stringData READ stringData)

    /// For unpitched percussion instruments, information about
    /// this instrument's percussion.
    /// \since MuseScore 4.6
    Q_PROPERTY(apiv1::Drumset * drumset READ drumset)

    /// The list of channels for this instrument.
    Q_PROPERTY(QQmlListProperty<apiv1::Channel> channels READ channels)

    mu::engraving::Instrument* m_instrument;
    mu::engraving::Part* m_part;

public:
    /// \cond MS_INTERNAL
    Instrument(mu::engraving::Instrument* i, mu::engraving::Part* p)
        : QObject(), m_instrument(i), m_part(p) {}

    mu::engraving::Instrument* instrument() { return m_instrument; }
    const mu::engraving::Instrument* instrument() const { return m_instrument; }

    mu::engraving::Part* part() { return m_part; }

    QString instrumentId() const { return instrument()->id(); }
    QString musicXmlId() const { return instrument()->musicXmlId(); }
    QString longName() const;
    QString shortName() const;

    apiv1::StringData* stringData() { return customWrap<StringData>(instrument()->stringData()); }

    apiv1::Drumset* drumset() { return customWrap<Drumset>(instrument()->drumset()); }

    ChannelListProperty channels();
    /// \endcond

    /// Checks whether two instruments represent the same object.
    Q_INVOKABLE bool is(apiv1::Instrument* other) { return other && instrument() == other->instrument(); }
};
}
