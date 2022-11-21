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
#ifndef MU_ENGRAVING_MASTERSCORE_H
#define MU_ENGRAVING_MASTERSCORE_H

#include "infrastructure/ifileinfoprovider.h"

#include "instrument.h"
#include "score.h"

namespace mu::engraving {
class EngravingProject;
class MscReader;
class MscWriter;
class ScoreReader;
class Read400;
}

namespace mu::engraving::compat {
class ScoreAccess;
class Read114;
class Read206;
class Read302;
class ReadStyleHook;
}

namespace mu::engraving {
class Excerpt;
class MasterScore;
class Part;
class RepeatList;
class Revisions;
class TempoMap;
class TimeSigMap;
class UndoStack;

class MidiMapping
{
    Part* _part;
    std::unique_ptr<InstrChannel> _articulation;
    signed char _port;
    signed char _channel;
    InstrChannel* masterChannel;
    PartChannelSettingsLink link;

    MidiMapping() = default;   // should be created only within MasterScore
    friend class MasterScore;

public:
    Part* part() { return _part; }
    const Part* part() const { return _part; }
    InstrChannel* articulation() { return _articulation.get(); }
    const InstrChannel* articulation() const { return _articulation.get(); }
    signed char port() const { return _port; }
    signed char channel() const { return _channel; }
};

class MasterScore : public Score
{
    OBJECT_ALLOCATOR(engraving, MasterScore)

    UndoStack* _undoStack = nullptr;
    TimeSigMap* _sigmap;
    TempoMap* _tempomap;
    RepeatList* _repeatList;
    RepeatList* _repeatList2;
    bool _expandRepeats = MScore::playRepeats;
    bool _playlistDirty = true;
    std::vector<Excerpt*> _excerpts;
    std::vector<PartChannelSettingsLink> _playbackSettingsLinks;
    Score* _playbackScore = nullptr;
    async::Channel<ScoreChangesRange> m_changesRangeChannel;

    bool _readOnly = false;

    CmdState _cmdState;       // modified during cmd processing

    Fraction _pos[3];                      ///< 0 - current, 1 - left loop, 2 - right loop

    int _midiPortCount = 0;                           // A count of ALSA midi out ports
//    QQueue<MidiInputEvent> _midiInputQueue;           // MIDI events that have yet to be processed
    std::list<MidiInputEvent> _activeMidiPitches;     // MIDI keys currently being held down
    std::vector<MidiMapping> _midiMapping;
    bool isSimpleMidiMapping = false;                 // midi mapping is simple if all ports and channels
                                                      // don't decrease and don't have gaps
    std::set<int> occupiedMidiChannels;               // each entry is port*16+channel, port range: 0-inf, channel: 0-15
    unsigned int searchMidiMappingFrom = 0;           // makes getting next free MIDI mapping faster

    double m_widthOfSegmentCell = 3;

    std::weak_ptr<EngravingProject> m_project;

    // FIXME: Move to EngravingProject
    // We can't yet, because m_project is not set on every MasterScore
    IFileInfoProviderPtr m_fileInfoProvider;

    bool m_saved { false };
    bool m_autosaveDirty { true };

    void reorderMidiMapping();
    void rebuildExcerptsMidiMapping();
    void removeDeletedMidiMapping();
    int updateMidiMapping();

    friend class EngravingProject;
    friend class compat::ScoreAccess;
    friend class compat::Read114;
    friend class compat::Read206;
    friend class compat::Read302;
    friend class Read400;

    MasterScore(std::weak_ptr<EngravingProject> project  = std::weak_ptr<EngravingProject>());
    MasterScore(const MStyle&, std::weak_ptr<EngravingProject> project  = std::weak_ptr<EngravingProject>());

    bool writeMscz(MscWriter& mscWriter, bool onlySelection = false, bool createThumbnail = true);
    bool exportPart(MscWriter& mscWriter, Score* partScore);

    void initParts(Excerpt*);

public:

    ~MasterScore();
    MasterScore* clone();

    Score* createScore();
    Score* createScore(const MStyle& s);

    std::weak_ptr<EngravingProject> project() const { return m_project; }

    bool isMaster() const override { return true; }
    bool readOnly() const override { return _readOnly; }
    void setReadOnly(bool ro) { _readOnly = ro; }
    UndoStack* undoStack() const override { return _undoStack; }
    TimeSigMap* sigmap() const override { return _sigmap; }
    TempoMap* tempomap() const override { return _tempomap; }
    async::Channel<ScoreChangesRange> changesChannel() const override { return m_changesRangeChannel; }

    bool playlistDirty() const override { return _playlistDirty; }
    void setPlaylistDirty() override;
    void setPlaylistClean() { _playlistDirty = false; }

    void setExpandRepeats(bool expandRepeats);
    bool expandRepeats() const { return _expandRepeats; }
    void updateRepeatListTempo();
    void updateRepeatList();
    const RepeatList& repeatList() const override;
    const RepeatList& repeatList2() const override;

    std::vector<Excerpt*>& excerpts() { return _excerpts; }
    const std::vector<Excerpt*>& excerpts() const { return _excerpts; }
    //   QQueue<MidiInputEvent>* midiInputQueue() override { return &_midiInputQueue; }
    std::list<MidiInputEvent>& activeMidiPitches() override { return _activeMidiPitches; }

    void setUpdateAll() override;

    void setLayoutAll(staff_idx_t staff = mu::nidx, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e = nullptr);

    CmdState& cmdState() override { return _cmdState; }
    const CmdState& cmdState() const override { return _cmdState; }
    void addLayoutFlags(LayoutFlags val) override { _cmdState.layoutFlags |= val; }
    void setInstrumentsChanged(bool val) override { _cmdState._instrumentsChanged = val; }

    void setExcerptsChanged(bool val) { _cmdState._excerptsChanged = val; }
    bool excerptsChanged() const { return _cmdState._excerptsChanged; }
    bool instrumentsChanged() const { return _cmdState._instrumentsChanged; }

    void setTempomap(TempoMap* tm);

    int midiPortCount() const { return _midiPortCount; }
    void setMidiPortCount(int val) { _midiPortCount = val; }
    std::vector<MidiMapping>& midiMapping() { return _midiMapping; }
    MidiMapping* midiMapping(int channel) { return &_midiMapping[channel]; }
    void addMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel);
    void updateMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel);
    int midiPort(int idx) const { return _midiMapping[idx].port(); }
    int midiChannel(int idx) const { return _midiMapping[idx].channel(); }
    void rebuildMidiMapping();
    void checkMidiMapping();
    bool exportMidiMapping() { return !isSimpleMidiMapping; }
    int getNextFreeMidiMapping(int p = -1, int ch = -1);
    int getNextFreeDrumMidiMapping();
//    void enqueueMidiEvent(MidiInputEvent ev) { _midiInputQueue.enqueue(ev); }
    void rebuildAndUpdateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth, bool expressive, bool force = false);
    void setSoloMute();

    using Score::pos;
    Fraction pos(POS pos) const { return _pos[int(pos)]; }
    void setPos(POS pos, Fraction tick);

    void addExcerpt(Excerpt*, size_t index = mu::nidx);
    void removeExcerpt(Excerpt*);
    void deleteExcerpt(Excerpt*);

    void initAndAddExcerpt(Excerpt*, bool);
    void initExcerpt(Excerpt*);
    void initEmptyExcerpt(Excerpt*);

    void setPlaybackScore(Score*);
    Score* playbackScore() { return _playbackScore; }
    const Score* playbackScore() const { return _playbackScore; }
    InstrChannel* playbackChannel(const InstrChannel* c) { return _midiMapping[c->channel()].articulation(); }
    const InstrChannel* playbackChannel(const InstrChannel* c) const { return _midiMapping[c->channel()].articulation(); }

    MasterScore* unrollRepeats();

    IFileInfoProviderPtr fileInfo() const;
    void setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider);

    bool saved() const;
    void setSaved(bool v);

    bool autosaveDirty() const;
    void setAutosaveDirty(bool v);

    String name() const override;

    void setWidthOfSegmentCell(double val) { m_widthOfSegmentCell = val; }
    double widthOfSegmentCell() const { return m_widthOfSegmentCell; }
};

extern MasterScore* gpaletteScore;
}

#endif // MU_ENGRAVING_MASTERSCORE_H
