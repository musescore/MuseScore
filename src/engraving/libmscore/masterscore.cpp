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

#include <QDate>
#include <QBuffer>
#include <QRegularExpression>

#include "io/mscreader.h"
#include "io/mscwriter.h"
#include "rw/xml.h"
#include "rw/writecontext.h"
#include "style/defaultstyle.h"
#include "compat/writescorehook.h"
#include "rw/scorereader.h"

#include "engravingproject.h"

#include "repeatlist.h"
#include "undo.h"
#include "revisions.h"
#include "imageStore.h"
#include "audio.h"
#include "utils.h"
#include "excerpt.h"
#include "part.h"
#include "linkedobjects.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace Ms;

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
    _revisions   = new Revisions;
    setMasterScore(this);

    _pos[int(POS::CURRENT)] = Fraction(0, 1);
    _pos[int(POS::LEFT)]    = Fraction(0, 1);
    _pos[int(POS::RIGHT)]   = Fraction(0, 1);

#if defined(Q_OS_WIN)
    metaTags().insert({ "platform", "Microsoft Windows" });
#elif defined(Q_OS_MAC)
    metaTags().insert({ "platform", "Apple Macintosh" });
#elif defined(Q_OS_LINUX)
    metaTags().insert({ "platform", "Linux" });
#else
    metaTags().insert({ "platform", "Unknown" });
#endif
    metaTags().insert({ "movementNumber", "" });
    metaTags().insert({ "movementTitle", "" });
    metaTags().insert({ "workNumber", "" });
    metaTags().insert({ "workTitle", "" });
    metaTags().insert({ "arranger", "" });
    metaTags().insert({ "composer", "" });
    metaTags().insert({ "lyricist", "" });
    metaTags().insert({ "poet", "" });
    metaTags().insert({ "translator", "" });
    metaTags().insert({ "source", "" });
    metaTags().insert({ "copyright", "" });
    metaTags().insert({ "creationDate", QDate::currentDate().toString(Qt::ISODate) });
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

    delete _revisions;
    delete _repeatList;
    delete _repeatList2;
    delete _sigmap;
    delete _tempomap;
    delete _undoStack;
    qDeleteAll(_excerpts);
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

bool MasterScore::isNewlyCreated() const
{
    return m_isNewlyCreated;
}

void MasterScore::setNewlyCreated(bool val)
{
    m_isNewlyCreated = val;
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

QString MasterScore::name() const
{
    return fileInfo()->fileName(false).toQString();
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
        QByteArray styleData;
        QBuffer styleBuf(&styleData);
        styleBuf.open(QIODevice::WriteOnly);
        style().write(&styleBuf);
        mscWriter.writeStyleFile(styleData);
    }

    WriteContext ctx;

    // Write MasterScore
    {
        QByteArray scoreData;
        QBuffer scoreBuf(&scoreData);
        scoreBuf.open(QIODevice::ReadWrite);

        compat::WriteScoreHook hook;
        Score::writeScore(&scoreBuf, false, onlySelection, hook, ctx);

        mscWriter.writeScoreFile(scoreData);
    }

    // Write Excerpts
    {
        if (!onlySelection) {
            for (const Excerpt* excerpt : qAsConst(this->excerpts())) {
                Score* partScore = excerpt->excerptScore();
                if (partScore != this) {
                    // Write excerpt style
                    {
                        QByteArray excerptStyleData;
                        QBuffer styleStyleBuf(&excerptStyleData);
                        styleStyleBuf.open(QIODevice::WriteOnly);
                        partScore->style().write(&styleStyleBuf);

                        mscWriter.addExcerptStyleFile(excerpt->name(), excerptStyleData);
                    }

                    // Write excerpt
                    {
                        QByteArray excerptData;
                        QBuffer excerptBuf(&excerptData);
                        excerptBuf.open(QIODevice::ReadWrite);

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
            QByteArray chlData;
            QBuffer chlBuf(&chlData);
            chlBuf.open(QIODevice::WriteOnly);
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
            mscWriter.addImageFile(ip->hashName(), ip->buffer());
        }
    }

    // Write thumbnail
    {
        if (doCreateThumbnail && !pages().empty()) {
            auto pixmap = createThumbnail();

            QByteArray ba;
            QBuffer b(&ba);
            b.open(QIODevice::WriteOnly);
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

bool MasterScore::exportPart(mu::engraving::MscWriter& mscWriter, Score* partScore)
{
    // Write excerpt style as main
    {
        QByteArray excerptStyleData;
        QBuffer styleStyleBuf(&excerptStyleData);
        styleStyleBuf.open(QIODevice::WriteOnly);
        partScore->style().write(&styleStyleBuf);

        mscWriter.writeStyleFile(excerptStyleData);
    }

    // Write excerpt as main score
    {
        QByteArray excerptData;
        QBuffer excerptBuf(&excerptData);
        excerptBuf.open(QIODevice::WriteOnly);

        compat::WriteScoreHook hook;
        partScore->writeScore(&excerptBuf, false, false, hook);

        mscWriter.writeScoreFile(excerptData);
    }

    // Write thumbnail
    {
        if (!partScore->pages().empty()) {
            auto pixmap = partScore->createThumbnail();

            QByteArray ba;
            QBuffer b(&ba);
            b.open(QIODevice::WriteOnly);
            imageProvider()->saveAsPng(pixmap, &b);
            mscWriter.writeThumbnailFile(ba);
        }
    }

    return true;
}

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void MasterScore::addExcerpt(Excerpt* ex, int index)
{
    Score* score = ex->excerptScore();

    int nstaves { 1 }; // Initialise to 1 to force writing of the first part.
    std::set<ID> assignedStavesIds;
    for (Staff* excerptStaff : score->staves()) {
        const LinkedObjects* ls = excerptStaff->links();
        if (ls == 0) {
            continue;
        }

        for (auto le : *ls) {
            if (le->score() != this) {
                continue;
            }

            Staff* linkedMasterStaff = toStaff(le);
            if (mu::contains(assignedStavesIds, linkedMasterStaff->id())) {
                continue;
            }

            Part* excerptPart = excerptStaff->part();
            Part* masterPart = linkedMasterStaff->part();

            //! NOTE: parts/staves of excerpt must have the same ID as parts/staves of the master score
            //! In fact, excerpts are just viewers for the master score
            excerptStaff->setId(linkedMasterStaff->id());
            excerptPart->setId(masterPart->id());

            assignedStavesIds.insert(linkedMasterStaff->id());

            // For instruments with multiple staves, every staff will point to the
            // same part. To prevent adding the same part several times to the excerpt,
            // add only the part of the first staff pointing to the part.
            if (!(--nstaves)) {
                ex->parts().push_back(linkedMasterStaff->part());
                nstaves = linkedMasterStaff->part()->nstaves();
            }
            break;
        }
    }

    if (ex->tracksMapping().empty()) {   // SHOULDN'T HAPPEN, protected in the UI, but it happens during read-in!!!
        ex->updateTracksMapping();
    }

    excerpts().insert(excerpts().begin() + (index < 0 ? excerpts().size() : index), ex);
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
        qDebug("removeExcerpt:: ex not found");
    }
}

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

MasterScore* MasterScore::clone()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    WriteContext writeCtx;
    XmlWriter xml(this, &buffer);
    xml.setContext(&writeCtx);
    xml.writeHeader();

    xml.startObject("museScore version=\"" MSC_VERSION "\"");

    compat::WriteScoreHook hook;
    write(xml, false, hook);
    xml.endObject();

    buffer.close();

    QByteArray scoreData = buffer.buffer();
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
        emit s->posChanged(pos, unsigned(tick.ticks()));
    }
}

//---------------------------------------------------------
//   setSoloMute
//   called once at opening file, adds soloMute marks
//---------------------------------------------------------

void MasterScore::setSoloMute()
{
    for (unsigned i = 0; i < _midiMapping.size(); i++) {
        Channel* b = _midiMapping[i].articulation();
        if (b->solo()) {
            b->setSoloMute(false);
            for (unsigned j = 0; j < _midiMapping.size(); j++) {
                Channel* a = _midiMapping[j].articulation();
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
            for (Channel* ch : instr->channel()) {
                Channel* pChannel = playbackChannel(ch);
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
    SynthesizerGroup g = s.group("master");

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
        SynthesizerGroup g = s.group("master");

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
