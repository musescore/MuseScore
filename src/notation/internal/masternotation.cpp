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
#include "masternotation.h"

#include <QFileInfo>

#include "log.h"
#include "translation.h"

#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/excerpt.h"
#include "libmscore/measure.h"
#include "libmscore/box.h"
#include "libmscore/keysig.h"
#include "libmscore/rest.h"
#include "libmscore/tempotext.h"
#include "libmscore/undo.h"

#include "excerptnotation.h"
#include "masternotationparts.h"
#include "notationplayback.h"
#include "../notationerrors.h"

using namespace mu::notation;
using namespace mu::async;
using namespace mu::engraving;

static ExcerptNotation* get_impl(const IExcerptNotationPtr& excerpt)
{
    return static_cast<ExcerptNotation*>(excerpt.get());
}

static IExcerptNotationPtr createAndInitExcerptNotation(mu::engraving::Excerpt* excerpt)
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt);
    excerptNotation->init();

    return excerptNotation;
}

MasterNotation::MasterNotation()
    : Notation()
{
    m_parts = std::make_shared<MasterNotationParts>(this, interaction(), undoStack());
    m_notationPlayback = std::make_shared<NotationPlayback>(this, m_notationChanged);

    m_parts->partsChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    undoStack()->stackChanged().onNotify(this, [this]() {
        updateExcerpts();
        notifyAboutNeedSaveChanged();
    });
}

MasterNotation::~MasterNotation()
{
    m_parts = nullptr;
}

INotationPtr MasterNotation::notation()
{
    return shared_from_this();
}

void MasterNotation::setMasterScore(mu::engraving::MasterScore* score)
{
    if (masterScore() == score) {
        return;
    }

    TRACEFUNC;

    setScore(score);
    m_notationPlayback->init(m_undoStack);
    initExcerptNotations(masterScore()->excerpts());
}

mu::engraving::MasterScore* MasterNotation::masterScore() const
{
    return dynamic_cast<mu::engraving::MasterScore*>(score());
}

static void clearMeasures(mu::engraving::Score* score)
{
    for (mu::engraving::Score* _score : score->scoreList()) {
        mu::engraving::MeasureBaseList* measures = _score->measures();

        for (mu::engraving::MeasureBase* measure = measures->first(); measure; measure = measure->next()) {
            measure->deleteLater();
        }

        measures->clear();
    }
}

static void createMeasures(mu::engraving::Score* score, const ScoreCreateOptions& scoreOptions)
{
    TRACEFUNC;

    mu::engraving::Fraction timesig(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);
    score->sigmap()->add(0, timesig);
    bool pickupMeasure = scoreOptions.withPickupMeasure;
    int measures = scoreOptions.measures;
    if (pickupMeasure) {
        measures += 1;
    }
    mu::engraving::Fraction firstMeasureTicks = pickupMeasure ? mu::engraving::Fraction(scoreOptions.measureTimesigNumerator,
                                                                                        scoreOptions.measureTimesigDenominator) : timesig;

    KeySigEvent ks;
    if (scoreOptions.key == Key::INVALID) {
        // Make atonal key signature
        ks.setCustom(true);
        ks.setMode(KeyMode::NONE);
    } else {
        ks.setKey(scoreOptions.key);
        ks.setMode(scoreOptions.keyMode);
    }

    for (int i = 0; i < measures; ++i) {
        mu::engraving::Fraction tick = firstMeasureTicks + timesig * (i - 1);
        if (i == 0) {
            tick = mu::engraving::Fraction(0, 1);
        }
        QList<mu::engraving::Rest*> puRests;
        for (mu::engraving::Score* _score : score->scoreList()) {
            mu::engraving::Rest* rest = 0;
            mu::engraving::Measure* measure = mu::engraving::Factory::createMeasure(_score->dummy()->system());
            measure->setTimesig(timesig);
            measure->setTicks(timesig);
            measure->setTick(tick);

            if (pickupMeasure && tick.isZero()) {
                measure->setIrregular(true);                // don’t count pickup measure
                measure->setTicks(mu::engraving::Fraction(scoreOptions.measureTimesigNumerator,
                                                          scoreOptions.measureTimesigDenominator));
            }
            _score->measures()->add(measure);

            for (mu::engraving::Staff* staff : _score->staves()) {
                mu::engraving::staff_idx_t staffIdx = staff->idx();
                if (tick.isZero()) {
                    mu::engraving::Measure* m = _score->firstMeasure();
                    mu::engraving::Segment* s = m->getSegment(mu::engraving::SegmentType::TimeSig, mu::engraving::Fraction(0, 1));
                    mu::engraving::TimeSig* ts = mu::engraving::Factory::createTimeSig(s);
                    ts->setTrack(staffIdx * mu::engraving::VOICES);
                    ts->setSig(timesig, scoreOptions.timesigType);
                    s->add(ts);
                    Part* part = staff->part();
                    if (!part->instrument()->useDrumset()) {
                        //
                        // transpose key
                        //
                        mu::engraving::KeySigEvent nKey = ks;
                        if (!nKey.isAtonal() && part->instrument()->transpose().chromatic
                            && !score->styleB(mu::engraving::Sid::concertPitch)) {
                            int diff = -part->instrument()->transpose().chromatic;
                            nKey.setKey(mu::engraving::transposeKey(nKey.key(), diff, part->preferSharpFlat()));
                        }
                        // do not create empty keysig unless custom or atonal
                        if (nKey.custom() || nKey.isAtonal() || nKey.key() != Key::C) {
                            staff->setKey(mu::engraving::Fraction(0, 1), nKey);
                            mu::engraving::Segment* ss
                                = measure->getSegment(mu::engraving::SegmentType::KeySig, mu::engraving::Fraction(0, 1));
                            mu::engraving::KeySig* keysig = mu::engraving::Factory::createKeySig(ss);
                            keysig->setTrack(staffIdx * mu::engraving::VOICES);
                            keysig->setKeySigEvent(nKey);
                            ss->add(keysig);
                        }
                    }
                }

                // determined if this staff is linked to previous so we can reuse rests
                bool linkedToPrevious = staffIdx && staff->isLinked(_score->staff(staffIdx - 1));
                if (measure->timesig() != measure->ticks()) {
                    if (!linkedToPrevious) {
                        puRests.clear();
                    }
                    std::vector<mu::engraving::TDuration> dList = mu::engraving::toDurationList(measure->ticks(), false);
                    if (!dList.empty()) {
                        mu::engraving::Fraction ltick = tick;
                        int k = 0;
                        foreach (mu::engraving::TDuration d, dList) {
                            mu::engraving::Segment* seg = measure->getSegment(mu::engraving::SegmentType::ChordRest, ltick);
                            if (k < puRests.count()) {
                                rest = static_cast<mu::engraving::Rest*>(puRests[k]->linkedClone());
                            } else {
                                rest = mu::engraving::Factory::createRest(seg, d);
                                puRests.append(rest);
                            }
                            rest->setScore(_score);
                            rest->setTicks(d.fraction());
                            rest->setTrack(staffIdx * mu::engraving::VOICES);
                            seg->add(rest);
                            ltick += rest->actualTicks();
                            k++;
                        }
                    }
                } else {
                    mu::engraving::Segment* seg = measure->getSegment(mu::engraving::SegmentType::ChordRest, tick);
                    if (linkedToPrevious && rest) {
                        rest = static_cast<mu::engraving::Rest*>(rest->linkedClone());
                    } else {
                        rest = mu::engraving::Factory::createRest(seg, mu::engraving::TDuration(mu::engraving::DurationType::V_MEASURE));
                    }
                    rest->setScore(_score);
                    rest->setTicks(measure->ticks());
                    rest->setTrack(staffIdx * mu::engraving::VOICES);
                    seg->add(rest);
                }
            }
        }
    }
}

//! NOTE: this method with all of its dependencies was copied from MU3
//! source: file.cpp, MuseScore::getNewFile()
mu::Ret MasterNotation::setupNewScore(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions)
{
    TRACEFUNC;

    setScore(score);

    parts()->setParts(scoreOptions.parts, scoreOptions.order);

    score->checkChordList();

    applyOptions(score, scoreOptions);

    m_notationPlayback->init(m_undoStack);
    initExcerptNotations(score->excerpts());
    addExcerptsToMasterScore(score->excerpts());

    return make_ret(Err::NoError);
}

void MasterNotation::applyOptions(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions, bool createdFromTemplate)
{
    TRACEFUNC;

    mu::engraving::VBox* nvb = nullptr;

    if (createdFromTemplate) {
        clearMeasures(score);

        mu::engraving::MeasureBase* mb = score->first();
        if (mb && mb->isVBox()) {
            mu::engraving::VBox* tvb = toVBox(mb);
            nvb = new mu::engraving::VBox(score->dummy()->system());
            nvb->setBoxHeight(tvb->boxHeight());
            nvb->setBoxWidth(tvb->boxWidth());
            nvb->setTopGap(tvb->topGap());
            nvb->setBottomGap(tvb->bottomGap());
            nvb->setTopMargin(tvb->topMargin());
            nvb->setBottomMargin(tvb->bottomMargin());
            nvb->setLeftMargin(tvb->leftMargin());
            nvb->setRightMargin(tvb->rightMargin());
            nvb->setAutoSizeEnabled(tvb->isAutoSizeEnabled());
        }
    }

    score->setSaved(true);

    score->checkChordList();

    createMeasures(score, scoreOptions);

    {
        QString title = score->metaTag(u"workTitle");
        QString subtitle = score->metaTag(u"subtitle");
        QString composer = score->metaTag(u"composer");
        QString lyricist = score->metaTag(u"lyricist");

        if (!title.isEmpty() || !subtitle.isEmpty() || !composer.isEmpty() || !lyricist.isEmpty()) {
            mu::engraving::MeasureBase* measure = score->measures()->first();
            if (measure->type() != ElementType::VBOX) {
                mu::engraving::MeasureBase* nm = nvb ? nvb : new mu::engraving::VBox(score->dummy()->system());
                nm->setTick(mu::engraving::Fraction(0, 1));
                nm->setNext(measure);
                score->measures()->add(nm);
                measure = nm;
            } else if (nvb) {
                delete nvb;
            }

            auto setText = [score](mu::engraving::TextStyleType textItemId, const QString& text) {
                mu::engraving::TextBase* textItem = score->getText(textItemId);

                if (!textItem) {
                    textItem = score->addText(textItemId);
                }

                if (textItem) {
                    textItem->setPlainText(text);
                }
            };

            if (!title.isEmpty()) {
                setText(mu::engraving::TextStyleType::TITLE, title);
            }
            if (!subtitle.isEmpty()) {
                setText(mu::engraving::TextStyleType::SUBTITLE, subtitle);
            }
            if (!composer.isEmpty()) {
                setText(mu::engraving::TextStyleType::COMPOSER, composer);
            }
            if (!lyricist.isEmpty()) {
                setText(mu::engraving::TextStyleType::POET, lyricist);
            }
        } else if (nvb) {
            delete nvb;
        }
    }

    if (scoreOptions.withTempo) {
        mu::engraving::Fraction ts(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);

        QString text("<sym>metNoteQuarterUp</sym> = %1");
        double bpm = scoreOptions.tempo.valueBpm;

        bool withDot = scoreOptions.tempo.withDot;
        switch (scoreOptions.tempo.duration) {
        case DurationType::V_WHOLE:
            text = "<sym>metNoteWhole</sym> = %1";
            break;
        case DurationType::V_HALF:
            if (withDot) {
                text = "<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
            } else {
                text = "<sym>metNoteHalfUp</sym> = %1";
            }
            break;
        case DurationType::V_QUARTER:
            if (withDot) {
                text = "<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
            } else {
                text = "<sym>metNoteQuarterUp</sym> = %1";
            }
            break;
        case DurationType::V_EIGHTH:
            if (withDot) {
                text = "<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
            } else {
                text = "<sym>metNote8thUp</sym> = %1";
            }
            break;
        case DurationType::V_16TH:
            if (withDot) {
                text = "<sym>metNote16thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
            } else {
                text = "<sym>metNote16thUp</sym> = %1";
            }
            break;
        default:
            break;
        }

        mu::engraving::Segment* seg = score->firstMeasure()->first(mu::engraving::SegmentType::ChordRest);
        mu::engraving::TempoText* tt = new mu::engraving::TempoText(seg);
        tt->setXmlText(text.arg(bpm));

        double tempo = scoreOptions.tempo.valueBpm;
        tempo /= 60; // bpm -> bps

        tt->setTempo(tempo);
        tt->setFollowText(true);
        tt->setTrack(0);
        seg->add(tt);
    }

    score->setUpTempoMap();
    score->autoUpdateSpatium();

    {
        mu::engraving::ScoreLoad sl;
        score->doLayout();
    }
}

mu::ValNt<bool> MasterNotation::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = masterScore() ? !masterScore()->saved() : false;
    needSave.notification = m_needSaveNotification;

    return needSave;
}

void MasterNotation::initExcerpts(const ExcerptNotationList& excerpts)
{
    for (IExcerptNotationPtr excerptNotation : excerpts) {
        ExcerptNotation* impl = get_impl(excerptNotation);

        masterScore()->initExcerpt(impl->excerpt());
        impl->init();
    }
}

void MasterNotation::addExcerpts(const ExcerptNotationList& excerpts)
{
    if (excerpts.empty()) {
        return;
    }

    TRACEFUNC;

    undoStack()->prepareChanges();

    ExcerptNotationList result = m_excerpts.val;
    for (IExcerptNotationPtr excerptNotation : excerpts) {
        auto it = std::find(result.cbegin(), result.cend(), excerptNotation);
        if (it != result.end()) {
            continue;
        }

        ExcerptNotation* excerptNotationImpl = get_impl(excerptNotation);
        masterScore()->initAndAddExcerpt(excerptNotationImpl->excerpt(), false);
        excerptNotationImpl->init();

        result.push_back(excerptNotation);
    }

    masterScore()->setExcerptsChanged(false);

    undoStack()->commitChanges();

    doSetExcerpts(result);
}

void MasterNotation::removeExcerpts(const ExcerptNotationList& excerpts)
{
    if (excerpts.empty()) {
        return;
    }

    TRACEFUNC;

    undoStack()->prepareChanges();

    for (IExcerptNotationPtr excerptNotation : excerpts) {
        auto it = std::find(m_excerpts.val.begin(), m_excerpts.val.end(), excerptNotation);
        if (it == m_excerpts.val.end()) {
            continue;
        }

        mu::engraving::Excerpt* excerpt = get_impl(excerptNotation)->excerpt();
        masterScore()->deleteExcerpt(excerpt);
        m_excerpts.val.erase(it);
    }

    masterScore()->setExcerptsChanged(false);

    undoStack()->commitChanges();

    doSetExcerpts(m_excerpts.val);
}

void MasterNotation::setExcerptIsOpen(const INotationPtr excerptNotation, bool open)
{
    excerptNotation->setIsOpen(open);

    markScoreAsNeedToSave();
}

void MasterNotation::doSetExcerpts(ExcerptNotationList excerpts)
{
    TRACEFUNC;

    m_excerpts.set(excerpts);
    static_cast<MasterNotationParts*>(m_parts.get())->setExcerpts(excerpts);

    for (auto excerpt : excerpts) {
        excerpt->notation()->undoStack()->stackChanged().onNotify(this, [this]() {
            updateExcerpts();
            notifyAboutNeedSaveChanged();
        });
    }

    m_needUpdatePotentialExcerpts = true;
}

void MasterNotation::updateExcerpts()
{
    if (!masterScore()->excerptsChanged()) {
        return;
    }

    TRACEFUNC;

    ExcerptNotationList updatedExcerpts;

    const std::vector<mu::engraving::Excerpt*>& excerpts = masterScore()->excerpts();

    // exclude notations for old excerpts
    for (IExcerptNotationPtr excerptNotation : m_excerpts.val) {
        ExcerptNotation* impl = get_impl(excerptNotation);

        if (mu::contains(excerpts, impl->excerpt())) {
            updatedExcerpts.push_back(excerptNotation);
            continue;
        }

        impl->setIsOpen(false);
    }

    // create notations for new excerpts
    for (mu::engraving::Excerpt* excerpt : excerpts) {
        if (containsExcerpt(excerpt)) {
            continue;
        }

        IExcerptNotationPtr excerptNotation = createAndInitExcerptNotation(excerpt);
        excerptNotation->notation()->setIsOpen(true);

        updatedExcerpts.push_back(excerptNotation);
    }

    doSetExcerpts(updatedExcerpts);

    masterScore()->setExcerptsChanged(false);
}

bool MasterNotation::containsExcerpt(const mu::engraving::Excerpt* excerpt) const
{
    for (IExcerptNotationPtr excerptNotation : m_excerpts.val) {
        if (get_impl(excerptNotation)->excerpt() == excerpt) {
            return true;
        }
    }

    return false;
}

bool MasterNotation::containsExcerptForPart(const Part* part) const
{
    for (const mu::engraving::Excerpt* excerpt : masterScore()->excerpts()) {
        if (excerpt->initialPartId() == part->id()) {
            return true;
        }
    }

    return false;
}

void MasterNotation::notifyAboutNeedSaveChanged()
{
    m_needSaveNotification.notify();
}

void MasterNotation::markScoreAsNeedToSave()
{
    masterScore()->setSaved(false);
    m_needSaveNotification.notify();
}

IExcerptNotationPtr MasterNotation::createEmptyExcerpt() const
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(new mu::engraving::Excerpt(masterScore()));
    excerptNotation->setName(qtrc("notation", "Part"));

    return excerptNotation;
}

mu::ValCh<ExcerptNotationList> MasterNotation::excerpts() const
{
    return m_excerpts;
}

INotationPartsPtr MasterNotation::parts() const
{
    return m_parts;
}

INotationPlaybackPtr MasterNotation::playback() const
{
    return m_notationPlayback;
}

const ExcerptNotationList& MasterNotation::potentialExcerpts() const
{
    updatePotentialExcerpts();

    return m_potentialExcerpts;
}

void MasterNotation::updatePotentialExcerpts() const
{
    TRACEFUNC;

    static std::vector<Part*> prevParts;

    if (prevParts != score()->parts()) {
        prevParts = score()->parts();
        m_needUpdatePotentialExcerpts = true;
    }

    if (!m_needUpdatePotentialExcerpts) {
        return;
    }

    m_potentialExcerpts.clear();

    std::vector<Part*> parts;
    for (Part* part : score()->parts()) {
        if (!containsExcerptForPart(part)) {
            parts.push_back(part);
        }
    }

    std::vector<mu::engraving::Excerpt*> excerpts = mu::engraving::Excerpt::createExcerptsFromParts(parts);

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt);
        m_potentialExcerpts.push_back(excerptNotation);
    }

    m_needUpdatePotentialExcerpts = false;
}

void MasterNotation::initExcerptNotations(const std::vector<mu::engraving::Excerpt*>& excerpts)
{
    TRACEFUNC;

    ExcerptNotationList notationExcerpts;

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        if (excerpt->isEmpty()) {
            masterScore()->initEmptyExcerpt(excerpt);
        }

        IExcerptNotationPtr excerptNotation = createAndInitExcerptNotation(excerpt);
        notationExcerpts.push_back(excerptNotation);
    }

    masterScore()->setExcerptsChanged(false);

    doSetExcerpts(notationExcerpts);
}

void MasterNotation::addExcerptsToMasterScore(const std::vector<mu::engraving::Excerpt*>& excerpts)
{
    TRACEFUNC;

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        masterScore()->initAndAddExcerpt(excerpt, false);
    }

    masterScore()->setExcerptsChanged(false);
}
