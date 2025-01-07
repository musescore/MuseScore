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
#ifndef MU_ENGRAVING_MASTERSCORE_H
#define MU_ENGRAVING_MASTERSCORE_H

#include <array>

#include "../infrastructure/ifileinfoprovider.h"
#include "../infrastructure/eidregister.h"

#include "instrument.h"
#include "score.h"

namespace mu::engraving {
class EngravingProject;
class MscReader;
class MscWriter;
class MscLoader;
}

namespace mu::engraving::compat {
class ScoreAccess;
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
public:
    Part* part() { return m_part; }
    const Part* part() const { return m_part; }
    InstrChannel* articulation() { return m_articulation.get(); }
    const InstrChannel* articulation() const { return m_articulation.get(); }
    signed char port() const { return m_port; }
    signed char channel() const { return m_channel; }

private:

    MidiMapping() = default;   // should be created only within MasterScore
    friend class MasterScore;

    Part* m_part = nullptr;
    std::unique_ptr<InstrChannel> m_articulation;
    signed char m_port = 0;
    signed char m_channel = 0;
    InstrChannel* m_masterChannel = nullptr;
    PartChannelSettingsLink m_link;
};

class MasterScore : public Score
{
    OBJECT_ALLOCATOR(engraving, MasterScore)

public:

    ~MasterScore();
    MasterScore* clone();

    Score* createScore();
    Score* createScore(const MStyle& s);

    std::weak_ptr<EngravingProject> project() const { return m_project; }

    bool isMaster() const override { return true; }

    EIDRegister* eidRegister() { return &m_eidRegister; }
    const EIDRegister* eidRegister() const { return &m_eidRegister; }

    bool readOnly() const override { return m_readOnly; }
    void setReadOnly(bool ro) { m_readOnly = ro; }
    UndoStack* undoStack() const override { return m_undoStack; }
    TimeSigMap* sigmap() const override { return m_sigmap; }
    TempoMap* tempomap() const override { return m_tempomap; }
    muse::async::Channel<ScoreChangesRange> changesChannel() const override { return m_changesRangeChannel; }

    bool playlistDirty() const override { return m_playlistDirty; }
    void setPlaylistDirty() override;
    void setPlaylistClean() { m_playlistDirty = false; }

    /// Always call this before calling `repeatList()`
    /// No need to set it back after use, because everyone always calls it before using `repeatList()`
    void setExpandRepeats(bool expandRepeats);
    bool expandRepeats() const { return m_expandRepeats; }

    void updateRepeatListTempo();
    void updateRepeatList();

    const RepeatList& repeatList() const override;
    const RepeatList& repeatList(bool expandRepeats, bool updateTies = true) const override;

    std::vector<Excerpt*>& excerpts() { return m_excerpts; }
    const std::vector<Excerpt*>& excerpts() const { return m_excerpts; }
    //   QQueue<MidiInputEvent>* midiInputQueue() override { return &_midiInputQueue; }
    std::list<MidiInputEvent>& activeMidiPitches() override { return m_activeMidiPitches; }

    void setUpdateAll() override;

    void setLayoutAll(staff_idx_t staff = muse::nidx, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick, staff_idx_t staff, const EngravingItem* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e = nullptr);

    CmdState& cmdState() override { return m_cmdState; }
    const CmdState& cmdState() const override { return m_cmdState; }
    void addLayoutFlags(LayoutFlags val) override { m_cmdState.layoutFlags |= val; }
    void setInstrumentsChanged(bool val) override { m_cmdState.instrumentsChanged = val; }

    void setExcerptsChanged(bool val) { m_cmdState.excerptsChanged = val; }
    bool excerptsChanged() const { return m_cmdState.excerptsChanged; }
    bool instrumentsChanged() const { return m_cmdState.instrumentsChanged; }

    void setTempomap(TempoMap* tm);

    int midiPortCount() const { return m_midiPortCount; }
    void setMidiPortCount(int val) { m_midiPortCount = val; }
    std::vector<MidiMapping>& midiMapping() { return m_midiMapping; }
    MidiMapping* midiMapping(int channel) { return &m_midiMapping[channel]; }
    void addMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel);
    void updateMidiMapping(InstrChannel* channel, Part* part, int midiPort, int midiChannel);
    int midiPort(int idx) const { return m_midiMapping[idx].port(); }
    int midiChannel(int idx) const { return m_midiMapping[idx].channel(); }
    void rebuildMidiMapping();
    void checkMidiMapping();
    bool exportMidiMapping() { return !m_isSimpleMidiMapping; }
    int getNextFreeMidiMapping(std::set<int>& occupiedMidiChannels, unsigned int& searchMidiMappingFrom, int p = -1, int ch = -1);
    int getNextFreeDrumMidiMapping(std::set<int>& occupiedMidiChannels);
//    void enqueueMidiEvent(MidiInputEvent ev) { _midiInputQueue.enqueue(ev); }
    void rebuildAndUpdateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth, bool expressive, bool force = false);

    using Score::loopBoundaryTick;
    Fraction loopBoundaryTick(LoopBoundaryType type) const;
    void setLoopBoundaryTick(LoopBoundaryType type, Fraction tick);

    void addExcerpt(Excerpt*, size_t index = muse::nidx);
    void removeExcerpt(Excerpt*);
    void deleteExcerpt(Excerpt*);

    void initAndAddExcerpt(Excerpt*, bool);
    void initExcerpt(Excerpt*);
    void initEmptyExcerpt(Excerpt*);

    void setPlaybackScore(Score*);
    Score* playbackScore() { return m_playbackScore; }
    const Score* playbackScore() const { return m_playbackScore; }
    InstrChannel* playbackChannel(const InstrChannel* c) { return m_midiMapping[c->channel()].articulation(); }
    const InstrChannel* playbackChannel(const InstrChannel* c) const { return m_midiMapping[c->channel()].articulation(); }

    MasterScore* unrollRepeats();

    void splitMeasure(const Fraction&);
    void joinMeasure(const Fraction&, const Fraction&);

    MeasureBase* insertMeasure(MeasureBase* beforeMeasure = nullptr, const InsertMeasureOptions& options = InsertMeasureOptions());

    IFileInfoProviderPtr fileInfo() const;
    void setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider);

    bool saved() const;
    void setSaved(bool v);

    String name() const override;

    muse::Ret sanityCheck();

    void setWidthOfSegmentCell(double val) { m_widthOfSegmentCell = val; }
    double widthOfSegmentCell() const { return m_widthOfSegmentCell; }

private:

    void reorderMidiMapping();
    void rebuildExcerptsMidiMapping();
    void removeDeletedMidiMapping();
    int updateMidiMapping();

    friend class EngravingProject;
    friend class compat::ScoreAccess;
    friend class read114::Read114;
    friend class read400::Read400;

    MasterScore(const muse::modularity::ContextPtr& iocCtx, std::weak_ptr<EngravingProject> project = std::weak_ptr<EngravingProject>());
    MasterScore(const muse::modularity::ContextPtr& iocCtx, const MStyle&,
                std::weak_ptr<EngravingProject> project  = std::weak_ptr<EngravingProject>());

    void initParts(Excerpt*);

    EIDRegister m_eidRegister;
    UndoStack* m_undoStack = nullptr;
    TimeSigMap* m_sigmap = nullptr;
    TempoMap* m_tempomap = nullptr;
    RepeatList* m_expandedRepeatList = nullptr;
    RepeatList* m_nonExpandedRepeatList = nullptr;
    bool m_expandRepeats = true;
    bool m_playlistDirty = true;
    std::vector<Excerpt*> m_excerpts;
    std::vector<PartChannelSettingsLink> m_playbackSettingsLinks;
    Score* m_playbackScore = nullptr;
    muse::async::Channel<ScoreChangesRange> m_changesRangeChannel;

    bool m_readOnly = false;

    CmdState m_cmdState;       // modified during cmd processing

    std::array<Fraction, 2> m_loopBoundaries; ///< 0 - LoopIn, 1 - LoopOut

    int m_midiPortCount = 0;                           // A count of ALSA midi out ports
    //    QQueue<MidiInputEvent> _midiInputQueue;           // MIDI events that have yet to be processed
    std::list<MidiInputEvent> m_activeMidiPitches;     // MIDI keys currently being held down
    std::vector<MidiMapping> m_midiMapping;
    bool m_isSimpleMidiMapping = false;                 // midi mapping is simple if all ports and channels
    // don't decrease and don't have gaps
    double m_widthOfSegmentCell = 3;

    std::weak_ptr<EngravingProject> m_project;

    // FIXME: Move to EngravingProject
    // We can't yet, because m_project is not set on every MasterScore
    IFileInfoProviderPtr m_fileInfoProvider;

    bool m_saved = false;
};

extern MasterScore* gpaletteScore;
}

#endif // MU_ENGRAVING_MASTERSCORE_H
