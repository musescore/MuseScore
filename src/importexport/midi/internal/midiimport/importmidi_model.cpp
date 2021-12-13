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
#include "importmidi_model.h"

#include "translation.h"

#include "importmidi_inner.h"
#include "importmidi_clef.h"
#include "engraving/dom/instrtemplate.h"

namespace mu::iex::midi {
class TracksModel::Column
{
public:
    explicit Column(MidiOperations::Opers& opers)
        : _opers(opers) {}
    virtual ~Column() {}

    virtual QVariant value(int trackIndex) const = 0;
    virtual void setValue(const QVariant& value, int trackIndex) = 0;
    virtual QString headerName() const = 0;
    virtual bool isVisible(int /*trackIndex*/) const { return true; }
    virtual QStringList valueList(int /*trackIndex*/) const { return _values; }
    virtual int width() const { return -1; }
    virtual bool isEditable(int /*trackIndex*/) const { return true; }
    virtual bool isForAllTracksOnly() const { return false; }

protected:
    MidiOperations::Opers& _opers;
    QStringList _values;
};

TracksModel::TracksModel()
    : _trackCount(0)
    , _frozenColCount(0)
    , _isAllApplied(true)
{
}

TracksModel::~TracksModel()
{
}

void TracksModel::reset(const MidiOperations::Opers& opers,
                        const QList<std::string>& lyricsList,
                        int trackCount,
                        const QString& midiFile,
                        bool hasHumanBeats,
                        bool hasTempoText,
                        bool hasChordNames)
{
    beginResetModel();
    _trackOpers = opers;
    _columns.clear();
    _trackCount = trackCount;
    _frozenColCount = 0;
    _midiFile = midiFile;
    _isAllApplied = true;
    if (trackCount == 0) {
        return;
    }

    //-----------------------------------------------------------------------
    struct Import : Column {
        Import(MidiOperations::Opers& opers)
            : Column(opers) {}

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Import");
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.doImport.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.doImport.setValue(trackIndex, value.toBool());
        }
    };
    ++_frozenColCount;
    _columns.push_back(std::unique_ptr<Column>(new Import(_trackOpers)));

    //-----------------------------------------------------------------------
    struct Channel : Column {
        Channel(MidiOperations::Opers& opers)
            : Column(opers) {}
        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Channel");
        }

        bool isEditable(int /*trackIndex*/) const override { return false; }
        QVariant value(int trackIndex) const override
        {
            return QString::number(_opers.channel.value(trackIndex));
        }

        void setValue(const QVariant& /*value*/, int /*trackIndex*/) override {}
    };
    ++_frozenColCount;
    _columns.push_back(std::unique_ptr<Column>(new Channel(_trackOpers)));

    //-----------------------------------------------------------------------
    bool hasStaffName = false;
    for (int i = 0; i != _trackCount; ++i) {
        if (_trackOpers.staffName.value(i) != "") {
            hasStaffName = true;
            break;
        }
    }
    if (hasStaffName) {
        struct StaffName : Column {
            StaffName(MidiOperations::Opers& opers, const QString& midiFile)
                : Column(opers), _midiFile(midiFile)
            {
            }

            int width() const override { return 180; }
            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Staff name");
            }

            bool isEditable(int /*trackIndex*/) const override { return false; }
            QVariant value(int trackIndex) const override
            {
                MidiOperations::Data& opers = midiImportOperations;
                MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                return MidiCharset::convertToCharset(_opers.staffName.value(trackIndex));
            }

            void setValue(const QVariant& /*value*/, int /*trackIndex*/) override {}

        private:
            QString _midiFile;
        };
        ++_frozenColCount;
        _columns.push_back(std::unique_ptr<Column>(new StaffName(_trackOpers, _midiFile)));
    }

    //-----------------------------------------------------------------------
    struct MidiInstrumentName : Column {
        MidiInstrumentName(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        int width() const override { return 130; }
        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Sound");
        }

        bool isEditable(int /*trackIndex*/) const override { return false; }
        QVariant value(int trackIndex) const override
        {
            return _opers.midiInstrName.value(trackIndex);
        }

        void setValue(const QVariant& /*value*/, int /*trackIndex*/) override {}
    };
    ++_frozenColCount;
    _columns.push_back(std::unique_ptr<Column>(new MidiInstrumentName(_trackOpers)));

    //-----------------------------------------------------------------------
    bool hasMsInstrument = false;
    for (int i = 0; i != _trackCount; ++i) {
        if (!_trackOpers.msInstrList.value(i).empty()) {
            hasMsInstrument = true;
            break;
        }
    }
    if (hasMsInstrument) {
        struct MsInstrument : Column {
            MsInstrument(MidiOperations::Opers& opers)
                : Column(opers)
            {
            }

            int width() const override { return 220; }
            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "MuseScore Studio instrument");
            }

            bool isEditable(int trackIndex) const override
            {
                return _opers.msInstrList.value(trackIndex).size() > 1;
            }

            QVariant value(int trackIndex) const override
            {
                const int instrIndex = _opers.msInstrIndex.value(trackIndex);
                const auto& trackInstrList = _opers.msInstrList.value(trackIndex);
                const engraving::InstrumentTemplate* instr = (trackInstrList.empty())
                                                             ? nullptr : trackInstrList[instrIndex];
                return instrName(instr);
            }

            void setValue(const QVariant& value, int trackIndex) override
            {
                _opers.msInstrIndex.setValue(trackIndex, value.toInt());
            }

            QStringList valueList(int trackIndex) const override
            {
                auto list = QStringList();
                const auto& trackInstrList = _opers.msInstrList.value(trackIndex);
                for (const engraving::InstrumentTemplate* instr: trackInstrList) {
                    list.append(instrName(instr));
                }
                return list;
            }

        private:
            static QString instrName(const engraving::InstrumentTemplate* instr)
            {
                if (!instr) {
                    return "-";
                }
                if (!instr->trackName.isEmpty()) {
                    return instr->trackName;
                }
                if (instr->longNames.empty()) {
                    return instr->id;
                }
                return instr->longNames.front().name();
            }
        };
        _columns.push_back(std::unique_ptr<Column>(new MsInstrument(_trackOpers)));
    }

    //-----------------------------------------------------------------------
    if (!lyricsList.isEmpty()) {
        struct Lyrics : Column {
            Lyrics(MidiOperations::Opers& opers,
                   const QList<std::string>& lyricsList,
                   const QString& midiFile)
                : Column(opers), _lyricsList(lyricsList), _midiFile(midiFile)
            {
            }

            int width() const override { return 185; }
            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Lyrics");
            }

            QVariant value(int trackIndex) const override
            {
                int index = _opers.lyricTrackIndex.value(trackIndex);
                if (index >= 0) {
                    MidiOperations::Data& opers = midiImportOperations;
                    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                    return MidiCharset::convertToCharset(_lyricsList[index]);
                }
                return "";
            }

            void setValue(const QVariant& value, int trackIndex) override
            {
                // GUI lyrics list always have "" row, so: (index - 1)
                _opers.lyricTrackIndex.setValue(trackIndex, value.toInt() - 1);
            }

            QStringList valueList(int /*trackIndex*/) const override
            {
                MidiOperations::Data& opers = midiImportOperations;
                MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                auto list = QStringList("");
                for (const auto& lyric: _lyricsList) {
                    list.append(MidiCharset::convertToCharset(lyric));
                }
                return list;
            }

        private:
            QList<std::string> _lyricsList;
            QString _midiFile;
        };
        _columns.push_back(std::unique_ptr<Column>(new Lyrics(_trackOpers, lyricsList, _midiFile)));
    }

    //-----------------------------------------------------------------------
    struct QuantValue : Column {
        QuantValue(MidiOperations::Opers& opers)
            : Column(opers)
        {
            _values.push_back(muse::qtrc("iex_midi", "Quarter"));
            _values.push_back(muse::qtrc("iex_midi", "Eighth"));
            _values.push_back(muse::qtrc("iex_midi", "16th"));
            _values.push_back(muse::qtrc("iex_midi", "32nd"));
            _values.push_back(muse::qtrc("iex_midi", "64th"));
            _values.push_back(muse::qtrc("iex_midi", "128th"));
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Max. quantization");
        }

        QVariant value(int trackIndex) const override
        {
            return _values[(int)_opers.quantValue.value(trackIndex)];
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.quantValue.setValue(trackIndex, (MidiOperations::QuantValue)value.toInt());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new QuantValue(_trackOpers)));

    //-----------------------------------------------------------------------
    struct VoiceCount : Column {
        VoiceCount(MidiOperations::Opers& opers)
            : Column(opers)
        {
            _values.push_back("1");
            _values.push_back("2");
            _values.push_back("3");
            _values.push_back("4");
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Max. voices");
        }

        QVariant value(int trackIndex) const override
        {
            return _values[(int)_opers.maxVoiceCount.value(trackIndex)];
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.maxVoiceCount.setValue(trackIndex, (MidiOperations::VoiceCount)value.toInt());
        }

        bool isVisible(int trackIndex) const override
        {
            if (_opers.isDrumTrack.value(trackIndex)) {
                return false;
            }
            return true;
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new VoiceCount(_trackOpers)));

    //-----------------------------------------------------------------------
    struct Tuplets : Column {
        Tuplets(MidiOperations::Opers& opers, int trackCount)
            : Column(opers), _trackCount(trackCount)
        {
            _values.push_back(muse::qtrc("iex_midi", "2-plets"));
            _values.push_back(muse::qtrc("iex_midi", "3-plets"));
            _values.push_back(muse::qtrc("iex_midi", "4-plets"));
            _values.push_back(muse::qtrc("iex_midi", "5-plets"));
            _values.push_back(muse::qtrc("iex_midi", "7-plets"));
            _values.push_back(muse::qtrc("iex_midi", "9-plets"));
        }

        int width() const override { return 140; }
        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Tuplets");
        }

        QVariant value(int trackIndex) const override
        {
            QString val;
            if (_opers.search2plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "2";
            }
            if (_opers.search3plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "3";
            }
            if (_opers.search4plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "4";
            }
            if (_opers.search5plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "5";
            }
            if (_opers.search7plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "7";
            }
            if (_opers.search9plets.value(trackIndex)) {
                if (val != "") {
                    val += ", ";
                }
                val += "9";
            }
            return val;
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            const QStringList list = value.toStringList();

            Q_ASSERT_X(list.size() > 5, "MIDI import operations",
                       "Invalid size of the tuplets value list");

            bool searchTuplets = false;
            if (list[0] != "undefined") {
                const bool doSearch = (list[0] == "true");
                _opers.search2plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            if (list[1] != "undefined") {
                const bool doSearch = (list[1] == "true");
                _opers.search3plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            if (list[2] != "undefined") {
                const bool doSearch = (list[2] == "true");
                _opers.search4plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            if (list[3] != "undefined") {
                const bool doSearch = (list[3] == "true");
                _opers.search5plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            if (list[4] != "undefined") {
                const bool doSearch = (list[4] == "true");
                _opers.search7plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            if (list[5] != "undefined") {
                const bool doSearch = (list[5] == "true");
                _opers.search9plets.setValue(trackIndex, doSearch);
                if (!searchTuplets && doSearch) {
                    searchTuplets = true;
                }
            }
            _opers.searchTuplets.setValue(trackIndex, searchTuplets);
        }

        QStringList valueList(int trackIndex) const override
        {
            auto list = QStringList("__MultiValue__");

            list.append(_values[0]);
            list.append(checkBoxValue(trackIndex, _opers.search2plets));
            list.append(_values[1]);
            list.append(checkBoxValue(trackIndex, _opers.search3plets));
            list.append(_values[2]);
            list.append(checkBoxValue(trackIndex, _opers.search4plets));
            list.append(_values[3]);
            list.append(checkBoxValue(trackIndex, _opers.search5plets));
            list.append(_values[4]);
            list.append(checkBoxValue(trackIndex, _opers.search7plets));
            list.append(_values[5]);
            list.append(checkBoxValue(trackIndex, _opers.search9plets));

            return list;
        }

    private:
        QString checkBoxValue(int trackIndex,
                              const MidiOperations::TrackOp<bool>& operation) const
        {
            if (trackIndex == -1) {             // symbolizes all tracks
                const bool firstValue = operation.value(0);
                for (int i = 1; i < _trackCount; ++i) {
                    if (operation.value(i) != firstValue) {
                        return "undefined";
                    }
                }
                trackIndex = 0;           // to pick the first track value on return
            }
            return operation.value(trackIndex) ? "true" : "false";
        }

        int _trackCount;
    };
    _columns.push_back(std::unique_ptr<Column>(new Tuplets(_trackOpers, _trackCount)));

    //-----------------------------------------------------------------------
    if (hasHumanBeats) {
        struct TimeSig : Column {
            TimeSig(MidiOperations::Opers& opers)
                : Column(opers)
            {
                _values.push_back("2");
                _values.push_back("3");
                _values.push_back("4");
                _values.push_back("5");
                _values.push_back("6");
                _values.push_back("7");
                _values.push_back("9");
                _values.push_back("12");
                _values.push_back("15");
                _values.push_back("21");
                _numeratorCount = _values.size();

                _values.push_back("2");
                _values.push_back("4");
                _values.push_back("8");
                _values.push_back("16");
                _values.push_back("32");
            }

            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Time signature");
            }

            bool isForAllTracksOnly() const override { return true; }
            QVariant value(int /*trackIndex*/) const override
            {
                const int numeratorIndex = (int)_opers.timeSigNumerator.value();
                const int denominatorIndex = (int)_opers.timeSigDenominator.value();

                return _values[numeratorIndex] + " / " + _values[_numeratorCount + denominatorIndex];
            }

            void setValue(const QVariant& value, int /*trackIndex*/) override
            {
                const QStringList list = value.toStringList();

                Q_ASSERT_X(list.size() == 2, "MIDI import operations",
                           "Invalid size of the time signature value list");

                bool ok = false;
                _opers.timeSigNumerator.setValue((MidiOperations::TimeSigNumerator)list[0].toInt(&ok));

                Q_ASSERT_X(ok, "MIDI import operations", "Invalid numerator value");

                ok = false;
                _opers.timeSigDenominator.setValue((MidiOperations::TimeSigDenominator)list[1].toInt(&ok));

                Q_ASSERT_X(ok, "MIDI import operations", "Invalid denominator value");
            }

            QStringList valueList(int /*trackIndex*/) const override
            {
                auto list = QStringList("__TimeSig__");
                list.append("__Numerator__");
                list.append(QString::number((int)_opers.timeSigNumerator.value()));
                for (int i = 0; i != _numeratorCount; ++i) {
                    list.append(_values[i]);
                }
                list.append("__Denominator__");
                list.append(QString::number((int)_opers.timeSigDenominator.value()));
                for (int i = _numeratorCount; i != _values.size(); ++i) {
                    list.append(_values[i]);
                }
                return list;
            }

        private:
            int _numeratorCount;
        };
        _columns.push_back(std::unique_ptr<Column>(new TimeSig(_trackOpers)));

        //-----------------------------------------------------------------------
        struct MeasureCount2xLess : Column {
            MeasureCount2xLess(MidiOperations::Opers& opers)
                : Column(opers)
            {
            }

            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Halving the\nmeasure count");
            }

            bool isForAllTracksOnly() const override { return true; }
            QVariant value(int /*trackIndex*/) const override
            {
                return _opers.measureCount2xLess.value();
            }

            void setValue(const QVariant& value, int /*trackIndex*/) override
            {
                _opers.measureCount2xLess.setValue(value.toBool());
            }
        };
        _columns.push_back(std::unique_ptr<Column>(new MeasureCount2xLess(_trackOpers)));
    }

    //-----------------------------------------------------------------------
    struct Human : Column {
        Human(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Is human\nperformance");
        }

        bool isForAllTracksOnly() const override { return true; }
        QVariant value(int /*trackIndex*/) const override
        {
            return _opers.isHumanPerformance.value();
        }

        void setValue(const QVariant& value, int /*trackIndex*/) override
        {
            _opers.isHumanPerformance.setValue(value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new Human(_trackOpers)));

    //-----------------------------------------------------------------------
    struct StaffSplit : Column {
        StaffSplit(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Split staff");
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.doStaffSplit.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.doStaffSplit.setValue(trackIndex, value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new StaffSplit(_trackOpers)));

    //-----------------------------------------------------------------------
    struct ClefChanges : Column {
        ClefChanges(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Clef\nchanges");
        }

        bool isEditable(int trackIndex) const override
        {
            if (_opers.isDrumTrack.value(trackIndex)) {
                return false;
            }
            const int instrIndex = _opers.msInstrIndex.value(trackIndex);
            const auto& trackInstrList = _opers.msInstrList.value(trackIndex);
            const engraving::InstrumentTemplate* instr = (trackInstrList.empty())
                                                         ? nullptr : trackInstrList[instrIndex];
            if (instr && !MidiClef::hasGFclefs(instr)) {
                return false;
            }
            return true;
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.changeClef.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.changeClef.setValue(trackIndex, value.toBool());
        }

        bool isVisible(int trackIndex) const override
        {
            return isEditable(trackIndex);
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new ClefChanges(_trackOpers)));

    //-----------------------------------------------------------------------
    struct Simplify : Column {
        Simplify(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Simplify\ndurations");
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.simplifyDurations.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.simplifyDurations.setValue(trackIndex, value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new Simplify(_trackOpers)));

    //-----------------------------------------------------------------------
    struct ShowStaccato : Column {
        ShowStaccato(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Show\nstaccato");
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.showStaccato.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.showStaccato.setValue(trackIndex, value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new ShowStaccato(_trackOpers)));

    //-----------------------------------------------------------------------
    struct DottedNotes : Column {
        DottedNotes(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Dotted\nnotes");
        }

        QVariant value(int trackIndex) const override
        {
            return _opers.useDots.value(trackIndex);
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.useDots.setValue(trackIndex, value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new DottedNotes(_trackOpers)));

    //-----------------------------------------------------------------------
    if (hasTempoText) {
        struct ShowTempoText : Column {
            ShowTempoText(MidiOperations::Opers& opers)
                : Column(opers)
            {
            }

            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Show\ntempo text");
            }

            bool isForAllTracksOnly() const override { return true; }
            QVariant value(int /*trackIndex*/) const override
            {
                return _opers.showTempoText.value();
            }

            void setValue(const QVariant& value, int /*trackIndex*/) override
            {
                _opers.showTempoText.setValue(value.toBool());
            }
        };
        _columns.push_back(std::unique_ptr<Column>(new ShowTempoText(_trackOpers)));
    }

    //-----------------------------------------------------------------------
    if (hasChordNames) {
        struct ShowChordNames : Column {
            ShowChordNames(MidiOperations::Opers& opers)
                : Column(opers)
            {
            }

            QString headerName() const override
            {
                return muse::qtrc("iex_midi", "Show\nchord symbols");
            }

            bool isForAllTracksOnly() const override { return true; }
            QVariant value(int /*trackIndex*/) const override
            {
                return _opers.showChordNames.value();
            }

            void setValue(const QVariant& value, int /*trackIndex*/) override
            {
                _opers.showChordNames.setValue(value.toBool());
            }
        };
        _columns.push_back(std::unique_ptr<Column>(new ShowChordNames(_trackOpers)));
    }

    //-----------------------------------------------------------------------
    struct PickupBar : Column {
        PickupBar(MidiOperations::Opers& opers)
            : Column(opers)
        {
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Recognize\npickup measure");
        }

        bool isForAllTracksOnly() const override { return true; }
        QVariant value(int /*trackIndex*/) const override
        {
            return _opers.searchPickupMeasure.value();
        }

        void setValue(const QVariant& value, int /*trackIndex*/) override
        {
            _opers.searchPickupMeasure.setValue(value.toBool());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new PickupBar(_trackOpers)));

    //-----------------------------------------------------------------------
    struct Swing : Column {
        Swing(MidiOperations::Opers& opers)
            : Column(opers)
        {
            _values.push_back(muse::qtrc("iex_midi", "None (1:1)"));
            _values.push_back(muse::qtrc("iex_midi", "Swing (2:1)"));
            _values.push_back(muse::qtrc("iex_midi", "Shuffle (3:1)"));
        }

        QString headerName() const override
        {
            return muse::qtrc("iex_midi", "Detect swing");
        }

        int width() const override { return 130; }
        QVariant value(int trackIndex) const override
        {
            return _values[(int)_opers.swing.value(trackIndex)];
        }

        void setValue(const QVariant& value, int trackIndex) override
        {
            _opers.swing.setValue(trackIndex, (MidiOperations::Swing)value.toInt());
        }
    };
    _columns.push_back(std::unique_ptr<Column>(new Swing(_trackOpers)));

    endResetModel();
}

void TracksModel::clear()
{
    beginResetModel();
    _trackCount = 0;
    _frozenColCount = 0;
    _trackOpers = MidiOperations::Opers();
    _columns.clear();
    _isAllApplied = true;
    endResetModel();
}

const MidiOperations::Opers& TracksModel::trackOpers() const
{
    return _trackOpers;
}

void TracksModel::updateCharset()
{
    _isAllApplied = false;
    forceAllChanged();
}

void TracksModel::notifyAllApplied()
{
    _isAllApplied = true;
}

int TracksModel::rowFromTrackIndex(int trackIndex) const
{
    // first row reserved for all tracks if track count > 1
    return (_trackCount > 1) ? trackIndex + 1 : trackIndex;
}

int TracksModel::trackIndexFromRow(int row) const
{
    // first row reserved for all tracks if track count > 1
    // return -1 if row is all tracks row
    return (_trackCount > 1) ? row - 1 : row;
}

int TracksModel::trackCountForImport() const
{
    int count = 0;
    for (int i = 0; i != _trackCount; ++i) {
        if (_trackOpers.doImport.value(i)) {
            ++count;
        }
    }
    return count;
}

int TracksModel::frozenRowCount() const
{
    if (_trackCount > 1) {
        return 1;
    }
    return 0;
}

int TracksModel::frozenColCount() const
{
    return _frozenColCount;
}

int TracksModel::rowCount(const QModelIndex& /*parent*/) const
{
    return (_trackCount > 1) ? _trackCount + 1 : _trackCount;
}

int TracksModel::columnCount(const QModelIndex& /*parent*/) const
{
    return int(_columns.size());
}

bool TracksModel::editableSingleTrack(int trackIndex, int column) const
{
    return !(trackIndex >= 0 && _trackCount != 1 && _columns[column]->isForAllTracksOnly());
}

QVariant TracksModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const int trackIndex = trackIndexFromRow(index.row());
    if (!isTrackIndexValid(trackIndex) || !isColumnValid(index.column())) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (trackIndex == -1) {                 // all tracks
            if (_columns[index.column()]->isEditable(-1)) {
                QVariant value = _columns[index.column()]->value(0);
                if (value.typeId() == QMetaType::QString) {
                    value = QVariant();
                    if (!_columns[index.column()]->isForAllTracksOnly()) {
                        for (int i = 0; i < _trackCount; ++i) {
                            if (!_columns[index.column()]->isVisible(i)) {
                                continue;
                            }
                            const auto newValue
                                = _columns[index.column()]->value(i);
                            if (!value.isValid()) {                           // value to compare with
                                value = newValue;
                                continue;
                            }
                            if (newValue.toString() != value.toString()) {
                                return "…";
                            }
                        }
                    } else {
                        value = _columns[index.column()]->value(0);
                    }
                    if (value.isValid()) {
                        return value.toString();
                    }
                }
            }
        } else if (editableSingleTrack(trackIndex, index.column())
                   && _columns[index.column()]->isVisible(trackIndex)) {
            QVariant value = _columns[index.column()]->value(trackIndex);
            if (value.typeId() == QMetaType::QString) {
                return value.toString();
            }
        }
        break;
    case Qt::EditRole:
        if (_columns[index.column()]->isEditable(trackIndex)
            && editableSingleTrack(trackIndex, index.column())
            && _columns[index.column()]->isVisible(trackIndex)) {
            const auto list = _columns[index.column()]->valueList(trackIndex);
            if (!list.isEmpty()) {
                return list;
            }
        }
        break;
    case Qt::CheckStateRole:
        if (trackIndex == -1) {
            QVariant value = _columns[index.column()]->value(0);
            if (value.typeId() == QMetaType::Bool) {
                value = QVariant();
                if (!_columns[index.column()]->isForAllTracksOnly()) {
                    for (int i = 0; i < _trackCount; ++i) {
                        if (!_columns[index.column()]->isVisible(i)) {
                            continue;
                        }
                        const auto newValue = _columns[index.column()]->value(i);
                        if (!value.isValid()) {                         // value to compare with
                            value = newValue;
                            continue;
                        }
                        if (newValue.toBool() != value.toBool()) {
                            return Qt::PartiallyChecked;
                        }
                    }
                } else {
                    value = _columns[index.column()]->value(0);
                }
                if (value.isValid()) {
                    return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
                }
            }
        } else if (editableSingleTrack(trackIndex, index.column())
                   && _columns[index.column()]->isVisible(trackIndex)) {
            QVariant value = _columns[index.column()]->value(trackIndex);
            if (value.typeId() == QMetaType::Bool) {
                return (value.toBool()) ? Qt::Checked : Qt::Unchecked;
            }
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;
        break;
    case Qt::ToolTipRole:
        if (trackIndex != -1 && _columns[index.column()]->isVisible(trackIndex)) {
            QVariant value = _columns[index.column()]->value(trackIndex);
            if (value.typeId() == QMetaType::QString
                && _columns[index.column()]->valueList(trackIndex).empty()) {
                MidiOperations::Data& opers = midiImportOperations;
                MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, _midiFile);

                return MidiCharset::convertToCharset(value.toString().toUtf8().constData());
            }
        }
        break;
    case Qt::SizeHintRole:
        return QSize(_columns[index.column()]->width(), -1);
        break;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags TracksModel::editableFlags(int row, int col) const
{
    Qt::ItemFlags flags;
    const int trackIndex = trackIndexFromRow(row);

    if (_columns[col]->isVisible(trackIndex)) {
        if (_columns[col]->value(0).typeId() == QMetaType::Bool) {
            flags |= Qt::ItemIsUserCheckable;
        } else if (_columns[col]->isEditable(trackIndex)) {
            if (trackIndex == -1) {
                flags |= Qt::ItemIsEditable;
            } else if (editableSingleTrack(trackIndex, col)) {
                QVariant value = _columns[col]->value(0);
                if (value.typeId() != QMetaType::Bool) { // not checkboxes
                    flags |= Qt::ItemIsEditable;
                }
            }
        }
    }
    return flags;
}

Qt::ItemFlags TracksModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return {};
    }

    Qt::ItemFlags flags = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    const int trackIndex = trackIndexFromRow(index.row());

    if (trackIndex == -1) {         // all tracks row
        if (!_columns[index.column()]->isForAllTracksOnly()
            && _columns[index.column()]->isEditable(-1)) {
            for (int i = 0; i < _trackCount; ++i) {
                const auto newFlags = editableFlags(rowFromTrackIndex(i), index.column());
                if (newFlags) {
                    flags |= newFlags;
                    break;
                }
            }
        } else {
            flags |= editableFlags(index.row(), index.column());
        }
    } else {
        flags |= editableFlags(index.row(), index.column());
    }

    return flags;
}

void TracksModel::forceRowDataChanged(int row)
{
    const auto begIndex = this->index(row, 0);
    const auto endIndex = this->index(row, columnCount(QModelIndex()));
    emit dataChanged(begIndex, endIndex);
}

void TracksModel::forceColumnDataChanged(int col)
{
    const auto begIndex = this->index(0, col);
    const auto endIndex = this->index(rowCount(QModelIndex()), col);
    emit dataChanged(begIndex, endIndex);
}

void TracksModel::forceAllChanged()
{
    const auto begIndex = this->index(0, 0);
    const auto endIndex = this->index(rowCount(QModelIndex()), columnCount(QModelIndex()));
    emit dataChanged(begIndex, endIndex);
}

bool TracksModel::setData(const QModelIndex& index, const QVariant& value, int /*role*/)
{
    const int trackIndex = trackIndexFromRow(index.row());
    if (!isTrackIndexValid(trackIndex) || !isColumnValid(index.column())) {
        return false;
    }

    if (trackIndex == -1) {     // all tracks row
        if (!_columns[index.column()]->isForAllTracksOnly()) {
            for (int i = 0; i != _trackCount; ++i) {
                if (_columns[index.column()]->isVisible(i)) {
                    _columns[index.column()]->setValue(value, i);
                }
            }
            forceColumnDataChanged(index.column());
        } else {
            _columns[index.column()]->setValue(value, 0);
        }
    } else if (editableSingleTrack(trackIndex, index.column())
               && _columns[index.column()]->isVisible(trackIndex)) {
        _columns[index.column()]->setValue(value, trackIndex);
        emit dataChanged(index, index);
        if (_trackCount > 1) {      // update 'all tracks' row
            forceRowDataChanged(0);
        }
    }

    _isAllApplied = false;
    return true;
}

QVariant TracksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Vertical && !isRowValid(section))
        || (orientation == Qt::Horizontal && !isColumnValid(section))) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (!_columns.empty()) {
            return _columns[section]->headerName();
        }
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        if (_trackCount > 1) {
            if (section == 0) {
                return muse::qtrc("iex_midi", "All");
            }
            return section;
        }
        return section + 1;
    }
    return QVariant();
}

bool TracksModel::isTrackIndexValid(int trackIndex) const
{
    return trackIndex >= -1 && trackIndex < _trackCount;
}

bool TracksModel::isRowValid(int row) const
{
    return row >= 0 && ((_trackCount == 1) ? row < _trackCount : row <= _trackCount);
}

bool TracksModel::isColumnValid(int column) const
{
    return column >= 0 && column < (int)_columns.size();
}
} // namespace mu::iex::midi
