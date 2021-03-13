//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "masternotation.h"
#include "excerptnotation.h"
#include "masternotationparts.h"

#include <QFileInfo>

#include "log.h"
#include "translation.h"

#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/excerpt.h"
#include "libmscore/measure.h"
#include "libmscore/box.h"
#include "libmscore/keysig.h"
#include "libmscore/rest.h"
#include "libmscore/tempotext.h"
#include "libmscore/undo.h"

#include "../notationerrors.h"

using namespace mu::notation;
using namespace mu::async;

static ExcerptNotation* get_impl(const IExcerptNotationPtr& excerpt)
{
    return static_cast<ExcerptNotation*>(excerpt.get());
}

MasterNotation::MasterNotation()
    : Notation()
{
    m_parts = std::make_shared<MasterNotationParts>(this, interaction(), undoStack());
}

INotationPtr MasterNotation::notation()
{
    return shared_from_this();
}

Meta MasterNotation::metaInfo() const
{
    Meta meta = Notation::metaInfo();

    meta.fileName = masterScore()->fileInfo()->fileName();
    meta.filePath = masterScore()->fileInfo()->filePath();
    meta.partsCount = masterScore()->excerpts().count();

    return meta;
}

mu::Ret MasterNotation::load(const io::path& path)
{
    TRACEFUNC;

    std::string syffix = io::syffix(path);

    //! NOTE For "mscz", "mscx" see MsczNotationReader
    //! for others see readers in importexport module
    auto reader = readers()->reader(syffix);
    if (!reader) {
        LOGE() << "not found reader for file: " << path;
        return make_ret(Ret::Code::InternalError);
    }

    return load(path, reader);
}

Ms::MasterScore* MasterNotation::masterScore() const
{
    return dynamic_cast<Ms::MasterScore*>(score());
}

mu::Ret MasterNotation::load(const io::path& path, const INotationReaderPtr& reader)
{
    TRACEFUNC;

    Ms::ScoreLoad sl;

    Ms::MasterScore* score = new Ms::MasterScore(scoreGlobal()->baseStyle());
    Ret ret = doLoadScore(score, path, reader);

    if (ret) {
        setScore(score);
        initExcerpts();
    }

    return ret;
}

mu::Ret MasterNotation::doLoadScore(Ms::MasterScore* score,
                                    const io::path& path,
                                    const std::shared_ptr<INotationReader>& reader) const
{
    QFileInfo fi(path.toQString());
    score->setName(fi.completeBaseName());
    score->setImportedFilePath(fi.filePath());
    score->setMetaTag("originalFormat", fi.suffix().toLower());

    Ret ret = reader->read(score, path);
    if (!ret) {
        return ret;
    }

    score->connectTies();

    for (Ms::Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    score->setSoloMute();
    for (Ms::Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(Ms::LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }
    score->updateChannel();
    //score->updateExpressive(MuseScore::synthesizer("Fluid"));
    score->setSaved(true);
    score->setCreated(false);
    score->update();

    if (!score->sanityCheck(QString())) {
        return make_ret(Err::FileCorrupted);
    }

    return make_ret(Ret::Code::Ok);
}

mu::io::path MasterNotation::path() const
{
    const Ms::MasterScore* score = masterScore();

    if (!score) {
        return mu::io::path();
    }

    return score->fileInfo()->canonicalFilePath();
}

//! NOTE: this method with all of its dependencies was copied from MU3
//! source: file.cpp, MuseScore::getNewFile()
mu::Ret MasterNotation::createNew(const ScoreCreateOptions& scoreOptions)
{
    io::path templatePath = scoreOptions.templatePath;

    int measures = scoreOptions.measures;

    Ms::KeySigEvent ks;
    ks.setKey(scoreOptions.key);
    Ms::VBox* nvb = nullptr;

    bool pickupMeasure = scoreOptions.withPickupMeasure;
    if (pickupMeasure) {
        measures += 1;
    }

    Ms::MasterScore* score = new Ms::MasterScore(scoreGlobal()->baseStyle());

    QList<Ms::Excerpt*> excerpts;
    if (!templatePath.empty()) {
        std::string syffix = io::syffix(templatePath);
        auto reader = readers()->reader(syffix);
        if (!reader) {
            LOGE() << "not found reader for file: " << templatePath;
            return make_ret(Ret::Code::InternalError);
        }

        Ms::MasterScore* tscore = new Ms::MasterScore(scoreGlobal()->baseStyle());
        Ret ret = doLoadScore(tscore, templatePath, reader);
        if (!ret) {
            delete tscore;
            delete score;

            return ret;
        }
        score->setStyle(tscore->style());

        // create instruments from template
        for (Ms::Part* tpart : tscore->parts()) {
            Ms::Part* part = new Ms::Part(score);
            part->setInstrument(tpart->instrument());
            part->setPartName(tpart->partName());

            for (Ms::Staff* tstaff : *tpart->staves()) {
                Ms::Staff* staff = new Ms::Staff(score);
                staff->setPart(part);
                staff->init(tstaff);
                if (tstaff->links() && !part->staves()->isEmpty()) {
                    Staff* linkedStaff = part->staves()->back();
                    staff->linkTo(linkedStaff);
                }
                part->insertStaff(staff, -1);
                score->staves().append(staff);
            }
            score->appendPart(part);
        }
        for (Ms::Excerpt* ex : tscore->excerpts()) {
            Ms::Excerpt* x = new Ms::Excerpt(score);
            x->setTitle(ex->title());
            for (Part* p : ex->parts()) {
                int pidx = tscore->parts().indexOf(p);
                if (pidx == -1) {
                    LOGE() << "part not found";
                } else {
                    x->parts().append(score->parts()[pidx]);
                }
            }
            excerpts.append(x);
        }
        Ms::MeasureBase* mb = tscore->first();
        if (mb && mb->isVBox()) {
            Ms::VBox* tvb = toVBox(mb);
            nvb = new Ms::VBox(score);
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
        delete tscore;
    } else {
        score = new Ms::MasterScore(scoreGlobal()->baseStyle());
    }

    score->setName(qtrc("notation", "Untitled"));
    score->setCreated(true);
    setScore(score);

    if (templatePath.empty()) {
        parts()->setInstruments(scoreOptions.instruments);
    }

    score->style().checkChordList();
    if (!scoreOptions.title.isEmpty()) {
        score->fileInfo()->setFile(scoreOptions.title);
    }

    Ms::Fraction timesig(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);
    score->sigmap()->add(0, timesig);

    Ms::Fraction firstMeasureTicks = pickupMeasure ? Ms::Fraction(scoreOptions.measureTimesigNumerator,
                                                                  scoreOptions.measureTimesigDenominator) : timesig;

    for (int i = 0; i < measures; ++i) {
        Ms::Fraction tick = firstMeasureTicks + timesig * (i - 1);
        if (i == 0) {
            tick = Ms::Fraction(0,1);
        }
        QList<Ms::Rest*> puRests;
        for (Ms::Score* _score : score->scoreList()) {
            Ms::Rest* rest = 0;
            Ms::Measure* measure = new Ms::Measure(_score);
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
                    Ms::TimeSig* ts = new Ms::TimeSig(_score);
                    ts->setTrack(staffIdx * VOICES);
                    ts->setSig(timesig, scoreOptions.timesigType);
                    Ms::Measure* m = _score->firstMeasure();
                    Ms::Segment* s = m->getSegment(Ms::SegmentType::TimeSig, Ms::Fraction(0,1));
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
                            staff->setKey(Ms::Fraction(0,1), nKey);
                            Ms::KeySig* keysig = new Ms::KeySig(score);
                            keysig->setTrack(staffIdx * VOICES);
                            keysig->setKeySigEvent(nKey);
                            Ms::Segment* ss = measure->getSegment(Ms::SegmentType::KeySig, Ms::Fraction(0,1));
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
                    std::vector<Ms::TDuration> dList = toDurationList(measure->ticks(), false);
                    if (!dList.empty()) {
                        Ms::Fraction ltick = tick;
                        int k = 0;
                        foreach (Ms::TDuration d, dList) {
                            if (k < puRests.count()) {
                                rest = static_cast<Ms::Rest*>(puRests[k]->linkedClone());
                            } else {
                                rest = new Ms::Rest(score, d);
                                puRests.append(rest);
                            }
                            rest->setScore(_score);
                            rest->setTicks(d.fraction());
                            rest->setTrack(staffIdx * VOICES);
                            Ms::Segment* seg = measure->getSegment(Ms::SegmentType::ChordRest, ltick);
                            seg->add(rest);
                            ltick += rest->actualTicks();
                            k++;
                        }
                    }
                } else {
                    if (linkedToPrevious && rest) {
                        rest = static_cast<Ms::Rest*>(rest->linkedClone());
                    } else {
                        rest = new Ms::Rest(score, Ms::TDuration(Ms::TDuration::DurationType::V_MEASURE));
                    }
                    rest->setScore(_score);
                    rest->setTicks(measure->ticks());
                    rest->setTrack(staffIdx * VOICES);
                    Ms::Segment* seg = measure->getSegment(Ms::SegmentType::ChordRest, tick);
                    seg->add(rest);
                }
            }
        }
    }

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

    if (!scoreOptions.title.isEmpty() || !scoreOptions.subtitle.isEmpty() || !scoreOptions.composer.isEmpty()
        || !scoreOptions.lyricist.isEmpty()) {
        Ms::MeasureBase* measure = score->measures()->first();
        if (measure->type() != ElementType::VBOX) {
            Ms::MeasureBase* nm = nvb ? nvb : new Ms::VBox(score);
            nm->setTick(Ms::Fraction(0,1));
            nm->setNext(measure);
            score->measures()->add(nm);
            measure = nm;
        } else if (nvb) {
            delete nvb;
        }
        if (!scoreOptions.title.isEmpty()) {
            Ms::Text* s = new Ms::Text(score, Ms::Tid::TITLE);
            s->setPlainText(scoreOptions.title);
            measure->add(s);
            score->setMetaTag("workTitle", scoreOptions.title);
        }
        if (!scoreOptions.subtitle.isEmpty()) {
            Ms::Text* s = new Ms::Text(score, Ms::Tid::SUBTITLE);
            s->setPlainText(scoreOptions.subtitle);
            measure->add(s);
            score->setMetaTag("subtitle", scoreOptions.subtitle);
        }
        if (!scoreOptions.composer.isEmpty()) {
            Ms::Text* s = new Ms::Text(score, Ms::Tid::COMPOSER);
            s->setPlainText(scoreOptions.composer);
            measure->add(s);
            score->setMetaTag("composer", scoreOptions.composer);
        }
        if (!scoreOptions.lyricist.isEmpty()) {
            Ms::Text* s = new Ms::Text(score, Ms::Tid::POET);
            s->setPlainText(scoreOptions.lyricist);
            measure->add(s);
            score->setMetaTag("lyricist", scoreOptions.lyricist);
        }
    } else if (nvb) {
        delete nvb;
    }

    if (scoreOptions.withTempo) {
        Ms::Fraction ts = timesig;

        QString text("<sym>metNoteQuarterUp</sym> = %1");
        double bpm = scoreOptions.tempo.valueBpm;
        switch (ts.denominator()) {
        case 1:
            text = "<sym>metNoteWhole</sym> = %1";
            bpm /= 4;
            break;
        case 2:
            text = "<sym>metNoteHalfUp</sym> = %1";
            bpm /= 2;
            break;
        case 4:
            text = "<sym>metNoteQuarterUp</sym> = %1";
            break;
        case 8:
            if (ts.numerator() % 3 == 0) {
                text = "<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                bpm /= 1.5;
            } else {
                text = "<sym>metNote8thUp</sym> = %1";
                bpm *= 2;
            }
            break;
        case 16:
            if (ts.numerator() % 3 == 0) {
                text = "<sym>metNote8thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                bpm *= 1.5;
            } else {
                text = "<sym>metNote16thUp</sym> = %1";
                bpm *= 4;
            }
            break;
        case 32:
            if (ts.numerator() % 3 == 0) {
                text = "<sym>metNote16thUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                bpm *= 3;
            } else {
                text = "<sym>metNote32ndUp</sym> = %1";
                bpm *= 8;
            }
            break;
        case 64:
            if (ts.numerator() % 3 == 0) {
                text = "<sym>metNote32ndUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = %1";
                bpm *= 6;
            } else {
                text = "<sym>metNote64thUp</sym> = %1";
                bpm *= 16;
            }
            break;
        default:
            break;
        }

        Ms::TempoText* tt = new Ms::TempoText(score);
        tt->setXmlText(text.arg(bpm));

        double tempo = scoreOptions.tempo.valueBpm;
        tempo /= 60; // bpm -> bps

        tt->setTempo(tempo);
        tt->setFollowText(true);
        tt->setTrack(0);
        Ms::Segment* seg = score->firstMeasure()->first(Ms::SegmentType::ChordRest);
        seg->add(tt);
        score->setTempo(seg, tempo);
    }
    if (!scoreOptions.copyright.isEmpty()) {
        score->setMetaTag("copyright", scoreOptions.copyright);
    }

    {
        Ms::ScoreLoad sl;
        score->doLayout();
    }

    initExcerpts(excerpts);

    score->setExcerptsChanged(true);

    return make_ret(Err::NoError);
}

mu::RetVal<bool> MasterNotation::created() const
{
    RetVal<bool> result;
    if (!score()) {
        result.ret = make_ret(Err::NoScore);
        return result;
    }

    return RetVal<bool>::make_ok(score()->created());
}

mu::Ret MasterNotation::save(const io::path& path, SaveMode saveMode)
{
    switch (saveMode) {
    case SaveMode::SaveSelection:
        return saveSelectionOnScore(path);
    case SaveMode::Save:
    case SaveMode::SaveAs:
    case SaveMode::SaveCopy:
        return saveScore(path);
    case SaveMode::SaveOnline:
        return make_ret(Ret::Code::NotSupported);
    }

    return make_ret(Err::UnknownError);
}

mu::Ret MasterNotation::exportScore(const io::path& path, const std::string& suffix)
{
    QFile file(path.toQString());
    file.open(QFile::WriteOnly);

    auto writer = writers()->writer(suffix);
    if (!writer) {
        LOGE() << "Unknown export format:" << suffix;
        return false;
    }

    Ret ret = writer->write(shared_from_this(), file);
    file.close();

    return ret;
}

mu::ValNt<bool> MasterNotation::needSave() const
{
    ValNt<bool> needSave;
    needSave.val = !masterScore()->saved();
    needSave.notification = undoStack()->stackChanged();

    return needSave;
}

void MasterNotation::initExcerpts(const QList<Ms::Excerpt*>& scoreExcerpts)
{
    QList<Ms::Excerpt*> excerpts = scoreExcerpts;

    if (scoreExcerpts.empty()) {
        excerpts = Ms::Excerpt::createExcerptsFromParts(score()->parts());
    }

    ExcerptNotationList notationExcerpts;

    for (Ms::Excerpt* excerpt : excerpts) {
        masterScore()->initExcerpt(excerpt);
        notationExcerpts.push_back(std::make_shared<ExcerptNotation>(excerpt));
    }

    doSetExcerpts(notationExcerpts);

    m_parts->partsChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
        updateExcerpts();
    });
}

void MasterNotation::doSetExcerpts(ExcerptNotationList excerpts)
{
    m_excerpts.set(excerpts);
    static_cast<MasterNotationParts*>(m_parts.get())->setExcerpts(excerpts);
}

mu::ValCh<ExcerptNotationList> MasterNotation::excerpts() const
{
    return m_excerpts;
}

INotationPartsPtr MasterNotation::parts() const
{
    return m_parts;
}

INotationPtr MasterNotation::clone() const
{
    return std::make_shared<ExcerptNotation>(score()->clone()->excerpt());
}

void MasterNotation::setExcerpts(const ExcerptNotationList& excerpts)
{
    if (m_excerpts.val == excerpts) {
        return;
    }

    createNonexistentExcerpts(excerpts);

    doSetExcerpts(excerpts);
}

void MasterNotation::createNonexistentExcerpts(const ExcerptNotationList& newExcerpts)
{
    for (IExcerptNotationPtr excerptNotation : newExcerpts) {
        bool isNewExcerpt = std::find(m_excerpts.val.begin(), m_excerpts.val.end(), excerptNotation) == m_excerpts.val.end();
        bool isEmpty = excerptNotation->notation()->parts()->partList().empty();

        if (isNewExcerpt && isEmpty) {
            Ms::Excerpt* excerpt = new Ms::Excerpt(masterScore());
            masterScore()->initExcerpt(excerpt);
            get_impl(excerptNotation)->setExcerpt(excerpt);
        }
    }
}

void MasterNotation::updateExcerpts()
{
    ExcerptNotationList newExcerpts;

    for (IExcerptNotationPtr excerpt : m_excerpts.val) {
        if (!get_impl(excerpt)->excerpt()->isEmpty()) {
            newExcerpts.push_back(excerpt);
        }
    }

    QList<Ms::Excerpt*> excerpts = score()->excerpts();

    for (Part* part: score()->parts()) {
        bool isNewPart = true;

        for (Ms::Excerpt* exceprt: excerpts) {
            isNewPart &= !exceprt->containsPart(part);
        }

        if (isNewPart) {
            newExcerpts.push_back(createExcerpt(part));
        }
    }

    if (newExcerpts != m_excerpts.val) {
        doSetExcerpts(newExcerpts);
    }
}

IExcerptNotationPtr MasterNotation::createExcerpt(Part* part)
{
    Ms::Excerpt* excerpt = Ms::Excerpt::createExcerptFromPart(part);
    masterScore()->initExcerpt(excerpt);

    return std::make_shared<ExcerptNotation>(excerpt);
}

mu::Ret MasterNotation::saveScore(const mu::io::path& path, SaveMode saveMode)
{
    std::string suffix = io::syffix(path);
    if (suffix != "mscz" && suffix != "mscx" && !suffix.empty()) {
        return exportScore(path, suffix);
    }

    io::path oldFilePath = score()->masterScore()->fileInfo()->filePath().toStdString();

    if (!path.empty()) {
        score()->masterScore()->fileInfo()->setFile(path.toQString());
    }

    Ret ret = score()->masterScore()->saveFile(true);
    if (!ret) {
        ret.setText(Ms::MScore::lastError.toStdString());
    } else if (saveMode != SaveMode::SaveCopy || oldFilePath == path) {
        score()->setCreated(false);
        undoStack()->stackChanged().notify();
    }

    return make_ret(Ret::Code::Ok);
}

mu::Ret MasterNotation::saveSelectionOnScore(const mu::io::path& path)
{
    QFileInfo fileInfo(path.toQString());

    Ret ret = score()->saveCompressedFile(fileInfo, true);
    if (!ret) {
        ret.setText(Ms::MScore::lastError.toStdString());
    }

    return ret;
}
