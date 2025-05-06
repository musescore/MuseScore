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
#include "masternotation.h"

#include <cmath>
#include <QFileInfo>

#include "log.h"
#include "translation.h"

#include "engraving/style/defaultstyle.h"
#include "engraving/style/pagestyle.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/box.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/undo.h"

#include "excerptnotation.h"
#include "masternotationparts.h"

#ifdef MUE_BUILD_ENGRAVING_PLAYBACK
#include "notationplayback.h"
#else
#include "notationplaybackstub.h"
#endif

#include "../notationerrors.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::async;

static ExcerptNotation* get_impl(const IExcerptNotationPtr& excerpt)
{
    return static_cast<ExcerptNotation*>(excerpt.get());
}

static IExcerptNotationPtr createAndInitExcerptNotation(mu::engraving::Excerpt* excerpt, const muse::modularity::ContextPtr& iocCtx)
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt, iocCtx);
    excerptNotation->init();

    return excerptNotation;
}

MasterNotation::MasterNotation(const muse::modularity::ContextPtr& iocCtx)
    : Notation(iocCtx)
{
    m_parts = std::make_shared<MasterNotationParts>(this, interaction(), undoStack());

#ifdef MUE_BUILD_ENGRAVING_PLAYBACK
    m_notationPlayback = std::make_shared<NotationPlayback>(this, m_notationChanged, iocCtx);
#else
    m_notationPlayback = std::make_shared<NotationPlaybackStub>();
#endif

    m_parts->partsChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    async::NotifyList<const Part*> partList = m_parts->partList();

    partList.onChanged(this, [this]() {
        onPartsChanged();
    });

    partList.onItemAdded(this, [this](const Part*) {
        onPartsChanged();
    });

    partList.onItemRemoved(this, [this](const Part*) {
        onPartsChanged();
    });
}

MasterNotation::~MasterNotation()
{
    m_parts = nullptr;

    unloadExcerpts(m_potentialExcerpts);
}

int MasterNotation::mscVersion() const
{
    if (!masterScore()) {
        return 0;
    }

    return masterScore()->mscVersion();
}

INotationPtr MasterNotation::notation()
{
    return shared_from_this();
}

void MasterNotation::initAfterSettingScore(const MasterScore* score)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TRACEFUNC;

    score->changesChannel().onReceive(this, [this](const ScoreChangesRange&) {
        updateExcerpts();
    });

    m_notationPlayback->init();
    initExcerptNotations(score->excerpts());
}

void MasterNotation::setMasterScore(mu::engraving::MasterScore* score)
{
    if (masterScore() == score) {
        return;
    }

    TRACEFUNC;

    setScore(score);

    score->updateSwing();
    score->updateCapo();

    initAfterSettingScore(score);
}

mu::engraving::MasterScore* MasterNotation::masterScore() const
{
    return dynamic_cast<mu::engraving::MasterScore*>(score());
}

static void clearMeasures(mu::engraving::MasterScore* masterScore)
{
    TRACEFUNC;

    for (mu::engraving::Score* score : masterScore->scoreList()) {
        for (Part* part : score->parts()) {
            part->removeNonPrimaryInstruments();
        }

        mu::engraving::MeasureBaseList* measures = score->measures();
        for (mu::engraving::MeasureBase* measure = measures->first(); measure; measure = measure->next()) {
            measure->deleteLater();
        }

        auto spanners = score->spanner();
        for (auto spanner = spanners.begin(); spanner != spanners.end(); spanner = ++spanner) {
            score->removeSpanner(spanner->second);
        }

        measures->clear();
    }

    masterScore->setPlaylistDirty();
    masterScore->updateRepeatList();
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
        ks.setConcertKey(scoreOptions.key);
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
            _score->measures()->append(measure);

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
                    mu::engraving::KeySigEvent nKey;
                    // use atonal keysig for drums
                    if (part->instrument()->useDrumset()) {
                        nKey.setConcertKey(Key::C);
                        nKey.setCustom(true);
                        nKey.setMode(KeyMode::NONE);
                    } else {
                        //
                        // transpose key
                        //
                        nKey = ks;
                        mu::engraving::Interval v = part->instrument()->transpose();
                        if (!nKey.isAtonal() && !v.isZero() && !score->style().styleB(mu::engraving::Sid::concertPitch)) {
                            v.flip();
                            nKey.setKey(mu::engraving::transposeKey(nKey.concertKey(), v, part->preferSharpFlat()));
                        }
                    }
                    staff->setKey(mu::engraving::Fraction(0, 1), nKey);
                    mu::engraving::Segment* ss
                        = measure->getSegment(mu::engraving::SegmentType::KeySig, mu::engraving::Fraction(0, 1));
                    mu::engraving::KeySig* keysig = mu::engraving::Factory::createKeySig(ss);
                    keysig->setTrack(staffIdx * mu::engraving::VOICES);
                    keysig->setKeySigEvent(nKey);
                    ss->add(keysig);
                }

                // determined if this staff is linked to previous so we can reuse rests
                bool linkedToPrevious = staffIdx && staff->isLinked(_score->staff(staffIdx - 1));
                if (measure->timesig() != measure->ticks()) {
                    if (!linkedToPrevious) {
                        puRests.clear();
                    }
                    std::vector<mu::engraving::TDuration> dList = mu::engraving::toRhythmicDurationList(
                        measure->ticks(), true, mu::engraving::Fraction(0, 1),
                        measure->score()->sigmap()->timesig(measure->tick().ticks()).nominal(), measure, 0);
                    if (!dList.empty()) {
                        mu::engraving::Fraction ltick = tick;
                        int k = 0;
                        for (mu::engraving::TDuration d : dList) {
                            mu::engraving::Segment* seg = measure->getSegment(mu::engraving::SegmentType::ChordRest, ltick);
                            if (k < puRests.count()) {
                                rest = static_cast<mu::engraving::Rest*>(puRests[k]->linkedClone());
                            } else {
                                rest = mu::engraving::Factory::createRest(seg, d);
                                puRests.append(rest);
                            }
                            rest->setScore(_score);
                            rest->setTicks(d.isMeasure() ? measure->ticks() : d.fraction());
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
Ret MasterNotation::setupNewScore(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions)
{
    TRACEFUNC;

    setScore(score);

    undoStack()->lock();

    parts()->setParts(scoreOptions.parts, scoreOptions.order);

    score->checkChordList();
    score->updateSwing();
    score->updateCapo();

    applyOptions(score, scoreOptions);

    initAfterSettingScore(score);
    addExcerptsToMasterScore(score->excerpts());

    undoStack()->unlock();

    return make_ret(Err::NoError);
}

void MasterNotation::applyOptions(mu::engraving::MasterScore* score, const ScoreCreateOptions& scoreOptions, bool createdFromTemplate)
{
    TRACEFUNC;

    mu::engraving::VBox* nvb = nullptr;

    if (createdFromTemplate) {
        mu::engraving::MeasureBase* mb = score->first();
        if (mb && mb->isVBox()) {
            mu::engraving::VBox* tvb = toVBox(mb);
            nvb = Factory::createTitleVBox(score->dummy()->system());
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

        clearMeasures(score);

        // for templates using built-in base page style, set score page style to default (may be user-defined)
        bool isBaseWidth = (std::abs(score->style().styleD(Sid::pageWidth) - DefaultStyle::baseStyle().styleD(Sid::pageWidth)) < 0.1);
        bool isBaseHeight = (std::abs(score->style().styleD(Sid::pageHeight) - DefaultStyle::baseStyle().styleD(Sid::pageHeight)) < 0.1);
        if (isBaseWidth && isBaseHeight) {
            for (auto st : pageStyles()) {
                score->style().set(st, DefaultStyle::defaultStyle().value(st));
            }
        }
    }

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
                mu::engraving::MeasureBase* nm = nvb ? nvb : Factory::createTitleVBox(score->dummy()->system());
                nm->setTick(mu::engraving::Fraction(0, 1));
                nm->setExcludeFromOtherParts(false);
                nm->setNext(measure);
                score->measures()->add(nm);
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
                setText(mu::engraving::TextStyleType::LYRICIST, lyricist);
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
        for (auto staff : score->systemObjectStaves()) {
            TempoText* linkedTt = toTempoText(tt->linkedClone());
            linkedTt->setScore(score);
            linkedTt->setTrack(staff->idx() * VOICES);
            seg->add(linkedTt);
        }
    }

    score->setUpTempoMap();
    score->autoUpdateSpatium();

    {
        mu::engraving::ScoreLoad sl;
        for (mu::engraving::Score* s : score->scoreList()) {
            s->doLayout();
        }
    }
}

void MasterNotation::unloadExcerpts(ExcerptNotationList& excerpts)
{
    for (const IExcerptNotationPtr& ptr : excerpts) {
        Excerpt* excerpt = get_impl(ptr)->excerpt();
        if (!excerpt) {
            continue;
        }

        if (Score* score = excerpt->excerptScore()) {
            delete score;
            excerpt->setExcerptScore(nullptr);
        }
    }

    excerpts.clear();
}

void MasterNotation::initExcerpts(const ExcerptNotationList& excerpts)
{
    for (const IExcerptNotationPtr& excerptNotation : excerpts) {
        ExcerptNotation* impl = get_impl(excerptNotation);

        masterScore()->initExcerpt(impl->excerpt());
        impl->init();
    }
}

void MasterNotation::setExcerpts(const ExcerptNotationList& excerpts)
{
    TRACEFUNC;

    if (m_excerpts == excerpts) {
        return;
    }

    MasterScore* score = masterScore();
    IF_ASSERT_FAILED(score) {
        return;
    }

    undoStack()->prepareChanges(TranslatableString("undoableAction", "Add/remove parts"));

    // Delete old excerpts (that are not included in the new list)
    for (const IExcerptNotationPtr& excerptNotation : m_excerpts) {
        auto it = std::find(excerpts.begin(), excerpts.end(), excerptNotation);
        if (it != excerpts.end()) {
            continue;
        }

        score->deleteExcerpt(get_impl(excerptNotation)->excerpt());
    }

    // Init new excerpts
    for (size_t i = 0; i < excerpts.size(); ++i) {
        const IExcerptNotationPtr& excerptNotation = excerpts.at(i);
        ExcerptNotation* excerptNotationImpl = get_impl(excerptNotation);

        auto it = std::find(m_excerpts.cbegin(), m_excerpts.cend(), excerptNotation);
        if (it != m_excerpts.end()) {
            std::vector<Excerpt*>& msExcerpts = score->excerpts();
            muse::moveItem(msExcerpts, muse::indexOf(msExcerpts, excerptNotationImpl->excerpt()), i);
            continue;
        }

        score->initAndAddExcerpt(excerptNotationImpl->excerpt(), false);
        excerptNotationImpl->init();
    }

    score->setExcerptsChanged(false);

    undoStack()->commitChanges();

    doSetExcerpts(excerpts);
}

void MasterNotation::resetExcerpt(IExcerptNotationPtr excerptNotation)
{
    if (!excerptNotation || !excerptNotation->isInited()) {
        return;
    }

    TRACEFUNC;

    undoStack()->prepareChanges(TranslatableString("undoableAction", "Reset part"));

    mu::engraving::Excerpt* oldExcerpt = get_impl(excerptNotation)->excerpt();
    masterScore()->deleteExcerpt(oldExcerpt);

    mu::engraving::Excerpt* newExcerpt = new mu::engraving::Excerpt(*oldExcerpt, false);
    masterScore()->initAndAddExcerpt(newExcerpt, false);

    newExcerpt->excerptScore()->setIsOpen(oldExcerpt->excerptScore()->isOpen());

    get_impl(excerptNotation)->reinit(newExcerpt);

    masterScore()->setExcerptsChanged(false);

    undoStack()->commitChanges();
}

void MasterNotation::sortExcerpts(ExcerptNotationList& excerpts)
{
    TRACEFUNC;

    std::vector<ID> partIdList;
    for (const Part* part : masterScore()->parts()) {
        partIdList.push_back(part->id());
    }

    std::sort(excerpts.begin(), excerpts.end(), [&partIdList](const IExcerptNotationPtr& f, const IExcerptNotationPtr& s) {
        //! NOTE: excertps created by users are always at the end of the list in alphabetical order
        if (f->isCustom() && !s->isCustom()) {
            return false;
        } else if (!f->isCustom() && s->isCustom()) {
            return true;
        } else if (f->isCustom() && s->isCustom()) {
            return f->name() < s->name();
        }

        //! NOTE: sort standard excerpts in the order of their initial parts in the main score
        const ID& initialPart1 = get_impl(f)->excerpt()->initialPartId();
        const ID& initialPart2 = get_impl(s)->excerpt()->initialPartId();

        size_t index1 = muse::indexOf(partIdList, initialPart1);
        size_t index2 = muse::indexOf(partIdList, initialPart2);

        return index1 < index2;
    });
}

void MasterNotation::setExcerptIsOpen(const INotationPtr excerptNotation, bool open)
{
    if (excerptNotation->isOpen() == open) {
        return;
    }

    excerptNotation->setIsOpen(open);

    if (open) {
        excerptNotation->elements()->msScore()->doLayout();
    }
}

void MasterNotation::doSetExcerpts(const ExcerptNotationList& excerpts)
{
    TRACEFUNC;

    m_excerpts = excerpts;
    m_excerptsChanged.notify();

    static_cast<MasterNotationParts*>(m_parts.get())->setExcerpts(excerpts);

    updatePotentialExcerpts();
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
    for (const IExcerptNotationPtr& excerptNotation : m_excerpts) {
        ExcerptNotation* impl = get_impl(excerptNotation);

        if (muse::contains(excerpts, impl->excerpt())) {
            updatedExcerpts.push_back(excerptNotation);
            continue;
        }
    }

    // create notations for new excerpts
    for (mu::engraving::Excerpt* excerpt : excerpts) {
        if (containsExcerpt(excerpt)) {
            continue;
        }

        IExcerptNotationPtr excerptNotation = createAndInitExcerptNotation(excerpt, iocContext());
        bool open = excerpt->excerptScore()->isOpen();
        if (open) {
            excerptNotation->notation()->elements()->msScore()->doLayout();
        }

        updatedExcerpts.push_back(excerptNotation);
    }

    doSetExcerpts(updatedExcerpts);

    masterScore()->setExcerptsChanged(false);
}

void MasterNotation::updatePotentialExcerpts() const
{
    TRACEFUNC;

    auto findExcerptByPart = [](const ExcerptNotationList& excerpts, const Part* part) {
        return std::find_if(excerpts.cbegin(), excerpts.cend(), [part](const IExcerptNotationPtr& notation) {
            return get_impl(notation)->excerpt()->initialPartId() == part->id();
        });
    };

    ExcerptNotationList potentialExcerpts;
    std::vector<Part*> partsWithoutExcerpt;

    for (Part* part : score()->parts()) {
        if (findExcerptByPart(m_excerpts, part) != m_excerpts.end()) {
            continue;
        }

        if (!m_potentialExcerptsForcedDirty) {
            auto it = findExcerptByPart(m_potentialExcerpts, part);
            if (it != m_potentialExcerpts.cend()) {
                potentialExcerpts.push_back(*it);
                continue;
            }
        }

        partsWithoutExcerpt.push_back(part);
    }

    std::vector<mu::engraving::Excerpt*> excerpts = mu::engraving::Excerpt::createExcerptsFromParts(partsWithoutExcerpt, masterScore());

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        auto excerptNotation = std::make_shared<ExcerptNotation>(excerpt, iocContext());
        potentialExcerpts.push_back(excerptNotation);
    }

    m_potentialExcerpts = std::move(potentialExcerpts);
    m_potentialExcerptsForcedDirty = false;
}

bool MasterNotation::containsExcerpt(const mu::engraving::Excerpt* excerpt) const
{
    for (const IExcerptNotationPtr& excerptNotation : m_excerpts) {
        if (get_impl(excerptNotation)->excerpt() == excerpt) {
            return true;
        }
    }

    return false;
}

void MasterNotation::onPartsChanged()
{
    m_hasPartsChanged.notify();
    m_potentialExcerptsForcedDirty = true;
}

IExcerptNotationPtr MasterNotation::createEmptyExcerpt(const QString& name) const
{
    auto excerptNotation = std::make_shared<ExcerptNotation>(new mu::engraving::Excerpt(masterScore()), iocContext());
    excerptNotation->setName(name);

    return excerptNotation;
}

const ExcerptNotationList& MasterNotation::excerpts() const
{
    return m_excerpts;
}

muse::async::Notification MasterNotation::excerptsChanged() const
{
    return m_excerptsChanged;
}

INotationPartsPtr MasterNotation::parts() const
{
    return m_parts;
}

bool MasterNotation::hasParts() const
{
    return m_parts && m_parts->hasParts();
}

Notification MasterNotation::hasPartsChanged() const
{
    return m_hasPartsChanged;
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

void MasterNotation::initExcerptNotations(const std::vector<mu::engraving::Excerpt*>& excerpts)
{
    TRACEFUNC;

    ExcerptNotationList notationExcerpts;

    for (mu::engraving::Excerpt* excerpt : excerpts) {
        if (excerpt->isEmpty()) {
            masterScore()->initEmptyExcerpt(excerpt);
        }

        IExcerptNotationPtr excerptNotation = createAndInitExcerptNotation(excerpt, iocContext());
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
