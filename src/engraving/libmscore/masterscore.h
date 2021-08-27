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

#include "score.h"

namespace mu::engraving::compat {
class Read114;
class Read206;
class Read302;
class ReadStyleHook;
}

namespace Ms {
//---------------------------------------------------------
//   MasterScore
//---------------------------------------------------------

class MasterScore : public Score
{
    Q_OBJECT
    UndoStack * _undoStack = nullptr;
    TimeSigMap* _sigmap;
    TempoMap* _tempomap;
    RepeatList* _repeatList;
    RepeatList* _repeatList2;
    bool _expandRepeats     { MScore::playRepeats };
    bool _playlistDirty     { true };
    QList<Excerpt*> _excerpts;
    std::vector<PartChannelSettingsLink> _playbackSettingsLinks;
    Score* _playbackScore = nullptr;
    Revisions* _revisions;

    bool _readOnly          { false };

    CmdState _cmdState;       // modified during cmd processing

    Fraction _pos[3];                      ///< 0 - current, 1 - left loop, 2 - right loop

    int _midiPortCount      { 0 };                    // A count of ALSA midi out ports
    QQueue<MidiInputEvent> _midiInputQueue;           // MIDI events that have yet to be processed
    std::list<MidiInputEvent> _activeMidiPitches;     // MIDI keys currently being held down
    std::vector<MidiMapping> _midiMapping;
    bool isSimpleMidiMaping = false;                  // midi mapping is simple if all ports and channels
                                                      // don't decrease and don't have gaps
    QSet<int> occupiedMidiChannels;                   // each entry is port*16+channel, port range: 0-inf, channel: 0-15
    unsigned int searchMidiMappingFrom = 0;           // makes getting next free MIDI mapping faster

    std::shared_ptr<mu::engraving::EngravingProject> m_project = nullptr;

    void parseVersion(const QString&);
    void reorderMidiMapping();
    void rebuildExcerptsMidiMapping();
    void removeDeletedMidiMapping();
    int updateMidiMapping();

    QFileInfo _sessionStartBackupInfo;
    QFileInfo info;

    FileError read(XmlReader&, bool ignoreVersionError, mu::engraving::compat::ReadStyleHook* styleHook = nullptr);
    FileError doRead(XmlReader& e);

    friend class mu::engraving::EngravingProject;
    friend class mu::engraving::compat::ScoreAccess;
    friend class mu::engraving::compat::Read114;
    friend class mu::engraving::compat::Read206;
    friend class mu::engraving::compat::Read302;

    MasterScore(std::shared_ptr<mu::engraving::EngravingProject> project);
    MasterScore(const MStyle&, std::shared_ptr<mu::engraving::EngravingProject> project);

    FileError loadMscz(const mu::engraving::MscReader& mscReader, bool ignoreVersionError);
    bool writeMscz(mu::engraving::MscWriter& mscWriter, bool onlySelection = false, bool createThumbnail = true);
    bool exportPart(mu::engraving::MscWriter& mscWriter, Score* partScore);

public:

    virtual ~MasterScore();
    MasterScore* clone();

    Score* createScore();
    Score* createScore(const MStyle& s);

    virtual bool isMaster() const override { return true; }
    virtual bool readOnly() const override { return _readOnly; }
    void setReadOnly(bool ro) { _readOnly = ro; }
    virtual UndoStack* undoStack() const override { return _undoStack; }
    virtual TimeSigMap* sigmap() const override { return _sigmap; }
    virtual TempoMap* tempomap() const override { return _tempomap; }

    virtual bool playlistDirty() const override { return _playlistDirty; }
    virtual void setPlaylistDirty() override;
    void setPlaylistClean() { _playlistDirty = false; }

    void setExpandRepeats(bool expandRepeats);
    void updateRepeatListTempo();
    virtual const RepeatList& repeatList() const override;
    virtual const RepeatList& repeatList2() const override;

    QList<Excerpt*>& excerpts() { return _excerpts; }
    const QList<Excerpt*>& excerpts() const { return _excerpts; }
    virtual QQueue<MidiInputEvent>* midiInputQueue() override { return &_midiInputQueue; }
    virtual std::list<MidiInputEvent>* activeMidiPitches() override { return &_activeMidiPitches; }

    virtual void setUpdateAll() override;

    void setLayoutAll(int staff = -1, const Element* e = nullptr);
    void setLayout(const Fraction& tick, int staff, const Element* e = nullptr);
    void setLayout(const Fraction& tick1, const Fraction& tick2, int staff1, int staff2, const Element* e = nullptr);

    virtual CmdState& cmdState() override { return _cmdState; }
    const CmdState& cmdState() const override { return _cmdState; }
    virtual void addLayoutFlags(LayoutFlags val) override { _cmdState.layoutFlags |= val; }
    virtual void setInstrumentsChanged(bool val) override { _cmdState._instrumentsChanged = val; }

    void setExcerptsChanged(bool val) { _cmdState._excerptsChanged = val; }
    bool excerptsChanged() const { return _cmdState._excerptsChanged; }
    bool instrumentsChanged() const { return _cmdState._instrumentsChanged; }

    Revisions* revisions() { return _revisions; }

    bool isSavable() const;
    void setTempomap(TempoMap* tm);

    int midiPortCount() const { return _midiPortCount; }
    void setMidiPortCount(int val) { _midiPortCount = val; }
    std::vector<MidiMapping>& midiMapping() { return _midiMapping; }
    MidiMapping* midiMapping(int channel) { return &_midiMapping[channel]; }
    void addMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel);
    void updateMidiMapping(Channel* channel, Part* part, int midiPort, int midiChannel);
    int midiPort(int idx) const { return _midiMapping[idx].port(); }
    int midiChannel(int idx) const { return _midiMapping[idx].channel(); }
    void rebuildMidiMapping();
    void checkMidiMapping();
    bool exportMidiMapping() { return !isSimpleMidiMaping; }
    int getNextFreeMidiMapping(int p = -1, int ch = -1);
    int getNextFreeDrumMidiMapping();
    void enqueueMidiEvent(MidiInputEvent ev) { _midiInputQueue.enqueue(ev); }
    void rebuildAndUpdateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth);
    void updateExpressive(Synthesizer* synth, bool expressive, bool force = false);
    void setSoloMute();

    using Score::pos;
    Fraction pos(POS pos) const { return _pos[int(pos)]; }
    void setPos(POS pos, Fraction tick);

    void addExcerpt(Excerpt*, int index=-1);
    void removeExcerpt(Excerpt*);
    void deleteExcerpt(Excerpt*);
    void initAndAddExcerpt(Excerpt*, bool);

    void setPlaybackScore(Score*);
    Score* playbackScore() { return _playbackScore; }
    const Score* playbackScore() const { return _playbackScore; }
    Channel* playbackChannel(const Channel* c) { return _midiMapping[c->channel()].articulation(); }
    const Channel* playbackChannel(const Channel* c) const { return _midiMapping[c->channel()].articulation(); }

    MasterScore* unrollRepeats();

    QFileInfo* fileInfo() { return &info; }
    const QFileInfo* fileInfo() const { return &info; }
    void setName(const QString&);

    const QFileInfo& sessionStartBackupInfo() const { return _sessionStartBackupInfo; }

    virtual QString title() const override;
};

extern Ms::MasterScore* gpaletteScore;
}

#endif // MU_ENGRAVING_MASTERSCORE_H
