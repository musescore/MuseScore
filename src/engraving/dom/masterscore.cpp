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

#include "io/buffer.h"

#include "compat/writescorehook.h"
#include "infrastructure/mscwriter.h"

#include "rw/mscloader.h"
#include "rw/xmlreader.h"
#include "rw/rwregister.h"

#include "style/defaultstyle.h"

#include "engravingproject.h"

#include "audio.h"
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
    _expandedRepeatList  = new RepeatList(this);
    _nonExpandedRepeatList = new RepeatList(this);
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

    delete _expandedRepeatList;
    delete _nonExpandedRepeatList;
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

String MasterScore::name() const
{
    return fileInfo()->displayName();
}

//---------------------------------------------------------
//   setPlaylistDirty
//---------------------------------------------------------

void MasterScore::setPlaylistDirty()
{
    _playlistDirty = true;
    _expandedRepeatList->setScoreChanged();
    _nonExpandedRepeatList->setScoreChanged();
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
    _expandedRepeatList->updateTempo();
    _nonExpandedRepeatList->updateTempo();
}

void MasterScore::updateRepeatList()
{
    _expandedRepeatList->update(true);
    _nonExpandedRepeatList->update(false);
}

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

const RepeatList& MasterScore::repeatList() const
{
    if (_expandRepeats) {
        _expandedRepeatList->update(true);
        return *_expandedRepeatList;
    }

    _nonExpandedRepeatList->update(false);
    return *_nonExpandedRepeatList;
}

const RepeatList& MasterScore::repeatList(bool expandRepeats) const
{
    if (expandRepeats) {
        _expandedRepeatList->update(true);
        return *_expandedRepeatList;
    }

    _nonExpandedRepeatList->update(false);
    return *_nonExpandedRepeatList;
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

    rw::RWRegister::writer()->writeScore(this, &buffer, false);

    buffer.close();

    ByteArray scoreData = buffer.data();
    MasterScore* score = new MasterScore(style(), m_project);

    XmlReader r(scoreData);
    MscLoader().readMasterScore(score, r, true);

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

    const Fraction currentTimeSig = sigmap()->timesig(tick.ticks()).nominal();   // use nominal time signature of current measure
    Measure* masterMeasure = nullptr;
    Fraction ticks   = { 0, 1 };

    MeasureBase* result = nullptr;
    const bool isBeginning = tick.isZero();

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

        MeasureBase* newMeasureBase = toMeasureBase(Factory::createMeasure(score->dummy()->system()));
        newMeasureBase->setTick(tick);

        if (score == this) {
            result = newMeasureBase;
        }

        if (actualBeforeMeasure) {
            actualBeforeMeasure = actualBeforeMeasure->top(); // don't try to insert in front of nested frame
        }
        newMeasureBase->setNext(actualBeforeMeasure);
        newMeasureBase->setPrev(actualBeforeMeasure ? actualBeforeMeasure->prev() : score->last());
        if (newMeasureBase->isMeasure()) {
            Measure* m = toMeasure(newMeasureBase);
            m->setTimesig(currentTimeSig);
            m->setTicks(currentTimeSig);
        }
        undo(new InsertMeasures(newMeasureBase, newMeasureBase));

        Measure* newMeasure  = toMeasure(newMeasureBase); // new measure
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
        std::vector<Clef*> clefList;
        std::vector<Clef*> previousClefList;
        std::list<Clef*> specialCaseClefs;
        std::vector<BarLine*> previousBarLinesList;

        Measure* pm = newMeasure->prevMeasure();

        //
        // remove clef, barlines, time and key signatures
        //
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
                            undo(new RemoveElement(pc));
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
                        // if it's a clef, we only add if it's a header clef
                        bool moveClef = s->isHeaderClefType() && e && e->isClef() && !toClef(e)->forInstrumentChange();
                        // otherwise, we only add if e exists and is generated
                        bool moveOther = e && !e->isClef() && (!e->generated() || tick.isZero());
                        bool specialCase = false;
                        if (e && e->isClef() && !moveClef && isBeginning
                            && toClef(e)->clefToBarlinePosition() != ClefToBarlinePosition::AFTER) {
                            // special case:
                            // there is a non-header clef at global tick 0, and we are inserting at the beginning of the score.
                            // this clef will be moved with the measure it accompanies, but it will be moved before the barline.
                            specialCase = true;
                            moveClef = true;
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
                        } else if (e->isTimeSig()) {
                            TimeSig* ts = toTimeSig(e);
                            timeSigList.push_back(ts);
                            ee = e;
                        }
                        if (specialCase) {
                            specialCaseClefs.push_back(toClef(e));
                            ee = e;
                        } else if (tick.isZero() && e->isClef()) {
                            Clef* clef = toClef(e);
                            clefList.push_back(clef);
                            ee = e;
                        }
                        if (ee) {
                            undo(new RemoveElement(ee));
                            if (s->empty()) {
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

        if (pm && !options.moveSignaturesClef) {
            Segment* pbs = pm->findSegment(SegmentType::EndBarLine, tick);
            if (pbs && pbs->enabled()) {
                for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                    EngravingItem* pb = pbs->element(staffIdx * VOICES);
                    if (pb && !pb->generated()) {
                        previousBarLinesList.push_back(toBarLine(pb));
                        undo(new RemoveElement(pb));
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
            undoAddElement(nts);
        }
        for (KeySig* ks : keySigList) {
            KeySig* nks = Factory::copyKeySig(*ks);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
            nks->setParent(s);
            if (!nks->isAtonal()) {
                nks->setKey(nks->concertKey());  // to set correct (transposing) key
            }
            undoAddElement(nks);
        }
        for (Clef* clef : clefList) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::HeaderClef, Fraction(0, 1));
            nClef->setParent(s);
            undoAddElement(nClef);
        }
        for (Clef* clef : previousClefList) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::Clef, newMeasure->ticks());
            nClef->setParent(s);
            undoAddElement(nClef);
        }
        for (Clef* clef : specialCaseClefs) {
            Clef* nClef = Factory::copyClef(*clef);
            Segment* s  = newMeasure->undoGetSegmentR(SegmentType::Clef, newMeasure->ticks());
            nClef->setParent(s);
            undoAddElement(nClef);
        }
        for (BarLine* barLine : previousBarLinesList) {
            BarLine* nBarLine = Factory::copyBarLine(*barLine);
            Segment* s = newMeasure->undoGetSegmentR(SegmentType::EndBarLine, newMeasure->ticks());
            nBarLine->setParent(s);
            undoAddElement(nBarLine);
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

    return result;
}
