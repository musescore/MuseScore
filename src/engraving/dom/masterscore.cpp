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
#include "masterscore.h"

#include "io/buffer.h"

#include "compat/writescorehook.h"

#include "rw/mscloader.h"
#include "rw/xmlreader.h"
#include "rw/rwregister.h"

#include "style/defaultstyle.h"

#include "engravingproject.h"

#include "barline.h"
#include "excerpt.h"
#include "factory.h"
#include "linkedobjects.h"
#include "repeatlist.h"
#include "rest.h"
#include "sig.h"
#include "tempo.h"
#include "timesig.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace muse::io;
using namespace mu::engraving;

//---------------------------------------------------------
//   MasterScore
//---------------------------------------------------------

MasterScore::MasterScore(const muse::modularity::ContextPtr& iocCtx, std::weak_ptr<engraving::EngravingProject> project)
    : Score(iocCtx)
{
    m_project = project;
    m_undoStack   = new UndoStack();
    m_tempomap    = new TempoMap;
    m_sigmap      = new TimeSigMap();
    m_expandedRepeatList  = new RepeatList(this);
    m_nonExpandedRepeatList = new RepeatList(this);
    setMasterScore(this);

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
    metaTags().insert({ u"translator", u"" });
    metaTags().insert({ u"source", u"" });
    metaTags().insert({ u"copyright", u"" });
    metaTags().insert({ u"creationDate", muse::Date::currentDate().toString(muse::DateFormat::ISODate) });
}

MasterScore::MasterScore(const muse::modularity::ContextPtr& iocCtx, const MStyle& s, std::weak_ptr<engraving::EngravingProject> project)
    : MasterScore{iocCtx, project}
{
    setStyle(s);
}

MasterScore::~MasterScore()
{
    if (m_project.lock()) {
        m_project.lock()->m_masterScore = nullptr;
    }

    delete m_expandedRepeatList;
    delete m_nonExpandedRepeatList;
    delete m_sigmap;
    delete m_tempomap;
    delete m_undoStack;
    muse::DeleteAll(m_excerpts);
}

//---------------------------------------------------------
//   setTempomap
//---------------------------------------------------------

void MasterScore::setTempomap(TempoMap* tm)
{
    delete m_tempomap;
    m_tempomap = tm;
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

String MasterScore::name() const
{
    return fileInfo()->displayName();
}

//---------------------------------------------------------
//   setPlaylistDirty
//---------------------------------------------------------

void MasterScore::setPlaylistDirty()
{
    m_playlistDirty = true;
    m_expandedRepeatList->setScoreChanged();
    m_nonExpandedRepeatList->setScoreChanged();
}

//---------------------------------------------------------
//   setExpandRepeats
//---------------------------------------------------------

void MasterScore::setExpandRepeats(bool expand)
{
    if (m_expandRepeats == expand) {
        return;
    }
    m_expandRepeats = expand;
    setPlaylistDirty();
}

//---------------------------------------------------------
//   updateRepeatListTempo
///   needed for usage in Seq::processMessages
//---------------------------------------------------------

void MasterScore::updateRepeatListTempo()
{
    m_expandedRepeatList->updateTempo();
    m_nonExpandedRepeatList->updateTempo();
}

void MasterScore::updateRepeatList()
{
    m_expandedRepeatList->update(true);
    m_nonExpandedRepeatList->update(false);
}

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList() const
{
    if (m_expandRepeats) {
        m_expandedRepeatList->update(true);
        return *m_expandedRepeatList;
    }

    m_nonExpandedRepeatList->update(false);
    return *m_nonExpandedRepeatList;
}

const RepeatList& MasterScore::repeatList(bool expandRepeats, bool updateTies) const
{
    if (expandRepeats) {
        m_expandedRepeatList->update(true, updateTies);
        return *m_expandedRepeatList;
    }

    m_nonExpandedRepeatList->update(false, updateTies);
    return *m_nonExpandedRepeatList;
}

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void MasterScore::addExcerpt(Excerpt* ex, size_t index)
{
    if (!ex->inited()) {
        initParts(ex);
    }

    excerpts().insert(excerpts().begin() + (index == muse::nidx ? excerpts().size() : index), ex);
    setExcerptsChanged(true);
}

//---------------------------------------------------------
//   removeExcerpt
//---------------------------------------------------------

void MasterScore::removeExcerpt(Excerpt* ex)
{
    if (muse::remove(excerpts(), ex)) {
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

    rw::RWRegister::writer(iocContext())->writeScore(this, &buffer, false);

    buffer.close();

    muse::ByteArray scoreData = buffer.data();
    MasterScore* score = new MasterScore(iocContext(), style(), m_project);

    XmlReader r(scoreData);
    MscLoader().readMasterScore(score, r, true);

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

Fraction MasterScore::loopBoundaryTick(LoopBoundaryType type) const
{
    IF_ASSERT_FAILED(type != LoopBoundaryType::Unknown) {
        return Fraction();
    }
    return m_loopBoundaries[size_t(type)];
}

void MasterScore::setLoopBoundaryTick(LoopBoundaryType type, Fraction tick)
{
    IF_ASSERT_FAILED(type != LoopBoundaryType::Unknown) {
        return;
    }
    if (tick < Fraction(0, 1)) {
        tick = Fraction(0, 1);
    }
    if (tick > lastMeasure()->endTick()) {
        // End Reverb may last longer than written notation, but cursor position should not
        tick = lastMeasure()->endTick();
    }

    m_loopBoundaries[size_t(type)] = tick;
    // even though tick position might not have changed, layout might have
    // so we should update cursor here
    // however, we must be careful not to call setLoopBoundaryTick() again while handling posChanged, or recursion results
    for (Score* s : scoreList()) {
        s->notifyLoopBoundaryTickChanged(type, unsigned(tick.ticks()));
    }
}

//---------------------------------------------------------
//   setUpdateAll
//---------------------------------------------------------

void MasterScore::setUpdateAll()
{
    m_cmdState.setUpdateMode(UpdateMode::UpdateAll);
}

//---------------------------------------------------------
//   setLayoutAll
//---------------------------------------------------------

void MasterScore::setLayoutAll(staff_idx_t staff, const EngravingItem* e)
{
    m_cmdState.setTick(Fraction(0, 1));
    m_cmdState.setTick(measures()->last() ? measures()->last()->endTick() : Fraction(0, 1));

    if (e && e->score() == this) {
        // TODO: map staff number properly
        const staff_idx_t startStaff = staff == muse::nidx ? 0 : staff;
        const staff_idx_t endStaff = staff == muse::nidx ? (nstaves() - 1) : staff;
        m_cmdState.setStaff(startStaff);
        m_cmdState.setStaff(endStaff);

        m_cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setLayout
//---------------------------------------------------------

void MasterScore::setLayout(const Fraction& t, staff_idx_t staff, const EngravingItem* e)
{
    if (t >= Fraction(0, 1)) {
        m_cmdState.setTick(t);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        m_cmdState.setStaff(staff);
        m_cmdState.setElement(e);
    }
}

void MasterScore::setLayout(const Fraction& tick1, const Fraction& tick2, staff_idx_t staff1, staff_idx_t staff2, const EngravingItem* e)
{
    if (tick1 >= Fraction(0, 1)) {
        m_cmdState.setTick(tick1);
    }
    if (tick2 >= Fraction(0, 1)) {
        m_cmdState.setTick(tick2);
    }

    if (e && e->score() == this) {
        // TODO: map staff number properly
        m_cmdState.setStaff(staff1);
        m_cmdState.setStaff(staff2);

        m_cmdState.setElement(e);
    }
}

//---------------------------------------------------------
//   setPlaybackScore
//---------------------------------------------------------

void MasterScore::setPlaybackScore(Score* score)
{
    if (m_playbackScore == score) {
        return;
    }

    m_playbackScore = score;
    m_playbackSettingsLinks.clear();

    if (!m_playbackScore) {
        return;
    }

    for (Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            Instrument* instr = pair.second;
            for (InstrChannel* ch : instr->channel()) {
                InstrChannel* pChannel = playbackChannel(ch);
                IF_ASSERT_FAILED(pChannel) {
                    continue;
                }
                m_playbackSettingsLinks.emplace_back(pChannel, ch, /* excerpt */ true);
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

//---------------------------------------------------------
//   insertMeasure
//    Create a new MeasureBase of Measure type and insert
//    before measure.
//    If measure is zero, append new MeasureBase.
//---------------------------------------------------------

MeasureBase* MasterScore::insertMeasure(MeasureBase* beforeMeasure, const InsertMeasureOptions& options)
{
    Fraction tick;
    if (beforeMeasure) {
        if (beforeMeasure->isMeasure()) {
            if (toMeasure(beforeMeasure)->isMMRest()) {
                beforeMeasure = toMeasure(beforeMeasure)->prev();
                beforeMeasure = beforeMeasure ? beforeMeasure->next() : firstMeasure();
                deselectAll();
            }
            for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                if (toMeasure(beforeMeasure)->isMeasureRepeatGroupWithPrevM(staffIdx)) {
                    MScore::setError(MsError::CANNOT_SPLIT_MEASURE_REPEAT);
                    return nullptr;
                }
            }
        }
        tick = beforeMeasure->tick();
    } else {
        tick = last() ? last()->endTick() : Fraction(0, 1);
    }

    const bool isBeginning = tick.isZero();

    // Use nominal time signature of current or previous measure, depending on whether
    // the time sig from `beforeMeasure` will be moved to the newly created measure
    const Fraction currentTimeSig
        = sigmap()->timesig(!options.moveSignaturesClef && !isBeginning && beforeMeasure && beforeMeasure->prevMeasure()
                            ? beforeMeasure->prevMeasure()->tick()
                            : tick)
          .nominal();

    Measure* masterMeasure = nullptr;
    Fraction ticks = { 0, 1 };

    for (Score* score : scoreList()) {
        MeasureBase* actualBeforeMeasure = nullptr;

        if (beforeMeasure) {
            if (beforeMeasure->score() == score) {
                actualBeforeMeasure = beforeMeasure;
            } else if (!beforeMeasure->isMeasure() && beforeMeasure->links()) {
                for (EngravingObject* m : *beforeMeasure->links()) {
                    if (m && m->isMeasureBase() && m->score() == score) {
                        actualBeforeMeasure = toMeasureBase(m);
                        break;
                    }
                }
            }
            // if beforeMeasure is measure; or if din't find linked frame, use measure
            if (!actualBeforeMeasure) {
                actualBeforeMeasure = score->tick2measure(tick);
            }

            if (!actualBeforeMeasure) {
                LOGD("measure not found");
            }
        }

        Measure* newMeasure = Factory::createMeasure(score->dummy()->system());
        newMeasure->setTick(tick);

        if (actualBeforeMeasure) {
            actualBeforeMeasure = actualBeforeMeasure->top(); // don't try to insert in front of nested frame
        }
        newMeasure->setNext(actualBeforeMeasure);
        newMeasure->setPrev(actualBeforeMeasure ? actualBeforeMeasure->prev() : score->last());

        newMeasure->setTimesig(currentTimeSig);
        newMeasure->setTicks(currentTimeSig);

        undo(new InsertMeasures(newMeasure, newMeasure, options.moveStaffTypeChanges));

        ticks = newMeasure->ticks();
        Measure* measureInsert = nullptr; // insert before
        if (actualBeforeMeasure) {
            if (actualBeforeMeasure->isMeasure()) {
                measureInsert = toMeasure(actualBeforeMeasure);
            } else {
                measureInsert = score->tick2measure(actualBeforeMeasure->tick());
            }
        }

        if (score->isMaster()) {
            masterMeasure = newMeasure;
        }

        std::vector<TimeSig*> timeSigList;
        std::vector<KeySig*> keySigList;
        std::vector<Clef*> initClefList;
        std::vector<Clef*> previousClefList;
        std::list<Clef*> specialCaseClefs;
        std::vector<Clef*> afterBarlineClefs;
        std::vector<BarLine*> previousBarLinesList;

        Measure* pm = newMeasure->prevMeasure();

        //
        // remove clefs, barlines, time and key signatures
        //
        bool headerKeySig = false;
        if (measureInsert) {
            // if inserting before first measure, always preserve clefs and signatures
            // at the begining of the score (move them back)

            if (pm && !options.moveSignaturesClef && !isBeginning) {
                Segment* ps = pm->findSegment(SegmentType::Clef, tick);
                if (ps && ps->enabled()) {
                    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                        EngravingItem* pc = ps->element(staffIdx * VOICES);
                        if (pc) {
                            previousClefList.push_back(toClef(pc));
                            doUndoRemoveElement(pc);
                            if (ps->empty()) {
                                undoRemoveElement(ps);
                            }
                        }
                    }
                }
            }

            if (options.moveSignaturesClef || isBeginning) {
                for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                    for (Segment* s = measureInsert->first(); s && s->rtick().isZero(); s = s->next()) {
                        if (!s->enabled()) {
                            continue;
                        }
                        EngravingItem* e = s->element(staffIdx * VOICES);
                        bool specialCase = false;
                        bool initClef = false;
                        bool moveClef = false;
                        bool moveOther = false;
                        if (e && e->isClef()) {
                            if (s->isHeaderClefType()) {
                                // if it's a header clef, we only add if it's an init clef
                                // (other header clefs are handled by undo(new InsertMeasures())
                                initClef = isBeginning && !toClef(e)->forInstrumentChange();
                                moveClef = initClef;
                            } else {
                                ClefToBarlinePosition clefPos = toClef(e)->clefToBarlinePosition();
                                if (clefPos == ClefToBarlinePosition::AFTER) {
                                    // non header clef at the begining of the measure
                                    moveClef = true;
                                } else if (isBeginning) {
                                    // special case:
                                    // there is a non-header clef at global tick 0, and we are inserting at the beginning of the score.
                                    // this clef will be moved with the measure it accompanies, but it will be moved before the barline.
                                    specialCase = true;
                                    moveClef = true;
                                }
                            }
                        } else {
                            // otherwise, we only add if e is not generated
                            moveOther = e && (!e->generated() || tick.isZero());
                        }
                        if (!moveClef && !moveOther) {
                            continue; // this item will remain with the old measure
                        }
                        // otherwise, this item is moved back to the new measure
                        EngravingItem* ee = 0;
                        if (e->isKeySig()) {
                            KeySig* ks = toKeySig(e);
                            if (ks->forInstrumentChange()) {
                                continue;
                            }
                            keySigList.push_back(ks);
                            // if instrument change on that place, set correct key signature for instrument change
                            bool ic = s->next(SegmentType::ChordRest)->findAnnotation(ElementType::INSTRUMENT_CHANGE,
                                                                                      e->part()->startTrack(),
                                                                                      e->part()->endTrack() - 1);
                            if (ic) {
                                KeySigEvent ke = ks->keySigEvent();
                                ke.setForInstrumentChange(true);
                                undoChangeKeySig(ks->staff(), e->tick(), ke);
                            } else {
                                ee = e;
                            }
                            if (s->header()) {
                                headerKeySig = true;
                            }
                        } else if (e->isTimeSig()) {
                            TimeSig* ts = toTimeSig(e);
                            timeSigList.push_back(ts);
                            ee = e;
                        } else if (e->isClef()) {
                            Clef* clef = toClef(e);
                            if (specialCase) {
                                specialCaseClefs.push_back(clef);
                            } else if (initClef) {
                                initClefList.push_back(clef);
                            } else {
                                afterBarlineClefs.push_back(toClef(e));
                            }
                            ee = e;
                        }
                        if (ee) {
                            doUndoRemoveElement(ee);
                            if (s->empty() && s->isTimeSigType()) {
                                undoRemoveElement(s);
                            }
                        }
                    }
                }

                if (masterMeasure && measureInsert->repeatStart()) {
                    masterMeasure->undoChangeProperty(Pid::REPEAT_START, true);
                    measureInsert->undoChangeProperty(Pid::REPEAT_START, false);
                }
            }
        } else if (!measureInsert && tick == Fraction(0, 1)) {
            // If inserting measure into an empty score, restore default C key signature
            // and 4/4 time signature
            score->restoreInitialKeySigAndTimeSig();
        }

        if (pm && !options.moveSignaturesClef && !options.ignoreBarLines) {
            Segment* pbs = pm->findSegment(SegmentType::EndBarLine, tick);
            if (pbs && pbs->enabled()) {
                for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                    EngravingItem* pb = pbs->element(staffIdx * VOICES);
                    if (pb && !pb->generated()) {
                        previousBarLinesList.push_back(toBarLine(pb));
                        doUndoRemoveElement(pb);
                        if (pbs->empty()) {
                            pbs->setEnabled(false);
                        }
                    }
                }
            }
            if (masterMeasure && pm->repeatEnd()) {
                masterMeasure->undoChangeProperty(Pid::REPEAT_END, true);
                pm->undoChangeProperty(Pid::REPEAT_END, false);
            }
        }

        //
        // move clef, barline, time, key signatures
        //
        for (TimeSig* ts : timeSigList) {
            TimeSig* nts = Factory::copyTimeSig(*ts);
            Segment* s   = newMeasure->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1));
            nts->setParent(s);
            doUndoAddElement(nts);
        }
        for (KeySig* ks : keySigList) {
            KeySig* nks = Factory::copyKeySig(*ks);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
            if (headerKeySig || newMeasure->tick().isZero()) {
                s->setHeader(true);
            }
            nks->setParent(s);
            if (!nks->isAtonal()) {
                nks->setKey(nks->concertKey());  // to set correct (transposing) key
            }
            doUndoAddElement(nks);
        }
        for (Clef* clef : initClefList) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::HeaderClef, Fraction(0, 1));
            s->setHeader(true);
            nClef->setParent(s);
            doUndoAddElement(nClef);
        }
        for (Clef* clef : afterBarlineClefs) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::Clef, Fraction(0, 1));
            s->setHeader(true);
            nClef->setParent(s);
            doUndoAddElement(nClef);
        }
        for (Clef* clef : previousClefList) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::Clef, newMeasure->ticks());
            nClef->setParent(s);
            doUndoAddElement(nClef);
        }
        for (Clef* clef : specialCaseClefs) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::Clef, newMeasure->ticks());
            nClef->setParent(s);
            doUndoAddElement(nClef);
        }
        for (BarLine* barLine : previousBarLinesList) {
            BarLine* nBarLine = Factory::copyBarLine(*barLine);
            Segment* s = newMeasure->undoGetSegmentR(SegmentType::EndBarLine, newMeasure->ticks());
            nBarLine->setParent(s);
            doUndoAddElement(nBarLine);
        }
    }

    undoInsertTime(tick, ticks);

    if (masterMeasure && !options.createEmptyMeasures) {
        //
        // fill measure with rest
        // undoAddCR adds rest to linked staves as well
        for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            size_t track = staffIdx * VOICES;
            Rest* rest = Factory::createRest(dummy()->segment(), TDuration(DurationType::V_MEASURE));
            Fraction timeStretch(staff(staffIdx)->timeStretch(masterMeasure->tick()));
            rest->setTicks(masterMeasure->ticks() * timeStretch);
            rest->setTrack(track);
            undoAddCR(rest, masterMeasure, tick);
        }
    }

    if (options.needDeselectAll) {
        deselectAll();
    }

    return masterMeasure;
}
