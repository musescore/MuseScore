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
#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include "importmidi_inner.h"
#include "importmidi_operation.h"
#include "../midishared/midifile.h"

namespace mu::engraving {
class InstrumentTemplate;
}

namespace mu::iex::midi {
class ReducedFraction;

namespace MidiCharset {
QString defaultCharset();
}
namespace Quantize {
MidiOperations::QuantValue defaultQuantValueFromPreferences();
}

namespace MidiOperations {
// operation types are in importmidi_operation.h

// to add an operation one need to add code also to:
//   - importmidi_operation.h,
//   - importmidi_opmodel.cpp (2 places),
//   - importmidi_trmodel.cpp (2 places),
// and - other importmidi files where algorithm requires it

template<typename T>
class TrackOp
{
public:
    explicit TrackOp(T defaultValue)
        : _operation{{ -1, defaultValue }}, _canRedefineDefaultLater(true)
    {}

    const T& value(int trackIndex) const
    {
        const auto it = _operation.find(trackIndex);
        if (it == _operation.end()) {
            return _operation.find(-1)->second;
        }
        return it->second;
    }

    void setValue(int trackIndex, T value)
    {
        Q_ASSERT_X(trackIndex >= 0, "TrackOperation", "Invalid track index");

        if (value != this->value(trackIndex)) {
            _operation[trackIndex] = value;
        }
    }

    const T& defaultValue() const
    {
        return _operation.find(-1)->second;
    }

    bool canRedefineDefaultLater() const { return _canRedefineDefaultLater; }

    void setDefaultValue(T value, bool canRedefineDefaultLater = true)
    {
        Q_ASSERT_X(_canRedefineDefaultLater, "TrackOp::setDefaultValue",
                   "Cannot set default value");

        if (!_canRedefineDefaultLater) {
            return;
        }
        _operation[-1] = value;
        _canRedefineDefaultLater = canRedefineDefaultLater;
    }

    void clear()
    {
        T defaultVal = defaultValue();
        _operation.clear();
        _operation[-1] = defaultVal;
    }

private:
    // <track index, operation value>
    // if track index == -1 then it's default value (for all tracks)
    std::map<int, T> _operation;
    bool _canRedefineDefaultLater;
};

template<typename T>
class Op
{
public:

    explicit Op(T defaultValue)
        : _value(defaultValue), _valueWasSet(false),
        _defaultValue{defaultValue}, _canRedefineDefaultLater(true)
    {}

    T value() const
    {
        if (!_valueWasSet) {
            return _defaultValue;
        }
        return _value;
    }

    void setValue(T value)
    {
        if (value != this->value()) {
            _value = value;
            _valueWasSet = true;
        }
    }

    T defaultValue() const
    {
        return _defaultValue;
    }

    bool canRedefineDefaultLater() const { return _canRedefineDefaultLater; }

    void setDefaultValue(T value, bool canRedefineDefaultLater = true)
    {
        Q_ASSERT_X(_canRedefineDefaultLater, "Op::setDefaultValue",
                   "Cannot set default value");

        if (!_canRedefineDefaultLater) {
            return;
        }
        _defaultValue = value;
        _canRedefineDefaultLater = canRedefineDefaultLater;
    }

private:
    T _value;
    bool _valueWasSet;
    T _defaultValue;
    bool _canRedefineDefaultLater;
};

// values that can be changed

struct Opers
{
    // data that cannot be changed by the user
    TrackOp<int> channel = TrackOp<int>(int());
    TrackOp<std::string> staffName = TrackOp<std::string>(std::string());         // will be converted to unicode later
    TrackOp<QString> midiInstrName = TrackOp<QString>(QString());
    TrackOp<std::vector<const engraving::InstrumentTemplate*> > msInstrList
        = TrackOp<std::vector<const engraving::InstrumentTemplate*> >(
              std::vector<const engraving::InstrumentTemplate*>());
    TrackOp<bool> isDrumTrack = TrackOp<bool>(false);

    // operations for all tracks
    Op<bool> isHumanPerformance = Op<bool>(false);
    Op<bool> searchPickupMeasure = Op<bool>(true);
    Op<bool> measureCount2xLess = Op<bool>(false);
    Op<bool> showTempoText = Op<bool>(true);
    Op<bool> showChordNames = Op<bool>(true);
    Op<TimeSigNumerator> timeSigNumerator = Op<TimeSigNumerator>(TimeSigNumerator::_4);
    Op<TimeSigDenominator> timeSigDenominator = Op<TimeSigDenominator>(TimeSigDenominator::_4);

    // operations for individual tracks
    TrackOp<int> trackIndexAfterReorder = TrackOp<int>(0);
    TrackOp<bool> doImport = TrackOp<bool>(true);
    TrackOp<QuantValue> quantValue = TrackOp<QuantValue>(Quantize::defaultQuantValueFromPreferences());
    TrackOp<bool> searchTuplets = TrackOp<bool>(true);
    TrackOp<bool> search2plets = TrackOp<bool>(false);
    TrackOp<bool> search3plets = TrackOp<bool>(true);
    TrackOp<bool> search4plets = TrackOp<bool>(true);
    TrackOp<bool> search5plets = TrackOp<bool>(true);
    TrackOp<bool> search7plets = TrackOp<bool>(true);
    TrackOp<bool> search9plets = TrackOp<bool>(true);
    TrackOp<bool> useDots = TrackOp<bool>(true);
    TrackOp<bool> simplifyDurations = TrackOp<bool>(true);     // for drum tracks - remove rests and ties
    TrackOp<bool> showStaccato = TrackOp<bool>(true);
    TrackOp<bool> doStaffSplit = TrackOp<bool>(false);         // for drum tracks - split by voices
    TrackOp<VoiceCount> maxVoiceCount = TrackOp<VoiceCount>(VoiceCount::V_4);
    TrackOp<bool> changeClef = TrackOp<bool>(true);
    TrackOp<Swing> swing = TrackOp<Swing>(Swing::NONE);
    TrackOp<bool> removeDrumRests = TrackOp<bool>(true);
    TrackOp<int> lyricTrackIndex = TrackOp<int>(-1);           // empty lyric
    TrackOp<int> msInstrIndex = TrackOp<int>(-1);              // for empty instrument list
};

struct HumanBeatData
{
    std::set<ReducedFraction> beatSet;
    // to adapt human beats to a different time sig, if necessary
    int addedFirstBeats = 0;
    int addedLastBeats = 0;
    ReducedFraction firstChordTick;
    ReducedFraction lastChordTick;
    ReducedFraction timeSig;
    bool measureCount2xLess = false;
};

struct FileData
{
    MidiFile midiFile;
    QList<MTrack> tracks;
    int processingsOfOpenedFile = 0;
    bool hasTempoText = false;
    QByteArray HHeaderData;
    QByteArray VHeaderData;
    int trackCount = 0;
    Opers trackOpers;
    QString charset = MidiCharset::defaultCharset();
    // after the user apply MIDI import operations
    // this value should be set to false
    // tracks of <tick, lyric fragment> from karaoke files
    // QList of lyric tracks - there can be multiple lyric tracks,
    // lyric track count != MIDI track count in general
    QList<std::multimap<ReducedFraction, std::string> > lyricTracks;
    std::multimap<ReducedFraction, QString> chordNames;
    HumanBeatData humanBeatData;
};

class Data
{
public:
    FileData* data();
    const FileData* data() const;

    void addNewMidiFile(const QString& fileName);
    int currentTrack() const;
    void setMidiFileData(const QString& fileName, const MidiFile& midiFile);
    void excludeMidiFile(const QString& fileName);
    bool hasMidiFile(const QString& fileName);
    const MidiFile* midiFile(const QString& fileName);
    QStringList allMidiFiles() const;
    void setOperationsFile(const QString& fileName);

private:
    friend class CurrentTrackSetter;
    friend class CurrentMidiFileSetter;

    QString _currentMidiFile;
    QString _midiOperationsFile;
    int _currentTrack = -1;

    std::map<QString, FileData> _data;      // <file name, tracks data>
};

// scoped setter of current track
class CurrentTrackSetter
{
public:
    CurrentTrackSetter(Data& opers, int track)
        : _opers(opers)
    {
        _oldValue = _opers._currentTrack;
        _opers._currentTrack = track;
    }

    ~CurrentTrackSetter()
    {
        _opers._currentTrack = _oldValue;
    }

private:
    Data& _opers;
    int _oldValue;
    // disallow heap allocation - for stack-only usage
    void* operator new(size_t);                 // standard new
    void* operator new(size_t, void*);          // placement new
    void* operator new[](size_t);               // array new
    void* operator new[](size_t, void*);        // placement array new
};

// scoped setter of current MIDI file
class CurrentMidiFileSetter
{
public:
    CurrentMidiFileSetter(Data& opers, const QString& fileName)
        : _opers(opers)
    {
        _oldValue = _opers._currentMidiFile;
        _opers._currentMidiFile = fileName;
    }

    ~CurrentMidiFileSetter()
    {
        _opers._currentMidiFile = _oldValue;
    }

private:
    Data& _opers;
    QString _oldValue;
    // disallow heap allocation - for stack-only usage
    void* operator new(size_t);                 // standard new
    void* operator new(size_t, void*);          // placement new
    void* operator new[](size_t);               // array new
    void* operator new[](size_t, void*);        // placement array new
};
} // namespace MidiOperations

extern MidiOperations::Data midiImportOperations;
} // namespace mu::iex::midi

#endif // IMPORTMIDI_OPERATIONS_H
