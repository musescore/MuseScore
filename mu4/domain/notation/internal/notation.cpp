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
#include "notation.h"

#include <QPointF>
#include <QPainter>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>

#include "log.h"

#include "config.h"
#include "io/path.h"

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/part.h"

#include "libmscore/staff.h"
#include "libmscore/excerpt.h"
#include "libmscore/measure.h"
#include "libmscore/measurebase.h"
#include "libmscore/box.h"
#include "libmscore/timesig.h"
#include "libmscore/keysig.h"
#include "libmscore/rest.h"
#include "libmscore/tempotext.h"
#include "libmscore/synthesizerstate.h"

#include "../notationerrors.h"

//#ifdef BUILD_UI_MU4
////! HACK Temporary hack to link libmscore
//Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)

//namespace Ms {
//QString revision;
//MasterSynthesizer* synti;
//QString dataPath;
//QString mscoreGlobalShare;
//}
////! ---------
//#endif

using namespace mu::domain::notation;
using namespace Ms;

Notation::Notation()
{
    m_scoreGlobal = new MScore(); //! TODO May be static?

    m_interaction = new NotationInteraction(this);

    m_interaction->noteAdded().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dragChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->textEditingChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dropChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_playback = new NotationPlayback(this);
}

Notation::~Notation()
{
    delete m_score;
}

void Notation::init()
{
    MScore::init();         // initialize libmscore

    MScore::setNudgeStep(.1);           // cursor key (default 0.1)
    MScore::setNudgeStep10(1.0);        // Ctrl + cursor key (default 1.0)
    MScore::setNudgeStep50(0.01);       // Alt  + cursor key (default 0.01)

    MScore::pixelRatio = DPI / QGuiApplication::primaryScreen()->logicalDotsPerInch();
}

mu::Ret Notation::load(const io::path& path)
{
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

mu::Ret Notation::load(const io::path& path, const std::shared_ptr<INotationReader>& reader)
{
    if (m_score) {
        delete m_score;
        m_score = nullptr;
    }

    ScoreLoad sl;

    MasterScore* score = new MasterScore(m_scoreGlobal->baseStyle());
    Ret ret = doLoadScore(score, path, reader);
    if (ret) {
        m_score = score;
        m_interaction->init();
    }

    return ret;
}

mu::Ret Notation::doLoadScore(Ms::MasterScore* score,
                              const io::path& path,
                              const std::shared_ptr<INotationReader>& reader) const
{
    QFileInfo fi(io::pathToQString(path));
    score->setName(fi.completeBaseName());
    score->setImportedFilePath(fi.filePath());
    score->setMetaTag("originalFormat", fi.suffix().toLower());

    Ret ret = reader->read(score, path);
    if (!ret) {
        return ret;
    }

    score->connectTies();

    for (Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    score->setSoloMute();
    for (Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }
    score->updateChannel();
    //score->updateExpressive(MuseScore::synthesizer("Fluid"));
    score->setSaved(false);
    score->update();

    if (!score->sanityCheck(QString())) {
        return make_ret(Err::FileCorrupted);
    }

    return make_ret(Ret::Code::Ok);
}

mu::io::path Notation::path() const
{
    if (!m_score) {
        return io::path();
    }

    return io::pathFromQString(m_score->fileInfo()->canonicalFilePath());
}

mu::Ret Notation::createNew(const ScoreCreateOptions& scoreOptions)
{
    RetVal<MasterScore*> score = newScore(scoreOptions);

    if (!score.ret) {
        return score.ret;
    }

    m_score = score.val;
    m_interaction->init();

    return make_ret(Err::NoError);
}

void Notation::setViewSize(const QSizeF& vs)
{
    m_viewSize = vs;
}

void Notation::paint(QPainter* p, const QRect&)
{
    const QList<Ms::Page*>& mspages = m_score->pages();

    if (mspages.isEmpty()) {
        p->drawText(10, 10, "no pages");
        return;
    }

    Ms::Page* page = mspages.first();
    page->draw(p);

    p->fillRect(page->bbox(), QColor("#ffffff"));

    QList<Ms::Element*> ell = page->elements();
    for (const Ms::Element* e : ell) {
        if (!e->visible()) {
            continue;
        }

        e->itemDiscovered = false;
        QPointF pos(e->pagePos());
        //LOGI() << e->name() << ", x: " << pos.x() << ", y: " << pos.y() << "\n";

        p->translate(pos);

        e->draw(p);

        p->translate(-pos);
    }

    m_interaction->paint(p);
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

mu::RetVal<MasterScore*> Notation::newScore(const ScoreCreateOptions& scoreOptions)
{
    RetVal<MasterScore*> result;

    double tempo = scoreOptions.tempo;
    bool tempoChecked = tempo > 0;
    Fraction timesig(scoreOptions.timesigNumerator, scoreOptions.timesigDenominator);

    io::path templatePath = io::pathFromQString(scoreOptions.templatePath);

    int measures = scoreOptions.measures;

    KeySigEvent ks;
    ks.setKey(scoreOptions.key);
    VBox* nvb = nullptr;

    bool pickupMeasure = scoreOptions.measureTimesigNumerator > 0 && scoreOptions.measureTimesigDenominator > 0;
    if (pickupMeasure) {
        measures += 1;
    }

    MasterScore* score = new MasterScore(m_scoreGlobal->baseStyle());

    QList<Excerpt*> excerpts;
    if (!templatePath.empty()) {
        std::string syffix = io::syffix(templatePath);
        auto reader = readers()->reader(syffix);
        if (!reader) {
            LOGE() << "not found reader for file: " << templatePath;
            result.ret = make_ret(Ret::Code::InternalError);
            return result;
        }

        MasterScore* tscore = new MasterScore(m_scoreGlobal->baseStyle());
        Ret ret = doLoadScore(tscore, templatePath, reader);
        if (!ret) {
            delete tscore;
            delete score;

            result.ret = ret;
            return result;
        }
        score->setStyle(tscore->style());

        // create instruments from template
        for (Part* tpart : tscore->parts()) {
            Part* part = new Part(score);
            part->setInstrument(tpart->instrument());
            part->setPartName(tpart->partName());

            for (Staff* tstaff : *tpart->staves()) {
                Staff* staff = new Staff(score);
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
        for (Excerpt* ex : tscore->excerpts()) {
            Excerpt* x = new Excerpt(score);
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
        MeasureBase* mb = tscore->first();
        if (mb && mb->isVBox()) {
            VBox* tvb = toVBox(mb);
            nvb = new VBox(score);
            nvb->setBoxHeight(tvb->boxHeight());
            nvb->setBoxWidth(tvb->boxWidth());
            nvb->setTopGap(tvb->topGap());
            nvb->setBottomGap(tvb->bottomGap());
            nvb->setTopMargin(tvb->topMargin());
            nvb->setBottomMargin(tvb->bottomMargin());
            nvb->setLeftMargin(tvb->leftMargin());
            nvb->setRightMargin(tvb->rightMargin());
        }
        delete tscore;
    } else {
        score = new MasterScore(m_scoreGlobal->baseStyle());
//        newWizard->createInstruments(score);
    }
    score->setCreated(true);
    score->fileInfo()->setFile(scoreOptions.title); // create unique default name

    score->style().checkChordList();
    if (!scoreOptions.title.isEmpty()) {
        score->fileInfo()->setFile(scoreOptions.title);
    }

    score->sigmap()->add(0, timesig);

    Fraction firstMeasureTicks = pickupMeasure ? Fraction(scoreOptions.measureTimesigNumerator,
                                                          scoreOptions.measureTimesigDenominator) : timesig;

    for (int i = 0; i < measures; ++i) {
        Fraction tick = firstMeasureTicks + timesig * (i - 1);
        if (i == 0) {
            tick = Fraction(0,1);
        }
        QList<Rest*> puRests;
        for (Score* _score : score->scoreList()) {
            Rest* rest = 0;
            Measure* measure = new Measure(_score);
            measure->setTimesig(timesig);
            measure->setTicks(timesig);
            measure->setTick(tick);

            if (pickupMeasure && tick.isZero()) {
                measure->setIrregular(true);                // don’t count pickup measure
                measure->setTicks(Fraction(scoreOptions.measureTimesigNumerator,
                                           scoreOptions.measureTimesigDenominator));
            }
            _score->measures()->add(measure);

            for (Staff* staff : _score->staves()) {
                int staffIdx = staff->idx();
                if (tick.isZero()) {
                    TimeSig* ts = new TimeSig(_score);
                    ts->setTrack(staffIdx * VOICES);
                    ts->setSig(timesig, scoreOptions.timesigType);
                    Measure* m = _score->firstMeasure();
                    Segment* s = m->getSegment(SegmentType::TimeSig, Fraction(0,1));
                    s->add(ts);
                    Part* part = staff->part();
                    if (!part->instrument()->useDrumset()) {
                        //
                        // transpose key
                        //
                        KeySigEvent nKey = ks;
                        if (!nKey.custom() && !nKey.isAtonal() && part->instrument()->transpose().chromatic
                            && !score->styleB(Sid::concertPitch)) {
                            int diff = -part->instrument()->transpose().chromatic;
                            nKey.setKey(transposeKey(nKey.key(), diff, part->preferSharpFlat()));
                        }
                        // do not create empty keysig unless custom or atonal
                        if (nKey.custom() || nKey.isAtonal() || nKey.key() != Key::C) {
                            staff->setKey(Fraction(0,1), nKey);
                            KeySig* keysig = new KeySig(score);
                            keysig->setTrack(staffIdx * VOICES);
                            keysig->setKeySigEvent(nKey);
                            Segment* ss = measure->getSegment(SegmentType::KeySig, Fraction(0,1));
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
                    std::vector<TDuration> dList = toDurationList(measure->ticks(), false);
                    if (!dList.empty()) {
                        Fraction ltick = tick;
                        int k = 0;
                        foreach (TDuration d, dList) {
                            if (k < puRests.count()) {
                                rest = static_cast<Rest*>(puRests[k]->linkedClone());
                            } else {
                                rest = new Rest(score, d);
                                puRests.append(rest);
                            }
                            rest->setScore(_score);
                            rest->setTicks(d.fraction());
                            rest->setTrack(staffIdx * VOICES);
                            Segment* seg = measure->getSegment(SegmentType::ChordRest, ltick);
                            seg->add(rest);
                            ltick += rest->actualTicks();
                            k++;
                        }
                    }
                } else {
                    if (linkedToPrevious && rest) {
                        rest = static_cast<Rest*>(rest->linkedClone());
                    } else {
                        rest = new Rest(score, TDuration(TDuration::DurationType::V_MEASURE));
                    }
                    rest->setScore(_score);
                    rest->setTicks(measure->ticks());
                    rest->setTrack(staffIdx * VOICES);
                    Segment* seg = measure->getSegment(SegmentType::ChordRest, tick);
                    seg->add(rest);
                }
            }
        }
    }
    //TODO      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);

    //
    // select first rest
    //
    Measure* m = score->firstMeasure();
    for (Segment* s = m->first(); s; s = s->next()) {
        if (s->segmentType() == SegmentType::ChordRest) {
            if (s->element(0)) {
                score->select(s->element(0), SelectType::SINGLE, 0);
                break;
            }
        }
    }

    if (!scoreOptions.title.isEmpty() || !scoreOptions.subtitle.isEmpty() || !scoreOptions.composer.isEmpty()
        || !scoreOptions.lyricist.isEmpty()) {
        MeasureBase* measure = score->measures()->first();
        if (measure->type() != ElementType::VBOX) {
            MeasureBase* nm = nvb ? nvb : new VBox(score);
            nm->setTick(Fraction(0,1));
            nm->setNext(measure);
            score->measures()->add(nm);
            measure = nm;
        } else if (nvb) {
            delete nvb;
        }
        if (!scoreOptions.title.isEmpty()) {
            Text* s = new Text(score, Tid::TITLE);
            s->setPlainText(scoreOptions.title);
            measure->add(s);
            score->setMetaTag("workTitle", scoreOptions.title);
        }
        if (!scoreOptions.subtitle.isEmpty()) {
            Text* s = new Text(score, Tid::SUBTITLE);
            s->setPlainText(scoreOptions.subtitle);
            measure->add(s);
        }
        if (!scoreOptions.composer.isEmpty()) {
            Text* s = new Text(score, Tid::COMPOSER);
            s->setPlainText(scoreOptions.composer);
            measure->add(s);
            score->setMetaTag("composer", scoreOptions.composer);
        }
        if (!scoreOptions.lyricist.isEmpty()) {
            Text* s = new Text(score, Tid::POET);
            s->setPlainText(scoreOptions.lyricist);
            measure->add(s);
            // the poet() functions returns data called lyricist in the dialog
            score->setMetaTag("lyricist", scoreOptions.lyricist);
        }
    } else if (nvb) {
        delete nvb;
    }

    if (tempoChecked) {
        Fraction ts = timesig;

        QString text("<sym>metNoteQuarterUp</sym> = %1");
        double bpm = scoreOptions.tempo;
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

        TempoText* tt = new TempoText(score);
        tt->setXmlText(text.arg(bpm));
        tempo /= 60;          // bpm -> bps

        tt->setTempo(tempo);
        tt->setFollowText(true);
        tt->setTrack(0);
        Segment* seg = score->firstMeasure()->first(SegmentType::ChordRest);
        seg->add(tt);
        score->setTempo(seg, tempo);
    }
    if (!scoreOptions.copyright.isEmpty()) {
        score->setMetaTag("copyright", scoreOptions.copyright);
    }

//    if (synti) {
//        score->setSynthesizerState(synti->state());
//    }

    // Call this even if synti doesn't exist - we need to rebuild either way
//    score->rebuildAndUpdateExpressive(MuseScore::synthesizer("Fluid"));

    {
        ScoreLoad sl;
        score->doLayout();
    }

    for (Excerpt* x : excerpts) {
        Score* xs = new Score(static_cast<MasterScore*>(score));
        xs->style().set(Sid::createMultiMeasureRests, true);
        x->setPartScore(xs);
        xs->setExcerpt(x);
        score->excerpts().append(x);
        Excerpt::createExcerpt(x);
    }

    score->setExcerptsChanged(true);

    result.ret = make_ret(Err::NoError);
    result.val = score;
    return result;
}

INotationInteraction* Notation::interaction() const
{
    return m_interaction;
}

INotationPlayback* Notation::playback() const
{
    return m_playback;
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

Ms::Score* Notation::score() const
{
    return m_score;
}

QSizeF Notation::viewSize() const
{
    return m_viewSize;
}
