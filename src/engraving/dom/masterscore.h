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

#pragma once

#include <array>
#include <memory>

#include "../infrastructure/ifileinfoprovider.h"
#include "../infrastructure/eidregister.h"

#include "instrument.h"
#include "score.h"

namespace mu::engraving {
class EngravingProject;
class MscReader;
class MscWriter;
class MscLoader;
class TransactionManager;
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
class ScoreAutomationController;

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
    TransactionManager* transactionManager() const { return m_transactionManager.get(); }
    UndoStack* undoStack() const { return m_undoStack; }
    TimeSigMap* sigmap() const override { return m_sigmap; }
    TempoMap* tempomap() const override { return m_tempomap; }
    muse::async::Channel<ScoreChanges> changesChannel() const override { return m_changesChannel; }

    AutomationDataConstPtr automationData() const override;
    void setAutomationData(AutomationDataPtr data);
    void editAutomationPoints(const AutomationCurveKey& key, AutomationPointEdits& edits) override;

    /// Always call this before calling `repeatList()`
    /// No need to set it back after use, because everyone always calls it before using `repeatList()`
    void setExpandRepeats(bool expandRepeats);
    bool expandRepeats() const { return m_expandRepeats; }

    void updateRepeatListTempo();
    void updateRepeatList();

    const RepeatList& repeatList() const;
    const RepeatList& repeatList(bool expandRepeats, bool updateTies = true) const;

    void invalidateRepeatList();

    std::vector<Excerpt*>& excerpts() { return m_excerpts; }
    const std::vector<Excerpt*>& excerpts() const { return m_excerpts; }

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

    void update() { update(true); }
    void lockUpdates(bool locked);

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
    void rebuildAndUpdateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth, bool expressive, bool force = false);

    using Score::loopBoundaryTick;
    Fraction loopBoundaryTick(LoopBoundaryType type) const;
    void setLoopBoundaryTick(LoopBoundaryType type, Fraction tick);

    void addExcerpt(Excerpt*, size_t index = muse::nidx, bool initIfNeeded = true);
    void removeExcerpt(Excerpt*);
    void deleteExcerpt(Excerpt*);

    void initAndAddExcerpt(Excerpt*, bool);
    void initExcerpt(Excerpt*);
    void initEmptyExcerpt(Excerpt*);

    void initAutomation(); // TODO: Placeholder?

    void setPlaybackScore(Score*);
    Score* playbackScore() { return m_playbackScore; }
    const Score* playbackScore() const { return m_playbackScore; }
    InstrChannel* playbackChannel(const InstrChannel* c) { return m_midiMapping[c->channel()].articulation(); }
    const InstrChannel* playbackChannel(const InstrChannel* c) const { return m_midiMapping[c->channel()].articulation(); }

    MasterScore* unrollRepeats();

    MeasureBase* insertMeasure(MeasureBase* beforeMeasure = nullptr, const InsertMeasureOptions& options = InsertMeasureOptions());

    IFileInfoProviderPtr fileInfo() const;
    void setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider);

    String name() const override;

    muse::Ret sanityCheck();

private:
    void update(bool resetCmdState, bool layoutAllParts = false);

    void updateAutomation(const ScoreChanges& changes);

    void onTimeInserted(const Fraction& tick, const Fraction& len) override;

    void reorderMidiMapping();
    void rebuildExcerptsMidiMapping();
    void removeDeletedMidiMapping();

    int updateMidiMapping();
    void doUpdateMidiMapping(int& maxport, std::set<int>& occupiedMidiChannels, unsigned int& searchMidiMappingFrom, Part* part,
                             InstrChannel* channel, bool useDrumset);

    friend class EngravingProject;
    friend class compat::ScoreAccess;
    friend class read114::Read114;
    friend class read400::Read400;
    friend class TransactionManager;

    MasterScore(const muse::modularity::ContextPtr& iocCtx, std::weak_ptr<EngravingProject> project = std::weak_ptr<EngravingProject>());
    MasterScore(const muse::modularity::ContextPtr& iocCtx, const MStyle&,
                std::weak_ptr<EngravingProject> project  = std::weak_ptr<EngravingProject>());

    void initParts(Excerpt*);

    EIDRegister m_eidRegister;
    std::unique_ptr<TransactionManager> m_transactionManager;
    UndoStack* m_undoStack = nullptr;
    TimeSigMap* m_sigmap = nullptr;
    TempoMap* m_tempomap = nullptr;
    RepeatList* m_expandedRepeatList = nullptr;
    RepeatList* m_nonExpandedRepeatList = nullptr;
    ScoreAutomationController* m_automationController = nullptr;
    bool m_expandRepeats = true;

    std::vector<Excerpt*> m_excerpts;
    std::vector<PartChannelSettingsLink> m_playbackSettingsLinks;
    Score* m_playbackScore = nullptr;
    muse::async::Channel<ScoreChanges> m_changesChannel;

    bool m_readOnly = false;

    CmdState m_cmdState;       // modified during cmd processing
    bool m_updatesLocked = false;

    std::array<Fraction, 2> m_loopBoundaries; ///< 0 - LoopIn, 1 - LoopOut

    int m_midiPortCount = 0;                           // A count of ALSA midi out ports
    std::vector<MidiMapping> m_midiMapping;
    bool m_isSimpleMidiMapping = false;                 // midi mapping is simple if all ports and channels
    // don't decrease and don't have gaps

    std::weak_ptr<EngravingProject> m_project;

    // FIXME: Move to EngravingProject
    // We can't yet, because m_project is not set on every MasterScore
    IFileInfoProviderPtr m_fileInfoProvider;
};
}
