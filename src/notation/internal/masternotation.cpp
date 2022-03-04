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

static ExcerptNotation* get_impl(const IExcerptNotationPtr& excerpt)
{
    return static_cast<ExcerptNotation*>(excerpt.get());
}

static IExcerptNotationPtr createExcerptNotation(Ms::Excerpt* excerpt)
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt);
    excerptNotation->setIsCreated(true);

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
        updateExerpts();
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

void MasterNotation::setMasterScore(Ms::MasterScore* score)
{
    if (masterScore() == score) {
        return;
    }

    TRACEFUNC;

    setScore(score);
    score->setSystemObjectStaves();
    m_notationPlayback->init(m_undoStack);
    initExcerptNotations(masterScore()->excerpts());
}

Ms::MasterScore* MasterNotation::masterScore() const
{
    return dynamic_cast<Ms::MasterScore*>(score());
}

static void clearMeasures(Ms::Score* score)
{
    for (Ms::Score* _score : score->scoreList()) {
        Ms::MeasureBaseList* measures = _score->measures();

        for (Ms::MeasureBase* measure = measures->first(); measure; measure = measure->next()) {
            measure->deleteLater();
        }

        measures->clear();
    }
}

static void createMeasures(Ms::Score* score, const ScoreCreateOptions& scoreOptions)
{
    TRACEFUNC;

    Ms::Fraction timesig(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);
    score->sigmap()->add(0, timesig);
    bool pickupMeasure = scoreOptions.withPickupMeasure;
    int measures = scoreOptions.measures;
    if (pickupMeasure) {
        measures += 1;
    }
    Ms::Fraction firstMeasureTicks = pickupMeasure ? Ms::Fraction(scoreOptions.measureTimesigNumerator,
                                                                  scoreOptions.measureTimesigDenominator) : timesig;

    Ms::KeySigEvent ks;
    ks.setKey(scoreOptions.key);

    for (int i = 0; i < measures; ++i) {
        Ms::Fraction tick = firstMeasureTicks + timesig * (i - 1);
        if (i == 0) {
            tick = Ms::Fraction(0, 1);
        }
        QList<Ms::Rest*> puRests;
        for (Ms::Score* _score : score->scoreList()) {
            Ms::Rest* rest = 0;
            Ms::Measure* measure = mu::engraving::Factory::createMeasure(_score->dummy()->system());
            measure->setTimesig(timesig);
            measure->setTicks(timesig);
            measure->setTick(tick);

            if (pickupMeasure && tick.isZero()) {
                measure->setIrregular(true);                // don’t count pickup measure
                measure->setTicks(Ms::Fraction(scoreOptions.measureTimesigNumerator,
                                               scoreOptions.measureTimesigDenominator));
            }
            _score->measures()->add(measure);

            for (Ms::Staff* staff : _score->staves()) {
                int staffIdx = staff->idx();
                if (tick.isZero()) {
                    Ms::Measure* m = _score->firstMeasure();
                    Ms::Segment* s = m->getSegment(Ms::SegmentType::TimeSig, Ms::Fraction(0, 1));
                    Ms::TimeSig* ts = mu::engraving::Factory::createTimeSig(s);
                    ts->setTrack(staffIdx * Ms::VOICES);
                    ts->setSig(timesig, scoreOptions.timesigType);
                    s->add(ts);
                    Part* part = staff->part();
                    if (!part->instrument()->useDrumset()) {
                        //
                        // transpose key
                        //
                        Ms::KeySigEvent nKey = ks;
                        if (!nKey.custom() && !nKey.isAtonal() && part->instrument()->transpose().chromatic
                            && !score->styleB(Ms::Sid::concertPitch)) {
                            int diff = -part->instrument()->transpose().chromatic;
                            nKey.setKey(Ms::transposeKey(nKey.key(), diff, part->preferSharpFlat()));
                        }
                        // do not create empty keysig unless custom or atonal
                        if (nKey.custom() || nKey.isAtonal() || nKey.key() != Key::C) {
                            staff->setKey(Ms::Fraction(0, 1), nKey);
                            Ms::Segment* ss = measure->getSegment(Ms::SegmentType::KeySig, Ms::Fraction(0, 1));
                            Ms::KeySig* keysig = mu::engraving::Factory::createKeySig(ss);
                            keysig->setTrack(staffIdx * Ms::VOICES);
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
                    std::vector<Ms::TDuration> dList = Ms::toDurationList(measure->ticks(), false);
                    if (!dList.empty()) {
                        Ms::Fraction ltick = tick;
                        int k = 0;
                        foreach (Ms::TDuration d, dList) {
                            Ms::Segment* seg = measure->getSegment(Ms::SegmentType::ChordRest, ltick);
                            if (k < puRests.count()) {
                                rest = static_cast<Ms::Rest*>(puRests[k]->linkedClone());
                            } else {
                                rest = mu::engraving::Factory::createRest(seg, d);
                                puRests.append(rest);
                            }
                            rest->setScore(_score);
                            rest->setTicks(d.fraction());
                            rest->setTrack(staffIdx * Ms::VOICES);
                            seg->add(rest);
                            ltick += rest->actualTicks();
                            k++;
                        }
                    }
                } else {
                    Ms::Segment* seg = measure->getSegment(Ms::SegmentType::ChordRest, tick);
                    if (linkedToPrevious && rest) {
                        rest = static_cast<Ms::Rest*>(rest->linkedClone());
                    } else {
                        rest = mu::engraving::Factory::createRest(seg, Ms::TDuration(Ms::DurationType::V_MEASURE));
                    }
                    rest->setScore(_score);
                    rest->setTicks(measure->ticks());
                    rest->setTrack(staffIdx * Ms::VOICES);
                    seg->add(rest);
                }
            }
        }
    }
}

//! NOTE: this method with all of its dependencies was copied from MU3
//! source: file.cpp, MuseScore::getNewFile()
mu::Ret MasterNotation::setupNewScore(Ms::MasterScore* score, const ScoreCreateOptions& scoreOptions)
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

void MasterNotation::applyOptions(Ms::MasterScore* score, const ScoreCreateOptions& scoreOptions, bool createdFromTemplate)
{
    TRACEFUNC;

    Ms::VBox* nvb = nullptr;

    if (createdFromTemplate) {
        clearMeasures(score);

        Ms::MeasureBase* mb = score->first();
        if (mb && mb->isVBox()) {
            Ms::VBox* tvb = toVBox(mb);
            nvb = new Ms::VBox(score->dummy()->system());
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
        score->setSystemObjectStaves(); // use the template to determine where system objects go
    }

    score->setSaved(true);
    score->setNewlyCreated(true);

    score->checkChordList();

    createMeasures(score, scoreOptions);

    //
    // select first rest
    //
    Measure* m = score->firstMeasure();
    for (Ms::Segment* s = m->first(); s; s = s->next()) {
        if (s->segmentType() == Ms::SegmentType::ChordRest) {
            if (s->element(0)) {
                score->select(s->element(0), SelectType::SINGLE, 0);
                break;
            }
        }
    }

    {
        QString title = score->metaTag("workTitle");
        QString subtitle = score->metaTag("subtitle");
        QString composer = score->metaTag("composer");
        QString lyricist = score->metaTag("lyricist");

        if (!title.isEmpty() || !subtitle.isEmpty() || !composer.isEmpty() || !lyricist.isEmpty()) {
            Ms::MeasureBase* measure = score->measures()->first();
            if (measure->type() != ElementType::VBOX) {
                Ms::MeasureBase* nm = nvb ? nvb : new Ms::VBox(score->dummy()->system());
                nm->setTick(Ms::Fraction(0, 1));
                nm->setNext(measure);
                score->measures()->add(nm);
                measure = nm;
            } else if (nvb) {
                delete nvb;
            }

            auto setText = [score](Ms::TextStyleType textItemId, const QString& text) {
                Ms::TextBase* textItem = score->getText(textItemId);

                if (!textItem) {
                    textItem = score->addText(textItemId);
                }

                if (textItem) {
                    textItem->setPlainText(text);
                }
            };

            if (!title.isEmpty()) {
                setText(Ms::TextStyleType::TITLE, title);
            }
            if (!subtitle.isEmpty()) {
                setText(Ms::TextStyleType::SUBTITLE, subtitle);
            }
            if (!composer.isEmpty()) {
                setText(Ms::TextStyleType::COMPOSER, composer);
            }
            if (!lyricist.isEmpty()) {
                setText(Ms::TextStyleType::POET, lyricist);
            }
        } else if (nvb) {
            delete nvb;
        }
    }

    if (scoreOptions.withTempo) {
        Ms::Fraction ts(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);

        QString text("<sym>metNoteQuarterUp</sym> = %1");
        double bpm = scoreOptions.tempo.valueBpm;
        switch (ts.denominator()) {
        case 1:
            bpm /= 4;
            break;
        case 2:
            bpm /= 2;
            break;
        case 4:
            break;
        case 8:
            if (ts.numerator() % 3 == 0) {
                bpm /= 1.5;
            } else {
                bpm *= 2;
            }
            break;
        case 16:
            if (ts.numerator() % 3 == 0) {
                bpm *= 1.5;
            } else {
                bpm *= 4;
            }
            break;
        case 32:
            if (ts.numerator() % 3 == 0) {
                bpm *= 3;
            } else {
                bpm *= 8;
            }
            break;
        case 64:
            if (ts.numerator() % 3 == 0) {
                bpm *= 6;
            } else {
                bpm *= 16;
            }
            break;
        case 128:
            if (ts.numerator() % 3 == 0) {
                bpm *= 6;
            } else {
                bpm *= 16;
            }
            break;
        default:
            break;
        }

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

        Ms::Segment* seg = score->firstMeasure()->first(Ms::SegmentType::ChordRest);
        Ms::TempoText* tt = new Ms::TempoText(seg);
        tt->setXmlText(text.arg(bpm));

        double tempo = scoreOptions.tempo.valueBpm;
        tempo /= 60; // bpm -> bps

        tt->setTempo(tempo);
        tt->setFollowText(true);
        tt->setTrack(0);
        seg->add(tt);
    }

    score->setUpTempoMap();

    {
        Ms::ScoreLoad sl;
        score->doLayout();
    }
}

bool MasterNotation::isNewlyCreated() const
{
    IF_ASSERT_FAILED(masterScore()) {
        return true;
    }

    return masterScore()->isNewlyCreated();
}

mu::ValNt<bool> MasterNotation::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = masterScore() ? !masterScore()->saved() : false;
    needSave.notification = m_needSaveNotification;

    return needSave;
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

        if (!excerptNotationImpl->isCreated()) {
            masterScore()->initAndAddExcerpt(excerptNotationImpl->excerpt(), false);
            excerptNotationImpl->setIsCreated(true);
        }

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

        Ms::Excerpt* excerpt = get_impl(excerptNotation)->excerpt();
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
            updateExerpts();
            notifyAboutNeedSaveChanged();
        });
    }
}

void MasterNotation::updateExerpts()
{
    if (!masterScore()->excerptsChanged()) {
        return;
    }

    TRACEFUNC;

    ExcerptNotationList updatedExcerpts;

    const QList<Ms::Excerpt*>& excerpts = masterScore()->excerpts();

    // exclude notations for old excerpts
    for (IExcerptNotationPtr excerptNotation : m_excerpts.val) {
        ExcerptNotation* impl = get_impl(excerptNotation);

        if (excerpts.contains(impl->excerpt())) {
            updatedExcerpts.push_back(excerptNotation);
            continue;
        }

        impl->setIsCreated(false);
        impl->setIsOpen(false);
    }

    // create notations for new excerpts
    for (Ms::Excerpt* excerpt : excerpts) {
        if (containsExcerpt(excerpt)) {
            continue;
        }

        IExcerptNotationPtr excerptNotation = createExcerptNotation(excerpt);
        excerptNotation->notation()->setIsOpen(true);

        updatedExcerpts.push_back(excerptNotation);
    }

    doSetExcerpts(updatedExcerpts);

    masterScore()->setExcerptsChanged(false);
}

bool MasterNotation::containsExcerpt(const Ms::Excerpt* excerpt) const
{
    for (IExcerptNotationPtr excerptNotation : m_excerpts.val) {
        if (get_impl(excerptNotation)->excerpt() == excerpt) {
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

IExcerptNotationPtr MasterNotation::newExcerptBlankNotation() const
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(new Ms::Excerpt(masterScore()));
    excerptNotation->setName(qtrc("notation", "Part"));
    excerptNotation->setIsCreated(false);

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

ExcerptNotationList MasterNotation::potentialExcerpts() const
{
    TRACEFUNC;

    auto excerptExists = [this](const ID& partId) {
        for (const Ms::Excerpt* excerpt : masterScore()->excerpts()) {
            const QList<Part*>& excerptParts = excerpt->parts();

            if (excerptParts.size() != 1) {
                continue;
            }

            if (ID(excerptParts.first()->id()) == partId) {
                return true;
            }
        }

        return false;
    };

    QList<Part*> parts;
    for (Part* part : score()->parts()) {
        if (!excerptExists(part->id())) {
            parts << part;
        }
    }

    QList<Ms::Excerpt*> excerpts = Ms::Excerpt::createExcerptsFromParts(parts);

    ExcerptNotationList result;

    for (Ms::Excerpt* excerpt : excerpts) {
        auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt);
        excerptNotation->setIsCreated(false);
        result.push_back(excerptNotation);
    }

    return result;
}

void MasterNotation::initExcerptNotations(const QList<Ms::Excerpt*>& excerpts)
{
    TRACEFUNC;

    ExcerptNotationList notationExcerpts;

    for (Ms::Excerpt* excerpt : excerpts) {
        if (excerpt->isEmpty()) {
            masterScore()->initEmptyExcerpt(excerpt);
        }

        IExcerptNotationPtr excerptNotation = createExcerptNotation(excerpt);
        notationExcerpts.push_back(excerptNotation);
    }

    masterScore()->setExcerptsChanged(false);

    doSetExcerpts(notationExcerpts);
}

void MasterNotation::addExcerptsToMasterScore(const QList<Ms::Excerpt*>& excerpts)
{
    TRACEFUNC;

    for (Ms::Excerpt* excerpt : excerpts) {
        masterScore()->initAndAddExcerpt(excerpt, false);
    }

    masterScore()->setExcerptsChanged(false);
}
