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
#include "masterscore.h"

#include "types/datetime.h"
#include "io/buffer.h"

#include "compat/writescorehook.h"
#include "infrastructure/mscwriter.h"
#include "rw/scorereader.h"
#include "rw/xml.h"
#include "style/defaultstyle.h"

#include "engravingproject.h"

#include "audio.h"
#include "excerpt.h"
#include "imageStore.h"
#include "part.h"
#include "repeatlist.h"
#include "sig.h"
#include "tempo.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::io;
using namespace mu::engraving;

//---------------------------------------------------------
//   MasterScore
//---------------------------------------------------------

MasterScore::MasterScore(std::weak_ptr<engraving::EngravingProject> project)
    : Score()
{
    m_project = project;
    _undoStack   = new UndoStack();
    _tempomap    = new TempoMap;
    _sigmap      = new TimeSigMap();
    _repeatList  = new RepeatList(this);
    _repeatList2 = new RepeatList(this);
    setMasterScore(this);

    _pos[int(POS::CURRENT)] = Fraction(0, 1);
    _pos[int(POS::LEFT)]    = Fraction(0, 1);
    _pos[int(POS::RIGHT)]   = Fraction(0, 1);

#if defined(Q_OS_WIN)
    metaTags().insert({ u"platform", u"Microsoft Windows" });
#elif defined(Q_OS_MAC)
    metaTags().insert({ u"platform", u"Apple Macintosh" });
#elif defined(Q_OS_LINUX)
    metaTags().insert({ u"platform", u"Linux" });
#else
    metaTags().insert({ u"platform", u"Unknown" });
#endif
    metaTags().insert({ u"movementNumber", u"" });
    metaTags().insert({ u"movementTitle", u"" });
    metaTags().insert({ u"workNumber", u"" });
    metaTags().insert({ u"workTitle", u"" });
    metaTags().insert({ u"arranger", u"" });
    metaTags().insert({ u"composer", u"" });
    metaTags().insert({ u"lyricist", u"" });
    metaTags().insert({ u"poet", u"" });
    metaTags().insert({ u"translator", u"" });
    metaTags().insert({ u"source", u"" });
    metaTags().insert({ u"copyright", u"" });
    metaTags().insert({ u"creationDate", Date::currentDate().toString(DateFormat::ISODate) });
}

MasterScore::MasterScore(const MStyle& s, std::weak_ptr<engraving::EngravingProject> project)
    : MasterScore{project}
{
    setStyle(s);
}

MasterScore::~MasterScore()
{
    if (m_project.lock()) {
        m_project.lock()->m_masterScore = nullptr;
    }

    delete _repeatList;
    delete _repeatList2;
    delete _sigmap;
    delete _tempomap;
    delete _undoStack;
    DeleteAll(_excerpts);
}

//---------------------------------------------------------
//   setTempomap
//---------------------------------------------------------

void MasterScore::setTempomap(TempoMap* tm)
{
    delete _tempomap;
    _tempomap = tm;
}

//---------------------------------------------------------
//   fileInfo
//---------------------------------------------------------

IFileInfoProviderPtr MasterScore::fileInfo() const
{
    return m_fileInfoProvider;
}

void MasterScore::setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider)
{
    m_fileInfoProvider = fileInfoProvider;
}

bool MasterScore::saved() const
{
    return m_saved;
}

void MasterScore::setSaved(bool v)
{
    m_saved = v;
}

bool MasterScore::autosaveDirty() const
{
    return m_autosaveDirty;
}

void MasterScore::setAutosaveDirty(bool v)
{
    m_autosaveDirty = v;
}

String MasterScore::name() const
{
    return fileInfo()->fileName(false).toString();
}

//---------------------------------------------------------
//   setPlaylistDirty
//---------------------------------------------------------

void MasterScore::setPlaylistDirty()
{
    _playlistDirty = true;
    _repeatList->setScoreChanged();
    _repeatList2->setScoreChanged();
}

//---------------------------------------------------------
//   setExpandRepeats
//---------------------------------------------------------

void MasterScore::setExpandRepeats(bool expand)
{
    if (_expandRepeats == expand) {
        return;
    }
    _expandRepeats = expand;
    setPlaylistDirty();
}

//---------------------------------------------------------
//   updateRepeatListTempo
///   needed for usage in Seq::processMessages
//---------------------------------------------------------

void MasterScore::updateRepeatListTempo()
{
    _repeatList->updateTempo();
    _repeatList2->updateTempo();
}

void MasterScore::updateRepeatList()
{
    _repeatList->update(MScore::playRepeats);
    _repeatList2->update(false);
}

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList() const
{
    _repeatList->update(MScore::playRepeats);
    return *_repeatList;
}

//---------------------------------------------------------
//   repeatList2
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList2() const
{
    _repeatList2->update(false);
    return *_repeatList2;
}

bool MasterScore::writeMscz(MscWriter& mscWriter, bool onlySelection, bool doCreateThumbnail)
{
    IF_ASSERT_FAILED(mscWriter.isOpened()) {
        return false;
    }

    // Write style of MasterScore
    {
        //! NOTE The style is writing to a separate file only for the master score.
        //! At the moment, the style for the parts is still writing to the score file.
        ByteArray styleData;
        Buffer styleBuf(&styleData);
        styleBuf.open(IODevice::WriteOnly);
        style().write(&styleBuf);
        mscWriter.writeStyleFile(styleData);
    }

    WriteContext ctx;

    // Write MasterScore
    {
        ByteArray scoreData;
        Buffer scoreBuf(&scoreData);
        scoreBuf.open(IODevice::ReadWrite);

        compat::WriteScoreHook hook;
        Score::writeScore(&scoreBuf, false, onlySelection, hook, ctx);

        mscWriter.writeScoreFile(scoreData);
    }

    // Write Excerpts
    {
        if (!onlySelection) {
            for (const Excerpt* excerpt : this->excerpts()) {
                Score* partScore = excerpt->excerptScore();
                if (partScore != this) {
                    // Write excerpt style
                    {
                        ByteArray excerptStyleData;
                        Buffer styleStyleBuf(&excerptStyleData);
                        styleStyleBuf.open(IODevice::WriteOnly);
                        partScore->style().write(&styleStyleBuf);

                        mscWriter.addExcerptStyleFile(excerpt->name(), excerptStyleData);
                    }

                    // Write excerpt
                    {
                        ByteArray excerptData;
                        Buffer excerptBuf(&excerptData);
                        excerptBuf.open(IODevice::ReadWrite);

                        compat::WriteScoreHook hook;
                        excerpt->excerptScore()->writeScore(&excerptBuf, false, onlySelection, hook, ctx);

                        mscWriter.addExcerptFile(excerpt->name(), excerptData);
                    }
                }
            }
        }
    }

    // Write ChordList
    {
        ChordList* chordList = this->chordList();
        if (chordList->customChordList() && !chordList->empty()) {
            ByteArray chlData;
            Buffer chlBuf(&chlData);
            chlBuf.open(IODevice::WriteOnly);
            chordList->write(&chlBuf);
            mscWriter.writeChordListFile(chlData);
        }
    }

    // Write images
    {
        for (ImageStoreItem* ip : imageStore) {
            if (!ip->isUsed(this)) {
                continue;
            }
            ByteArray data = ip->buffer();
            mscWriter.addImageFile(ip->hashName(), data);
        }
    }

    // Write thumbnail
    {
        if (doCreateThumbnail && !pages().empty()) {
            auto pixmap = createThumbnail();

            ByteArray ba;
            Buffer b(&ba);
            b.open(IODevice::WriteOnly);
            imageProvider()->saveAsPng(pixmap, &b);
            mscWriter.writeThumbnailFile(ba);
        }
    }

    // Write audio
    {
        if (_audio) {
            mscWriter.writeAudioFile(_audio->data());
        }
    }

    return true;
}

bool MasterScore::exportPart(MscWriter& mscWriter, Score* partScore)
{
    // Write excerpt style as main
    {
        ByteArray excerptStyleData;
        Buffer styleStyleBuf(&excerptStyleData);
        styleStyleBuf.open(IODevice::WriteOnly);
        partScore->style().write(&styleStyleBuf);

        mscWriter.writeStyleFile(excerptStyleData);
    }

    // Write excerpt as main score
    {
        ByteArray excerptData;
        Buffer excerptBuf(&excerptData);
        excerptBuf.open(IODevice::WriteOnly);

        compat::WriteScoreHook hook;
        partScore->writeScore(&excerptBuf, false, false, hook);

        mscWriter.writeScoreFile(excerptData);
    }

    // Write thumbnail
    {
        if (!partScore->pages().empty()) {
            auto pixmap = partScore->createThumbnail();

            ByteArray ba;
            Buffer b(&ba);
            b.open(IODevice::WriteOnly);
            imageProvider()->saveAsPng(pixmap, &b);
            mscWriter.writeThumbnailFile(ba);
        }
    }

    return true;
}

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void MasterScore::addExcerpt(Excerpt* ex, size_t index)
{
    if (!ex->inited()) {
        initParts(ex);
    }

    excerpts().insert(excerpts().begin() + (index == mu::nidx ? excerpts().size() : index), ex);
    setExcerptsChanged(true);
}

//---------------------------------------------------------
//   removeExcerpt
//---------------------------------------------------------

void MasterScore::removeExcerpt(Excerpt* ex)
{
    if (mu::remove(excerpts(), ex)) {
        setExcerptsChanged(true);
        // delete ex;
    } else {
        LOGD("removeExcerpt:: ex not found");
    }
}

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

MasterScore* MasterScore::clone()
{
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);

    WriteContext writeCtx;
    XmlWriter xml(&buffer);
    xml.setContext(&writeCtx);
    xml.startDocument();

    xml.startElement("museScore", { { "version", MSC_VERSION } });

    compat::WriteScoreHook hook;
    write(xml, false, hook);
    xml.endElement();

    buffer.close();

    ByteArray scoreData = buffer.data();
    MasterScore* score = new MasterScore(style(), m_project);

    ReadContext readCtx(score);
    readCtx.setIgnoreVersionError(true);

    XmlReader r(scoreData);
    r.setContext(&readCtx);

    ScoreReader().read(score, r, readCtx);

    score->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
    score->doLayout();
    return score;
}

Score* MasterScore::createScore()
{
    return new Score(this, DefaultStyle::baseStyle());
}

Score* MasterScore::createScore(const MStyle& s)
{
    return new Score(this, s);
}

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void MasterScore::setPos(POS pos, Fraction tick)
{
    if (tick < Fraction(0, 1)) {
        tick = Fraction(0, 1);
    }
    if (tick > lastMeasure()->endTick()) {
        // End Reverb may last longer than written notation, but cursor position should not
        tick = lastMeasure()->endTick();
    }

    _pos[int(pos)] = tick;
    // even though tick position might not have changed, layout might have
    // so we should update cursor here
    // however, we must be careful not to call setPos() again while handling posChanged, or recursion results
    for (Score* s : scoreList()) {
        s->notifyPosChanged(pos, unsigned(tick.ticks()));
    }
}

//---------------------------------------------------------
//   setSoloMute
//   called once at opening file, adds soloMute marks
//---------------------------------------------------------

void MasterScore::setSoloMute()
{
    for (unsigned i = 0; i < _midiMapping.size(); i++) {
        InstrChannel* b = _midiMapping[i].articulation();
        if (b->solo()) {
            b->setSoloMute(false);
            for (unsigned j = 0; j < _midiMapping.size(); j++) {
                InstrChannel* a = _midiMapping[j].articulation();
                bool sameMidiMapping = _midiMapping[i].port() == _midiMapping[j].port()
                                       && _midiMapping[i].channel() == _midiMapping[j].channel();
                a->setSoloMute((i != j && !a->solo() && !sameMidiMapping));
                a->setSolo(i == j || a->solo() || sameMidiMapping);
            }
        }
    }
}

//---------------------------------------------------------
//   setUpdateAll
//---------------------------------------------------------

void MasterScore::setUpdateAll()
{
    _cmdState.setUpdateMode(UpdateMode::UpdateAll);
}

//---------------------------------------------------------
//   setLayoutAll
//---------------------------------------------------------

void MasterScore::setLayoutAll(staff_idx_t staff, const EngravingItem* e)
{
    _cmdState.setTick(Fraction(0, 1));
    _cmdState.setTick(measures()->last() ? measures()->last()->endTick() : Fraction(0, 1));

    if (e && e->score() == this) {
        // TODO: map staff number properly
        const staff_idx_t startStaff = staff == mu::nidx ? 0 : staff;
        const staff_idx_t endStaff = staff == mu::nidx ? (nstaves() - 1) : staff;
        _cmdState.setStaff(startStaff);
        _cmdState.setStaff(endStaff);

        _cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setLayout
//---------------------------------------------------------

void MasterScore::setLayout(const Fraction& t, staff_idx_t staff, const EngravingItem* e)
{
    if (t >= Fraction(0, 1)) {
        _cmdState.setTick(t);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        _cmdState.setStaff(staff);
        _cmdState.setElement(e);
    }
}

void MasterScore::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    if (tick1 >= Fraction(0, 1)) {
        _cmdState.setTick(tick1);
    }
    if (tick2 >= Fraction(0, 1)) {
        _cmdState.setTick(tick2);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        _cmdState.setStaff(staff1);
        _cmdState.setStaff(staff2);

        _cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setPlaybackScore
//---------------------------------------------------------

void MasterScore::setPlaybackScore(Score* score)
{
    if (_playbackScore == score) {
        return;
    }

    _playbackScore = score;
    _playbackSettingsLinks.clear();

    if (!_playbackScore) {
        return;
    }

    for (MidiMapping& mm : _midiMapping) {
        mm.articulation()->setSoloMute(true);
    }
    for (Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            Instrument* instr = pair.second;
            for (InstrChannel* ch : instr->channel()) {
                InstrChannel* pChannel = playbackChannel(ch);
                IF_ASSERT_FAILED(pChannel) {
                    continue;
                }
                _playbackSettingsLinks.emplace_back(pChannel, ch, /* excerpt */ true);
            }
        }
    }
}

//---------------------------------------------------------
//   updateExpressive
//    change patches to their expressive equivalent or vica versa, if possible
//    This works only with MuseScore general soundfont
//
//    The first version of the function decides whether to make patches expressive
//    or not, based on the synth settings. The second will switch patches based on
//    the value of the expressive parameter.
//---------------------------------------------------------

void MasterScore::updateExpressive(Synthesizer* synth)
{
    SynthesizerState s = synthesizerState();
    SynthesizerGroup g = s.group(u"master");

    int method = 1;
    for (const IdValue& idVal : g) {
        if (idVal.id == 4) {
            method = idVal.data.toInt();
            break;
        }
    }

    updateExpressive(synth, (method != 0));
}

void MasterScore::updateExpressive(Synthesizer* synth, bool expressive, bool force /* = false */)
{
    if (!synth) {
        return;
    }

    if (!force) {
        SynthesizerState s = synthesizerState();
        SynthesizerGroup g = s.group(u"master");

        for (const IdValue& idVal : g) {
            if (idVal.id == 4) {
                int method = idVal.data.toInt();
                if (expressive == (method == 0)) {
                    return; // method and expression change don't match, so don't switch
                }
            }
        }
    }

    for (Part* p : parts()) {
        for (const auto& pair : p->instruments()) {
            pair.second->switchExpressive(this, synth, expressive, force);
        }
    }
}

//---------------------------------------------------------
//   rebuildAndUpdateExpressive
//    implicitly rebuild midi mappings as well. Should be preferred over
//    just updateExpressive, in most cases.
//---------------------------------------------------------

void MasterScore::rebuildAndUpdateExpressive(Synthesizer* synth)
{
    // Rebuild midi mappings to make sure we have playback channels
    rebuildMidiMapping();

    updateExpressive(synth);

    // Rebuild midi mappings again to be safe
    rebuildMidiMapping();
}
