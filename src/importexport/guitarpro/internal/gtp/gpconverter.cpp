#include "gpconverter.h"

#include <chrono>

#include "gpdommodel.h"

#include "libmscore/factory.h"
#include "libmscore/arpeggio.h"
#include "libmscore/box.h"
#include "libmscore/bend.h"
#include "libmscore/bracketItem.h"
#include "libmscore/clef.h"
#include "libmscore/chord.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/factory.h"
#include "libmscore/fermata.h"
#include "libmscore/fret.h"
#include "libmscore/fingering.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/jump.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/letring.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/part.h"
#include "libmscore/palmmute.h"
#include "libmscore/rest.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/score.h"
#include "libmscore/slide.h"
#include "libmscore/slur.h"
#include "libmscore/spanner.h"
#include "libmscore/stafftext.h"
#include "libmscore/staff.h"
#include "types/symid.h"
#include "libmscore/tempotext.h"
#include "libmscore/tempochangeranged.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

#include "../importgtp.h"

#include "log.h"
#include "translation.h"

using namespace mu::engraving;

namespace Ms {
static Jump::Type jumpType(const QString& typeString)
{
    static QMap<QString, Jump::Type> types {
        { "DaCapo", Jump::Type::DC },
        { "DaCapoAlFine", Jump::Type::DC_AL_FINE },
        { "DaCapoAlCoda", Jump::Type::DC_AL_CODA },
        { "DaCapoAlDoubleCoda", Jump::Type::DC_AL_DBLCODA },
        { "DaSegnoAlCoda", Jump::Type::DS_AL_CODA },
        { "DaSegnoAlDoubleCoda", Jump::Type::DS_AL_DBLCODA },
        { "DaSegnoAlFine", Jump::Type::DS_AL_FINE },
        { "DaSegnoSegno", Jump::Type::DSS },
        { "DaSegnoSegnoAlCoda", Jump::Type::DSS_AL_CODA },
        { "DaSegnoSegnoAlDoubleCoda", Jump::Type::DSS_AL_DBLCODA },
        { "DaSegnoSegnoAlFine", Jump::Type::DSS_AL_FINE },
        { "DaCoda", Jump::Type::DCODA },
        { "DaDoubleCoda", Jump::Type::DDBLCODA },
        { "DaCoda", Jump::Type::DCODA },
    };

    if (types.contains(typeString)) {
        return types[typeString];
    }

    LOGE() << "wrong jump type";
    return Jump::Type::USER;
}

static Marker::Type markerType(const QString& typeString)
{
    static QMap<QString, Marker::Type> types {
        { "Segno", Marker::Type::SEGNO },
        { "SegnoSegno", Marker::Type::VARSEGNO },
        { "Coda", Marker::Type::CODA },
        { "DoubleCoda", Marker::Type::VARCODA },
        { "Fine", Marker::Type::FINE }
    };

    if (types.contains(typeString)) {
        return types[typeString];
    }

    LOGE() << "wrong direction marker type";
    return Marker::Type::USER;
}

GPConverter::GPConverter(Score* score, std::unique_ptr<GPDomModel>&& gpDom)
    : _score(score), _gpDom(std::move(gpDom))
{}

const std::unique_ptr<GPDomModel>& GPConverter::gpDom() const
{
    return _gpDom;
}

void GPConverter::convertGP()
{
    setUpGPScore(_gpDom->score());
    setUpTracks(_gpDom->tracks());
    collectTempoMap(_gpDom->masterTracks());

    convert(_gpDom->masterBars());

    clearDefectedSpanner();

    // @NOTE(o.zhukov@wsmgroup.ru): After reading some of the GP files, there're
    //                              parts, which contain only one percussion instrument.
    //                              In this case the program value for this instrument will
    //                              be set to the pitch value of the notes of the instrument,
    //                              thus we need to reset this value to 0.
    for (int i = 0; i < _score->parts().size(); ++i) {
        Ms::Part* pPart = _score->parts()[i];
        IF_ASSERT_FAILED(!!pPart) {
            continue;
        }

        Ms::Instrument* pInstrument = pPart->instrument();
        if (!pInstrument) {
            continue;
        }

        if (!pInstrument->useDrumset()) {
            continue;
        }

        for (int j = 0; j < pInstrument->channel().size(); ++j) {
            Ms::Channel* pChannel = pInstrument->channel()[j];
            IF_ASSERT_FAILED(!!pChannel) {
                continue;
            }

            if (!(pChannel->channel() == 9)) {
                continue;
            }

            pChannel->setProgram(0);
        }
    }

    // adding capo
    for (int i = 0; i < _score->parts().size(); ++i) {
        Ms::Part* part = _score->parts()[i];
        IF_ASSERT_FAILED(part && !part->staves()->isEmpty()) {
            continue;
        }
        Ms::Staff* staff = part->staves()->first();

        Fraction fr = { 0, 1 };
        int capo = staff->capo(fr);
        QString instrName = part->partName();
        if (capo != 0) {
            Measure* measure = _score->firstMeasure();
            Segment* s = measure->getSegment(SegmentType::TimeSig, measure->tick());
            StaffText* st = Factory::createStaffText(s);
            st->setTrack(0);
            QString capoText = QString("Capo fret %1").arg(capo);
            if (_score->parts().size() > 1) {
                capoText = instrName + ": " + capoText;
            }
            st->setPlainText(mu::qtrc("iex_guitarpro", capoText.toStdString().c_str()));
            s->add(st);
        }
    }
}

void GPConverter::convert(const std::vector<std::unique_ptr<GPMasterBar> >& masterBars)
{
    for (uint32_t mi = 0; mi < masterBars.size(); ++mi) {
        Context ctx;
        ctx.masterBarIndex = mi;
        convertMasterBar(masterBars.at(mi).get(), ctx);
    }

    // adding end tick of ottava if the score ends with it
    if (_lastOttava) {
        _lastOttava->setTick2(_score->endTick());
        _score->addElement(_lastOttava);
        _lastOttava = nullptr;
    }

    addTempoMap();
    addFermatas();
    addContiniousSlideHammerOn();
}

void GPConverter::convertMasterBar(const GPMasterBar* mB, Context ctx)
{
    Measure* measure = addMeasure(mB);

    addTimeSig(mB, measure);

    addKeySig(mB, measure);

    addBarline(mB, measure, ctx);

    addRepeat(mB, measure);

    collectFermatas(mB, measure);

    convertBars(mB->bars(), ctx);

    addTripletFeel(mB, measure);

    addSection(mB, measure);

    addDirection(mB, measure);
}

void GPConverter::convertBars(const std::vector<std::unique_ptr<GPBar> >& bars, Context ctx)
{
    ctx.curTrack = 0;
    for (const auto& bar : bars) {
        convertBar(bar.get(), ctx);
        ctx.curTrack += VOICES;
    }
}

void GPConverter::convertBar(const GPBar* bar, Context ctx)
{
    addClef(bar, ctx.curTrack);

    if (addSimileMark(bar, ctx.curTrack)) {
        return;
    }
    convertVoices(bar->voices(), ctx);
}

void GPConverter::addBarline(const GPMasterBar* mB, Measure* measure, Context ctx)
{
    static bool insideFreeTime = false;
    int curTrack = ctx.curTrack;

    if (mB->barlineType() == GPMasterBar::BarlineType::DOUBLE) {
        measure->setEndBarLineType(Ms::BarLineType::DOUBLE, curTrack);
    }

    GPMasterBar::TimeSig sig = mB->timeSig();
    auto scoreTimeSig = Fraction(sig.enumerator, sig.denumerator);

    if (mB->freeTime()) {
        if (mB->barlineType() != GPMasterBar::BarlineType::DOUBLE) {
            measure->setEndBarLineType(Ms::BarLineType::BROKEN, curTrack);
        }
        if (!insideFreeTime) {
            insideFreeTime = true;

            // Free time text
            Segment* s = measure->getSegment(SegmentType::TimeSig, measure->tick());
            StaffText* st = Factory::createStaffText(s);
            st->setTrack(curTrack);
            st->setPlainText(mu::qtrc("iex_guitarpro", "Free time"));
            s->add(st);

            // if timeSig is different, it was added before, here we handle "freetime"
            if (_lastTimeSig.enumerator != sig.enumerator
                || _lastTimeSig.denumerator != sig.denumerator) {
                return;
            }
            TimeSig* ts = Factory::createTimeSig(s);
            ts->setSig(scoreTimeSig);
            ts->setTrack(curTrack);
            ts->setLargeParentheses(true);
            s->add(ts);
        }
    } else {
        insideFreeTime = false;
    }
    _lastTimeSig.enumerator = sig.enumerator;
    _lastTimeSig.denumerator = sig.denumerator;
}

void GPConverter::convertVoices(const std::vector<std::unique_ptr<GPVoice> >& voices, Context ctx)
{
    if (voices.empty()) {
        ctx.curTick = _score->lastMeasure()->tick();
        fillUncompletedMeasure(ctx);
    }

    for (const auto& voice : voices) {
        convertVoice(voice.get(), ctx);
        ctx.curTrack++;
    }
}

void GPConverter::convertVoice(const GPVoice* voice, Context ctx)
{
    ctx.curTick = _score->lastMeasure()->tick();
    convertBeats(voice->beats(), ctx);
}

void GPConverter::convertBeats(const std::vector<std::shared_ptr<GPBeat> >& beats, Context ctx)
{
    ChordRestContainer graceGhords;
    for (const auto& beat : beats) {
        ctx.curTick = convertBeat(beat.get(), graceGhords, ctx);
    }

    fillUncompletedMeasure(ctx);
    clearDefectedGraceChord(graceGhords);
}

Fraction GPConverter::convertBeat(const GPBeat* beat, ChordRestContainer& graceGhords, Context ctx)
{
    if (ctx.curTick >= _score->lastMeasure()->ticks() + _score->lastMeasure()->tick()) {
        return ctx.curTick;
    }

    ChordRest* cr = addChordRest(beat, ctx);

    if (beat->graceNotes() != GPBeat::GraceNotes::None) {
        if (cr->type() == ElementType::REST) {
            delete cr;
            return ctx.curTick;
        }
        configureGraceChord(beat, cr);
        graceGhords.push_back({ cr, beat });
        return ctx.curTick;
    }

    auto curSegment = _score->lastMeasure()->getSegment(SegmentType::ChordRest, ctx.curTick);
    curSegment->add(cr);

    convertNotes(beat->notes(), cr);

    if (!graceGhords.empty()) {
        int grIndex = 0;
        for (auto [pGrChord, pBeat] : graceGhords) {
            if (pGrChord->type() == ElementType::CHORD) {
                static_cast<Chord*>(pGrChord)->setGraceIndex(grIndex++);
            }
            cr->add(pGrChord);
            addLegato(pBeat, pGrChord);
        }
    }
    graceGhords.clear();

    addTuplet(beat, cr);
    addTimer(beat, cr);
    addFreeText(beat, cr);
    addVibratoWTremBar(beat, cr);
    addFadding(beat, cr);
    addHairPin(beat, cr);
    addRasgueado(beat, cr);
    addTremolo(beat, cr);
    addPickStroke(beat, cr);
    addDynamic(beat, cr);
    addWah(beat, cr);
    addFretDiagram(beat, cr, ctx);
    addBarre(beat, cr);
    addSlapped(beat, cr);
    addPopped(beat, cr);
    addBrush(beat, cr);
    addArpeggio(beat, cr);
    addLyrics(beat, cr, ctx);
    addLegato(beat, cr);
    addOttava(beat, cr);

    ctx.curTick += cr->actualTicks();

    return ctx.curTick;
}

void GPConverter::convertNotes(const std::vector<std::shared_ptr<GPNote> >& notes, ChordRest* cr)
{
    for (const auto& note : notes) {
        convertNote(note.get(), cr);
    }
}

void GPConverter::convertNote(const GPNote* gpnote, ChordRest* cr)
{
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    Chord* ch = static_cast<Chord*>(cr);

    Note* note = mu::engraving::Factory::createNote(ch);
    note->setTrack(cr->track());
    cr->add(note);
    configureNote(gpnote, note);

    addAccent(gpnote, note);
    addSlide(gpnote, note);
    collectHammerOn(gpnote, note);
    addTapping(gpnote, note);
    addLeftHandTapping(gpnote, note);
    addOrnament(gpnote, note);
    addVibratoLeftHand(gpnote, note);
    addTrill(gpnote, note);
    addHarmonic(gpnote, note);
    addFingering(gpnote, note);
    addTie(gpnote, note);
}

void GPConverter::configureGraceChord(const GPBeat* beat, ChordRest* cr)
{
    convertNotes(beat->notes(), cr);

    auto rhytm = [](GPRhytm::RhytmType rhytm) {
        if (rhytm == GPRhytm::RhytmType::Whole) {
            return 1;
        } else if (rhytm == GPRhytm::RhytmType::Half) {
            return 2;
        } else if (rhytm == GPRhytm::RhytmType::Quarter) {
            return 4;
        } else if (rhytm == GPRhytm::RhytmType::Eighth) {
            return 8;
        } else if (rhytm == GPRhytm::RhytmType::Sixteenth) {
            return 16;
        } else if (rhytm == GPRhytm::RhytmType::ThirtySecond) {
            return 32;
        } else {
            return 64;
        }
    };

    Fraction fr(1, rhytm(beat->lenth().second));
    cr->setDurationType(TDuration(fr));

    if (cr->type() == ElementType::CHORD) {
        Chord* grChord = static_cast<Chord*>(cr);

        if (GPBeat::GraceNotes::OnBeat == beat->graceNotes()) {
            grChord->setNoteType(NoteType::APPOGGIATURA);
        } else {
            grChord->setNoteType(NoteType::ACCIACCATURA);
        }
    }
}

void GPConverter::addTimeSig(const GPMasterBar* mB, Measure* measure)
{
    GPMasterBar::TimeSig sig = mB->timeSig();
    Fraction tick = measure->tick();
    auto scoreTimeSig = Fraction(sig.enumerator, sig.denumerator);
    measure->setTicks(scoreTimeSig);
    int staves = _score->staves().count();

    if (_lastTimeSig.enumerator == sig.enumerator
        && _lastTimeSig.denumerator == sig.denumerator) {
        return;
    }
    _lastTimeSig.enumerator = sig.enumerator;
    _lastTimeSig.denumerator = sig.denumerator;

    for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Staff* staff = _score->staff(staffIdx);
        if (staff->staffType()->genTimesig()) {
            TimeSig* t = Factory::createTimeSig(_score->dummy()->segment());
            t->setTrack(staffIdx * VOICES);
            t->setSig(scoreTimeSig);
            if (mB->freeTime()) {
                t->setLargeParentheses(true);
            }
            Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
            s->add(t);
        }
    }
}

void GPConverter::addRepeat(const GPMasterBar* mB, Measure* measure)
{
    addVolta(mB, measure);

    if (mB->repeat().type == GPMasterBar::Repeat::Type::None) {
        return;
    }

    if (mB->repeat().type == GPMasterBar::Repeat::Type::Start) {
        measure->setRepeatStart(true);
    }
    if (mB->repeat().type == GPMasterBar::Repeat::Type::End) {
        measure->setRepeatEnd(true);
    }
    if (mB->repeat().type == GPMasterBar::Repeat::Type::StartEnd) {
        measure->setRepeatStart(true);
        measure->setRepeatEnd(true);
    }
    measure->setRepeatCount(mB->repeat().count);
}

void GPConverter::addVolta(const GPMasterBar* mB, Measure* measure)
{
    if (mB->alternateEnding().empty()) {
        _lastVolta = nullptr;
        return;
    }

    if (_lastVolta && _lastVolta->endings().size() != static_cast<int>(mB->alternateEnding().size())) {
        _lastVolta = nullptr;
    }

    if (_lastVolta) {
        for (const auto& end : mB->alternateEnding()) {
            if (!_lastVolta->hasEnding(end)) {
                _lastVolta = nullptr;
                break;
            }
        }
    }

    doAddVolta(mB, measure);
}

void GPConverter::doAddVolta(const GPMasterBar* mB, Measure* measure)
{
    Ms::Volta* volta;
    if (_lastVolta) {
        volta = _lastVolta;
        _score->removeElement(volta);
    } else {
        volta = new Ms::Volta(_score->dummy());
        volta->setTick(measure->tick());
        _lastVolta = volta;
    }

    volta->endings().clear();
    volta->setTick2(measure->tick() + measure->ticks());

    QString str;
    for (const auto& end : mB->alternateEnding()) {
        volta->endings().push_back(end);
        if (str.isEmpty()) {
            str += QString("%1").arg(end);
        } else {
            str += QString("-%1").arg(end);
        }
    }
    volta->setText(str);

    _score->addElement(volta);
}

void GPConverter::addDirection(const GPMasterBar* mB, Measure* measure)
{
    if (!mB->direction().jump.isEmpty()) {
        Jump* jump = Factory::createJump(measure);
        jump->setJumpType(jumpType(mB->direction().jump));
        jump->setTrack(0);
        measure->add(jump);
    }

    if (!mB->direction().target.isEmpty()) {
        Marker* marker = Factory::createMarker(measure);
        marker->setMarkerType(markerType(mB->direction().target));
        marker->setTrack(0);
        measure->add(marker);
    }
}

void GPConverter::addSection(const GPMasterBar* mB, Measure* measure)
{
    if (mB->section().first.isEmpty() && mB->section().second.isEmpty()) {
        return;
    }

    if (!mB->section().first.isEmpty()) {
        Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
        RehearsalMark* t = Factory::createRehearsalMark(_score->dummy()->segment());
        t->setPlainText(mB->section().first);
        t->setType(RehearsalMark::Type::Main);
        t->setTrack(0);
        s->add(t);
    }
    if (!mB->section().second.isEmpty()) {
        Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
        RehearsalMark* t = Factory::createRehearsalMark(_score->dummy()->segment());
        t->setPlainText(mB->section().second);
        t->setType(RehearsalMark::Type::Additional);
        t->setTrack(0);
        s->add(t);
    }
}

void GPConverter::addTripletFeel(const GPMasterBar* mB, Measure* /*measure*/)
{
    if (mB->tripletFeel() == _lastTripletFeel) {
        return; // if last triplet of last measure is equal current dont create new staff text
    }

//    auto convertTripletFeel = [] (GPMasterBar::TripletFeelType tf) {
//        if (tf == GPMasterBar::TripletFeelType::Triplet8th) return TripletFeelType::Triplet8th;
//        else if (tf == GPMasterBar::TripletFeelType::Triplet16th) return TripletFeelType::Triplet16th;
//        else if (tf == GPMasterBar::TripletFeelType::Dotted8th) return TripletFeelType::Dotted8th;
//        else if (tf == GPMasterBar::TripletFeelType::Dotted16th) return TripletFeelType::Dotted16th;
//        else if (tf == GPMasterBar::TripletFeelType::Scottish8th) return TripletFeelType::Scottish8th;
//        else if (tf == GPMasterBar::TripletFeelType::Scottish16th) return TripletFeelType::Scottish16th;
//        return TripletFeelType::None;
//    };
}

void GPConverter::addKeySig(const GPMasterBar* mB, Measure* measure)
{
    auto convertKeySig = [](GPMasterBar::KeySig kS) {
        if (kS == GPMasterBar::KeySig::C_B) {
            return Key::C_B;
        } else if (kS == GPMasterBar::KeySig::C_B) {
            return Key::C_B;
        } else if (kS == GPMasterBar::KeySig::G_B) {
            return Key::G_B;
        } else if (kS == GPMasterBar::KeySig::D_B) {
            return Key::D_B;
        } else if (kS == GPMasterBar::KeySig::A_B) {
            return Key::A_B;
        } else if (kS == GPMasterBar::KeySig::E_B) {
            return Key::E_B;
        } else if (kS == GPMasterBar::KeySig::B_B) {
            return Key::B_B;
        } else if (kS == GPMasterBar::KeySig::F) {
            return Key::F;
        } else if (kS == GPMasterBar::KeySig::C) {
            return Key::C;
        } else if (kS == GPMasterBar::KeySig::G) {
            return Key::G;
        } else if (kS == GPMasterBar::KeySig::D) {
            return Key::D;
        } else if (kS == GPMasterBar::KeySig::A) {
            return Key::A;
        } else if (kS == GPMasterBar::KeySig::E) {
            return Key::E;
        } else if (kS == GPMasterBar::KeySig::B) {
            return Key::B;
        } else if (kS == GPMasterBar::KeySig::F_S) {
            return Key::F_S;
        } else {
            return Key::C_S;
        }
    };

    Fraction tick = measure->tick();
    auto scoreKeySig = convertKeySig(mB->keySig());
    int staves = _score->staves().count();

    for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
        if (!tick.isZero() && _lastKeySigs[staffIdx] == mB->keySig()) {
            continue;
        }

        Staff* staff = _score->staff(staffIdx);
        if (staff->staffType()->genTimesig()) {
            KeySig* t = mu::engraving::Factory::createKeySig(_score->dummy()->segment());
            t->setTrack(staffIdx * VOICES);
            t->setKey(scoreKeySig);
            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            s->add(t);
            _lastKeySigs[staffIdx] = mB->keySig();
        }
    }
}

void GPConverter::setUpGPScore(const GPScore* gpscore)
{
    MeasureBase* m = nullptr;
    if (!_score->measures()->first()) {
        m = Factory::createVBox(_score->dummy()->system());
        m->setTick(Fraction(0, 1));
        _score->addMeasure(m, 0);
    } else {
        m = _score->measures()->first();
        if (!m->isVBox()) {
            MeasureBase* mb = Factory::createVBox(_score->dummy()->system());
            mb->setTick(Fraction(0, 1));
            _score->addMeasure(mb, m);
            m = mb;
        }
    }

    if (!gpscore->title().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::TITLE);
        s->setPlainText(gpscore->title());
        m->add(s);
    }
    if (!gpscore->subTitle().isEmpty() || !gpscore->artist().isEmpty() || !gpscore->album().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::SUBTITLE);
        QString str;
        if (!gpscore->subTitle().isEmpty()) {
            str.append(gpscore->subTitle());
        }
        if (!gpscore->artist().isEmpty()) {
            if (!str.isEmpty()) {
                str.append("\n");
            }
            str.append(gpscore->artist());
        }
        if (!gpscore->album().isEmpty()) {
            if (!str.isEmpty()) {
                str.append("\n");
            }
            str.append(gpscore->album());
        }
        s->setPlainText(str);
        m->add(s);
    }
    if (!gpscore->composer().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::COMPOSER);
        s->setPlainText(mu::qtrc("iex_guitarpro", "Music by %1").arg(gpscore->composer()));
        m->add(s);
    }
    if (!gpscore->poet().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::POET);
        s->setPlainText(mu::qtrc("iex_guitarpro", "Words by %1").arg(gpscore->poet()));
        m->add(s);
    }
}

void GPConverter::setUpTracks(const std::map<int, std::unique_ptr<GPTrack> >& tracks)
{
    for (const auto& track : tracks) {
        setUpTrack(track.second);
    }
}

void GPConverter::setUpTrack(const std::unique_ptr<GPTrack>& tR)
{
    int programm = tR->programm();
    int midiChannel = tR->midiChannel();

    int idx = tR->idx() + 1; // 0 is invalid index

    Part* part = new Part(_score);
    part->setPlainLongName(tR->name());
    part->setPartName(tR->name());
    part->setId(idx);

    _score->appendPart(part);
    for (size_t staffIdx = 0; staffIdx < tR->staffCount(); staffIdx++) {
        Staff* s = Factory::createStaff(part);
        StaffType stType;
        stType.fretFont();
        _score->appendStaff(s);
    }

    if (tR->staffCount() > 1) {
        part->staff(0)->addBracket(mu::engraving::Factory::createBracketItem(_score->dummy(), BracketType::BRACE, 2));
        part->staff(0)->setBarLineSpan(2);
    }

    part->setMidiProgram(programm);
    part->setMidiChannel(midiChannel);

    int vol_val = static_cast<int>(std::lround(tR->rse().volume * 127));
    part->instrument()->channel(0)->setVolume(std::clamp(vol_val, 0, 127));

    int pan_val = static_cast<int>(std::lround(tR->rse().pan * 127));
    part->instrument()->channel(0)->setPan(std::clamp(pan_val, 0, 127));

    if (midiChannel == 9) {
        part->instrument()->setDrumset(gpDrumset);
        part->setShortName(tR->instrument());
        Staff* staff = part->staff(0);
        staff->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
        part->instrument()->setDrumset(gpDrumset);
    }

    if (tR->staffCount() == 1) {  //MSCore can set tunning only for one staff
        std::vector<int> standartTuning = { 40, 45, 50, 55, 59, 64 };

        if (!tR->staffProperty().empty()) {
            auto staffProperty = tR->staffProperty();

            int capoFret = staffProperty[0].capoFret;
            part->staff(0)->insertIntoCapoList({ 0, 1 }, capoFret);
//            part->setCapoFret(capoFret);

            auto tunning = staffProperty[0].tunning;
            auto fretCount = staffProperty[0].fretCount;

            if (tunning.empty()) {
                tunning = standartTuning;
            }

            int transpose = tR->transponce();
            for (auto& t : tunning) {
                t -= transpose;
            }

            StringData stringData = StringData(fretCount, static_cast<int>(tunning.size()), tunning.data());

            part->instrument()->setStringData(stringData);
        } else {
            StringData stringData = StringData(24, static_cast<int>(standartTuning.size()), standartTuning.data());
            part->instrument()->setStringData(stringData);
//            part->staff(0)->insertIntoCapoList({0, 1}, 0);
//            part->setCapoFret(0);
        }
        part->instrument()->setSingleNoteDynamics(false);
    }

    // this code is almost a direct copy-paste code from android_improvement branch.
    // it sets score lyrics from the first processed track.
//    if (_score->OffLyrics.isEmpty())
//        _score->OffLyrics = tR->lyrics();

    part->instrument()->setTranspose(tR->transponce());
}

void GPConverter::collectTempoMap(const GPMasterTracks* mTr)
{
    for (const auto& temp : mTr->tempoMap()) {
        _tempoMap.insert(std::make_pair(temp.bar, temp));
    }
}

void GPConverter::collectFermatas(const GPMasterBar* mB, Measure* measure)
{
    for (const auto& ferm : mB->fermatas()) {
        _fermatas.push_back(std::make_pair(measure, ferm));
    }
}

void GPConverter::fillUncompletedMeasure(const Context& ctx)
{
    Measure* lastMeasure = _score->lastMeasure();
    int tickOffset = lastMeasure->ticks().ticks() + lastMeasure->tick().ticks() - ctx.curTick.ticks();
    if (tickOffset > 0) {
        _score->setRest(ctx.curTick, ctx.curTrack, Fraction::fromTicks(tickOffset), true, nullptr);
    }
}

void GPConverter::addContiniousSlideHammerOn()
{
    auto searchEndNote = [] (Note* start) -> Note* {
        ChordRest* nextCr;
        if (static_cast<Chord*>(start->parent())->ChordRest::isGrace()) {
            //! this case when start note is a grace note so end note can be next note in grace notes
            //! or parent note of grace notes
            Chord* startChord =  static_cast<Chord*>(start->parent());

            Chord* parentGrace = static_cast<Chord*>(start->parent()->parent());

            auto it = parentGrace->graceNotes().begin();
            for (; it != parentGrace->graceNotes().end(); ++it) {
                if (*it == startChord) {
                    break;
                }
            }

            if (it == parentGrace->graceNotes().end()) {
                nextCr = nullptr;
            } else if (std::next(it) == parentGrace->graceNotes().end()) {
                nextCr = parentGrace;
            } else {
                nextCr = *(++it);
            }
        } else {
            nextCr = start->chord()->segment()->next1()->nextChordRest(start->track());
            if (!nextCr) {
                return nullptr;
            }

            if (nextCr->isChord() && !static_cast<Chord*>(nextCr)->graceNotes().empty()) {
                nextCr = static_cast<Chord*>(nextCr)->graceNotes().front();
            }
        }

        if (!nextCr) {
            return nullptr;
        }

        if (nextCr->type() != ElementType::CHORD) {
            return nullptr;
        }
        auto nextChord = static_cast<Chord*>(nextCr);
        for (auto note : nextChord->notes()) {
            if (note->string() == start->string() && (note->harmonic() == start->harmonic())) {
                return note;
            }
        }
        return nullptr;
    };

    std::unordered_map<Note*, Slur*> legatoSlides;
    for (const auto& slide : _slideHammerOnMap) {
        Note* startNote = slide.first;
        Note* endNote = searchEndNote(startNote);
        if (!endNote || startNote->string() == -1 || startNote->fret() == endNote->fret()) {
            //mistake in GP: such kind od slides shouldn't exist
            continue;
        }

        if (SlideHammerOn::HammerOn == slide.second) {
            endNote->setIsHammerOn(true);
        }

        Fraction startTick = startNote->chord()->tick();
        Fraction endTick = endNote->chord()->tick();
        int track = startNote->track();

        /// Layout info
        if (slide.second == SlideHammerOn::LegatoSlide || slide.second == SlideHammerOn::Slide) {
            Glissando* gl = mu::engraving::Factory::createGlissando(_score->dummy());
            gl->setPlayGlissando(false);
            gl->setAnchor(Spanner::Anchor::NOTE);
            gl->setStartElement(startNote);
            gl->setTrack(track);
            gl->setTick(startTick);
            gl->setTick2(endNote->chord()->tick());
            gl->setEndElement(endNote);
            gl->setParent(startNote);
            gl->setText("");
            gl->setGlissandoType(GlissandoType::STRAIGHT);
            _score->addElement(gl);
        }

        if (slide.second == SlideHammerOn::LegatoSlide || slide.second == SlideHammerOn::HammerOn) {
            if (legatoSlides.count(startNote) == 0) {
                Slur* slur = mu::engraving::Factory::createSlur(_score->dummy());
                slur->setStartElement(startNote->chord());
                slur->setTrack(track);
                slur->setTick(startTick);
                slur->setTick2(endTick);
                legatoSlides[endNote] = slur;
                slur->setEndElement(endNote->chord());
                _score->addSpanner(slur);
            } else {
                Slur* slur = legatoSlides[startNote];
                slur->setTick2(endTick);
                slur->setEndElement(endNote->chord());
                legatoSlides.erase(startNote);
                legatoSlides[endNote] = slur;
            }

            // TODO-gp: implement for editing too. Now works just for import.
            if (slide.second == SlideHammerOn::HammerOn) {
                Measure* measure = startNote->chord()->measure();

                auto midTick = (startTick + endTick) / 2;
                Segment* segment = measure->getSegment(SegmentType::ChordRest, midTick);
                StaffText* staffText = Factory::createStaffText(segment);
                QString hammerText = (startNote->pitch() > endNote->pitch()) ? "P" : "H";

                staffText->setPlainText(hammerText);
                staffText->setTrack(track);
                segment->add(staffText);
            }
        }

        /// Sound info
        if (slide.second == SlideHammerOn::LegatoSlide
            || slide.second == SlideHammerOn::Slide) {
            Note::SlideType slideType = (slide.second == SlideHammerOn::Slide
                                         ? Note::SlideType::Shift
                                         : Note::SlideType::Legato);
            Note::Slide sl{ slideType, startNote, endNote };
            startNote->attachSlide(sl);
            endNote->relateSlide(*startNote);
        }
    }
}

void GPConverter::addFermatas()
{
    auto fermataType = [](const GPMasterBar::Fermata& fer) {
        if (fer.type == GPMasterBar::Fermata::Type::Long) {
            return SymId::fermataLongAbove;
        } else if (fer.type == GPMasterBar::Fermata::Type::Short) {
            return SymId::fermataShortAbove;
        }
        return SymId::fermataAbove;
    };

    for (const auto& fr : _fermatas) {
        const auto& measure = fr.first;
        const auto& gpFermata = fr.second;
        Fraction tick = Fraction::fromTicks(Ms::Constant::division * gpFermata.offsetEnum / gpFermata.offsetDenum);
        // bellow how gtp fermata timeStretch converting to MU timeStretch
        float convertingLength = 1.5f - gpFermata.lenght * 0.5f + gpFermata.lenght * gpFermata.lenght * 3;
        Segment* seg = measure->getSegmentR(SegmentType::ChordRest, tick);

        for (int staffIdx = 0; staffIdx < _score->staves().count(); staffIdx++) {
            Fermata* fermata = mu::engraving::Factory::createFermata(seg);
            SymId type = fermataType(fr.second);
            fermata->setSymId(type);
            fermata->setTimeStretch(convertingLength);
            fermata->setTrack(staffIdx * VOICES);
            seg->add(fermata);
        }
    }
}

void GPConverter::addTempoMap()
{
    auto realTempo = [](const GPMasterTracks::Automation& temp) {
        //real tempo - beats per second
        //formula ro convert tempo from GTP tempo values
        int tempo = temp.value;

        if (temp.tempoUnit == 0) {
            return tempo;
        }

        if (temp.tempoUnit != 5) {
            return tempo * temp.tempoUnit / 2;
        }

        if (temp.tempoUnit == 5) {
            return tempo + tempo * 2;
        }

        return tempo;
    };

    int measureIdx = 0;
    TempoChangeRanged* _lastTempoChangeRanged = nullptr;
    int previousTempo = -1;

    for (auto m = _score->firstMeasure(); m; m = m->nextMeasure()) {
        auto range = _tempoMap.equal_range(measureIdx); //one measure can have several tempo changing
        measureIdx++;
        for (auto tempIt = range.first; tempIt != range.second; tempIt++) {
            Fraction tick = m->tick() + Fraction::fromTicks(
                tempIt->second.position * Constant::division * 4 * m->ticks().numerator() / m->ticks().denominator());
            Segment* segment = m->getSegment(SegmentType::ChordRest, tick);
            int realTemp = realTempo(tempIt->second);
            TempoText* tt = Factory::createTempoText(segment);
            tt->setTempo((qreal)realTemp / 60);
            tt->setXmlText(QString("<sym>metNoteQuarterUp</sym> = %1").arg(realTemp));
            tt->setTrack(0);
            segment->add(tt);
            _score->setTempo(tick, tt->tempo());

            if (_lastTempoChangeRanged) {
                _lastTempoChangeRanged->setTick2(tick);
                if (realTemp > previousTempo) {
                    _lastTempoChangeRanged->setTempoChangeType(TempoTechniqueType::Accelerando);
                    _lastTempoChangeRanged->setBeginText("accel");
                } else {
                    _lastTempoChangeRanged->setTempoChangeType(TempoTechniqueType::Rallentando);
                    _lastTempoChangeRanged->setBeginText("rall");
                }

                _score->addElement(_lastTempoChangeRanged);
                _lastTempoChangeRanged = nullptr;
            }

            if (tempIt->second.linear) {
                TempoChangeRanged* tempoChangeRanged = Factory::createTempoChangeRanged(segment);
                tempoChangeRanged->setTick(tick);
                tempoChangeRanged->setTrack(0);
                _lastTempoChangeRanged = tempoChangeRanged;
            }

            previousTempo = realTemp;
        }
    }
}

bool GPConverter::addSimileMark(const GPBar* bar, int curTrack)
{
    if (bar->simileMark() == GPBar::SimileMark::None) {
        return false;
    }

    Measure* measure = _score->lastMeasure();
    Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
    int staffIdx = track2staff(curTrack);

    switch (bar->simileMark()) {
    case GPBar::SimileMark::Simple: {
        MeasureRepeat* mr = Factory::createMeasureRepeat(segment);
        mr->setTrack(curTrack);
        mr->setTicks(measure->ticks());
        mr->setNumMeasures(1);
        segment->add(mr);
        measure->setMeasureRepeatCount(1, staffIdx);
        break;
    }
    case GPBar::SimileMark::FirstOfDouble: {
        MeasureRepeat* mr = Factory::createMeasureRepeat(segment);
        mr->setTrack(staff2track(staffIdx));
        mr->setTicks(measure->ticks());
        mr->setNumMeasures(2);
        segment->add(mr);
        measure->setMeasureRepeatCount(1, staffIdx);
        break;
    }
    case GPBar::SimileMark::SecondOfDouble: {
        Rest* r = Factory::createRest(segment);
        r->setTrack(staff2track(staffIdx));
        r->setTicks(measure->ticks());
        r->setDurationType(DurationType::V_MEASURE);
        segment->add(r);
        measure->setMeasureRepeatCount(2, staffIdx);
        break;
    }
    case GPBar::SimileMark::None:
        break;
    }
    return true;
}

void GPConverter::addClef(const GPBar* bar, int curTrack)
{
    auto convertClef = [](GPBar::Clef cl) {
        if (cl.type == GPBar::ClefType::Neutral) {
            return ClefType::PERC;
        } else if (cl.type == GPBar::ClefType::G2) {
            switch (cl.ottavia) {
            case GPBar::OttaviaType::va8:
                return ClefType::G8_VA;
            case GPBar::OttaviaType::vb8:
                return ClefType::G8_VB;
            case GPBar::OttaviaType::ma15:
                return ClefType::G15_MA;
            case GPBar::OttaviaType::mb15:
                return ClefType::G15_MB;
            default:
                return ClefType::G;
            }
        } else if (cl.type == GPBar::ClefType::F4) {
            switch (cl.ottavia) {
            case GPBar::OttaviaType::va8:
                return ClefType::F_8VA;
            case GPBar::OttaviaType::vb8:
                return ClefType::F8_VB;
            case GPBar::OttaviaType::ma15:
                return ClefType::F_15MA;
            case GPBar::OttaviaType::mb15:
                return ClefType::F15_MB;
            default:
                return ClefType::F;
            }
        } else if (cl.type == GPBar::ClefType::C3) {
            return ClefType::C3;
        } else if (cl.type == GPBar::ClefType::C4) {
            return ClefType::C4;
        }
        return ClefType::G;
    };

    if (_clefs.count(curTrack)) {
        if (_clefs.at(curTrack) == bar->clef()) {
            return;
        }
    }

    ClefType clef = convertClef(bar->clef());
    auto lastMeasure = _score->lastMeasure();

    auto tick = lastMeasure->tick();
    Segment* s = lastMeasure->getSegment(SegmentType::HeaderClef, tick);
    Clef* cl = mu::engraving::Factory::createClef(_score->dummy()->segment());
    cl->setTrack(curTrack);
    cl->setClefType(clef);

    s->add(cl);
    _clefs[curTrack] = bar->clef();
}

Measure* GPConverter::addMeasure(const GPMasterBar* mB)
{
    Fraction tick{ 0, 1 };
    auto lastMeasure = _score->measures()->last();
    if (lastMeasure) {
        tick = lastMeasure->tick() + lastMeasure->ticks();
    }

    Measure* measure = Factory::createMeasure(_score->dummy()->system());
    measure->setTick(tick);
    GPMasterBar::TimeSig sig = mB->timeSig();
    auto scoreTimeSig = Fraction(sig.enumerator, sig.denumerator);
    measure->setTimesig(scoreTimeSig);
    measure->setTicks(scoreTimeSig);
    _score->measures()->add(measure);

    return measure;
}

ChordRest* GPConverter::addChordRest(const GPBeat* beat, const Context& ctx)
{
    auto rhytm = [](GPRhytm::RhytmType rhytm) {
        if (rhytm == GPRhytm::RhytmType::Whole) {
            return 1;
        } else if (rhytm == GPRhytm::RhytmType::Half) {
            return 2;
        } else if (rhytm == GPRhytm::RhytmType::Quarter) {
            return 4;
        } else if (rhytm == GPRhytm::RhytmType::Eighth) {
            return 8;
        } else if (rhytm == GPRhytm::RhytmType::Sixteenth) {
            return 16;
        } else if (rhytm == GPRhytm::RhytmType::ThirtySecond) {
            return 32;
        } else {
            return 64;
        }
    };

    ChordRest* cr{ nullptr };
    if (beat->isRest()) {
        cr = Factory::createRest(_score->dummy()->segment());
    } else {
        cr = Factory::createChord(_score->dummy()->segment());
    }

    cr->setTrack(ctx.curTrack);

    Fraction fr(1, rhytm(beat->lenth().second));
    for (int dot = 1; dot <= beat->lenth().first; dot++) {
        fr += Fraction(1, rhytm(beat->lenth().second) * 2 * dot);
    }

    cr->setDurationType(TDuration(fr));
    cr->setTicks(fr);

    return cr;
}

void GPConverter::addFingering(const GPNote* gpnote, Note* note)
{
    if (gpnote->leftFingering().isEmpty() && gpnote->rightFingering().isEmpty()) {
        return;
    }

    auto scoreFinger = [](const auto& str) {
        if (str == "Open") {
            return QString("O");
        }
        return str;
    };

    if (!gpnote->leftFingering().isEmpty()) {
        Fingering* f = Factory::createFingering(note);
        f->setPlainText(scoreFinger(gpnote->leftFingering()));
        note->add(f);
    }
    if (!gpnote->rightFingering().isEmpty()) {
        Fingering* f = Factory::createFingering(note);
        f->setPlainText(gpnote->rightFingering());
        note->add(f);
    }
}

void GPConverter::addTrill(const GPNote* gpnote, Note* note)
{
    if (gpnote->trill().auxillaryFret == -1) {
        return;
    }

    Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
    art->setSymId(SymId::ornamentTrill);
    if (!note->score()->toggleArticulation(note, art)) {
        delete art;
    }
}

void GPConverter::addOrnament(const GPNote* gpnote, Note* note)
{
    if (gpnote->ornament() == GPNote::Ornament::None) {
        return;
    }

    auto scoreOrnament = [](const auto& orn) {
        if (orn == GPNote::Ornament::Turn) {
            return SymId::ornamentTurn;
        } else if (orn == GPNote::Ornament::InvertedTurn) {
            return SymId::ornamentTurn;
        } else if (orn == GPNote::Ornament::LowerMordent) {
            return SymId::ornamentMordent;
        } else if (orn == GPNote::Ornament::UpperMordent) {
            return SymId::ornamentShortTrill;
        }
        return SymId::ornamentUpPrall;
    };

    Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
    art->setSymId(scoreOrnament(gpnote->ornament()));
    if (!_score->toggleArticulation(note, art)) {
        delete art;
    }
}

void GPConverter::addVibratoLeftHand(const GPNote* gpnote, Note* note)
{
    if (gpnote->vibratoType() == GPNote::VibratoType::None) {
        return;
    }

    auto scoreVibratoType = [](GPNote::VibratoType gpType) {
        switch (gpType) {
        case GPNote::VibratoType::Slight:
            return Vibrato::Type::GUITAR_VIBRATO;
        case GPNote::VibratoType::Wide:
            return Vibrato::Type::GUITAR_VIBRATO_WIDE;
        default:
            return Vibrato::Type::GUITAR_VIBRATO;
        }
    };

    Vibrato::Type vibratoType = scoreVibratoType(gpnote->vibratoType());
    addVibratoByType(note, vibratoType);
}

void GPConverter::addHarmonic(const GPNote* gpnote, Note* note)
{
    if (gpnote->harmonic().type == GPNote::Harmonic::Type::None) {
        return;
    }

    if (gpnote->harmonic().type != GPNote::Harmonic::Type::Natural) {
        Note* hnote = mu::engraving::Factory::createNote(_score->dummy()->chord());
        hnote->setTrack(note->track());
        hnote->setString(note->string());
        hnote->setFret(note->fret());
        hnote->setPitch(note->pitch());
        hnote->setTpcFromPitch();
        note->chord()->add(hnote);
        hnote->setPlay(false);
        addTie(gpnote, hnote);
    }

    int gproHarmonicType = static_cast<int>(gpnote->harmonic().type);
    int harmonicFret = GuitarPro::harmonicOvertone(note, gpnote->harmonic().fret, gproHarmonicType);
    int string = note->string();
    int harmonicPitch = note->part()->instrument()->stringData()->getPitch(string, harmonicFret, nullptr);
    note->setPitch(harmonicPitch);
    note->setTpcFromPitch();
    note->setHarmonic(true);

    auto harmonicText = [](const GPNote::Harmonic::Type& h) {
        if (h == GPNote::Harmonic::Type::Artificial) {
            return QString("A.H.");
        } else if (h == GPNote::Harmonic::Type::Pinch) {
            return QString("P.H.");
        } else if (h == GPNote::Harmonic::Type::Tap) {
            return QString("T.H.");
        } else if (h == GPNote::Harmonic::Type::Semi) {
            return QString("S.H.");
        } else if (h == GPNote::Harmonic::Type::FeedBack) {
            return QString("Fdbk.");
        } else {
            return QString("");
        }
    };

    addTextToNote(harmonicText(gpnote->harmonic().type), note);
}

void GPConverter::configureNote(const GPNote* gpnote, Note* note)
{
    setPitch(note, gpnote->midiPitch());

    addBend(gpnote, note);

    addLetRing(gpnote, note);
    addPalmMute(gpnote, note);
    note->setGhost(gpnote->ghostNote());
    note->setDeadNote(gpnote->muted());
}

void GPConverter::addAccent(const GPNote* gpnote, Note* note)
{
    if (gpnote->accents().none()) {
        return;
    }

    auto accentType = [](size_t flagIdx) {
        if (flagIdx == 0) {
            return SymId::articStaccatoAbove;
        } else if (flagIdx == 2) {
            return SymId::articMarcatoAbove;
        } else if (flagIdx == 3) {
            return SymId::articAccentAbove;
        } else {
            return SymId::dynamicSforzando;
        }
    };

    for (size_t flagIdx = 0; flagIdx < gpnote->accents().size(); flagIdx++) {
        if (gpnote->accents()[flagIdx]) {
            Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
            art->setSymId(accentType(flagIdx));
            note->chord()->add(art);
        }
    }
}

void GPConverter::addLeftHandTapping(const GPNote* gpnote, Note* note)
{
    if (!gpnote->leftHandTapped()) {
        return;
    }

    Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
    art->setSymId(SymId::guitarLeftHandTapping);
    if (!note->score()->toggleArticulation(note, art)) {
        delete art;
    }
}

void GPConverter::addTapping(const GPNote* gpnote, Note* note)
{
    if (!gpnote->tapping()) {
        return;
    }

    addTextToNote("T", note);
}

void GPConverter::addSlide(const GPNote* gpnote, Note* note)
{
    if (gpnote->slides().none()) {
        return;
    }

    addSingleSlide(gpnote, note);
    collectContiniousSlide(gpnote, note);
}

void GPConverter::addSingleSlide(const GPNote* gpnote, Note* note)
{
    auto slideType = [](size_t flagIdx) -> std::pair<ChordLineType, Note::SlideType> {
        if (flagIdx == 2) {
            return { ChordLineType::FALL, Note::SlideType::Fall };
        } else if (flagIdx == 3) {
            return { ChordLineType::DOIT, Note::SlideType::Doit };
        } else if (flagIdx == 4) {
            return { ChordLineType::SCOOP, Note::SlideType::Lift };
        } else {
            return { ChordLineType::PLOP, Note::SlideType::Plop };
        }
    };

    for (size_t flagIdx = 2; flagIdx < gpnote->slides().size(); flagIdx++) {
        if (gpnote->slides()[flagIdx]) {
            auto type = slideType(flagIdx);

            Slide* slide = mu::engraving::Factory::createSlide(_score->dummy()->chord());
            slide->setChordLineType(type.first);
            note->chord()->add(slide);

            slide->setNote(note);

            Note::Slide sl{ type.second, nullptr };
            note->attachSlide(sl);
        }
    }
}

void GPConverter::collectContiniousSlide(const GPNote* gpnote, Note* note)
{
    if (gpnote->slides()[0]) {
        _slideHammerOnMap.push_back(std::pair(note, SlideHammerOn::Slide));
    }
    if (gpnote->slides()[1]) {
        _slideHammerOnMap.push_back(std::pair(note, SlideHammerOn::LegatoSlide));
    }
}

void GPConverter::collectHammerOn(const GPNote* gpnote, Note* note)
{
    if (gpnote->hammerOn() == GPNote::HammerOn::Start) {
        _slideHammerOnMap.push_back(std::pair(note, SlideHammerOn::HammerOn));
    }
}

void GPConverter::addBend(const GPNote* gpnote, Note* note)
{
    if (!gpnote->bend()) {
        return;
    }

    Bend* bend = mu::engraving::Factory::createBend(note);
    auto gpBend = gpnote->bend();

    bool bendHasMiddleValue{ true };
    if (gpBend->middleOffset1 == 12 || gpBend->middleOffset2 == 12) {
        bendHasMiddleValue = false;
    }

    bend->points().append(PitchValue(0, gpBend->originValue));

    const auto& lastPoint = bend->points().back();

    if (bendHasMiddleValue) {
        if (PitchValue value(gpBend->middleOffset1, gpBend->middleValue);
            gpBend->middleOffset1 >= 0 && gpBend->middleOffset1 < gpBend->destinationOffset && value != lastPoint) {
            bend->points().append(std::move(value));
        }
        if (PitchValue value(gpBend->middleOffset2, gpBend->middleValue);
            gpBend->middleOffset2 >= 0 && gpBend->middleOffset2 != gpBend->middleOffset1
            && gpBend->middleOffset2 < gpBend->destinationOffset
            && value != lastPoint) {
            bend->points().append(std::move(value));
        }

        if (gpBend->middleOffset1 == -1 && gpBend->middleOffset2 == -1 && gpBend->middleValue != -1) {
            //!@NOTE It seems when middle point is places exatly in the middle
            //!of bend  GP6 stores this value equal -1
            if (gpBend->destinationOffset > 50 || gpBend->destinationOffset == -1) {
                bend->points().append(PitchValue(50, gpBend->middleValue));
            }
        }
    }

    if (gpBend->destinationOffset <= 0) {
        bend->points().append(PitchValue(100, gpBend->destinationValue)); //! In .gpx this value might be exist
    } else {
        if (PitchValue value(gpBend->destinationOffset, gpBend->destinationValue); value != lastPoint) {
            bend->points().append(std::move(value));
        }
    }

    bend->setTrack(note->track());
    note->add(bend);
}

void GPConverter::addLetRing(const GPNote* gpnote, Note* note)
{
    if (!gpnote->letRing()) {
        return;
    }

    int track = note->track();
    while (int(_letRings.size()) < track + 1) {
        _letRings.push_back(0);
    }

    Chord* chord = note->chord();
    if (_letRings[track]) {
        LetRing* lr      = _letRings[track];
        Chord* lastChord = toChord(lr->endCR());
        if (lastChord == note->chord()) {
            return;
        }
        //
        // extend the current "let ring" or start a new one
        //
        Fraction tick = note->chord()->segment()->tick();
        if (lr->tick2() < tick) {
            _letRings[track] = 0;
        } else {
            lr->setTick2(chord->tick() + chord->actualTicks());
            lr->setEndElement(chord);
        }
    }
    if (!_letRings[track]) {
        LetRing* lr = Factory::createLetRing(_score->dummy());
        _letRings[track] = lr;
        Segment* segment = chord->segment();
        Fraction tick = segment->tick();

        lr->setTick(tick);
        lr->setTick2(tick + chord->actualTicks());
        lr->setTrack(track);
        lr->setTrack2(track);
        lr->setStartElement(chord);
        lr->setEndElement(chord);
        _score->addElement(lr);
    }
}

void GPConverter::addPalmMute(const GPNote* gpnote, Note* note)
{
    if (!gpnote->palmMute()) {
        return;
    }

    int track = note->track();
    while (int(_palmMutes.size()) < track + 1) {
        _palmMutes.push_back(0);
    }

    Chord* chord = note->chord();
    if (_palmMutes[track]) {
        PalmMute* pm = _palmMutes[track];
        Chord* lastChord = toChord(pm->endCR());
        if (lastChord == note->chord()) {
            return;
        }
        //
        // extend the current palm mute or start a new one
        //
        Fraction tick = note->chord()->segment()->tick();
        if (pm->tick2() < tick) {
            _palmMutes[track] = 0;
        } else {
            pm->setTick2(chord->tick() + chord->actualTicks());
            pm->setEndElement(chord);
        }
    }
    if (!_palmMutes[track]) {
        PalmMute* pm = Factory::createPalmMute(_score->dummy());
        _palmMutes[track] = pm;
        Segment* segment = chord->segment();
        Fraction tick = segment->tick();

        pm->setTick(tick);
        pm->setTick2(tick + chord->actualTicks());
        pm->setTrack(track);
        pm->setTrack2(track);
        pm->setStartElement(chord);
        pm->setEndElement(chord);
        _score->addElement(pm);
    }
}

void GPConverter::setPitch(Note* note, const GPNote::MidiPitch& midiPitch)
{
    int32_t fret = midiPitch.fret;
    int32_t musescoreString{ -1 };
    if (midiPitch.string != -1) {
        musescoreString = note->part()->instrument()->stringData()->strings() - 1 - midiPitch.string;
    }

    int pitch = 0;
    if (midiPitch.midi != -1) {
        pitch = midiPitch.midi;
    } else if (midiPitch.octave != -1 || midiPitch.tone != 0) {
        pitch = midiPitch.octave * 12 + midiPitch.tone;
    } else if (midiPitch.variation != -1) {
        pitch = calculateDrumPitch(midiPitch.element, midiPitch.variation, note->part()->shortName());
    } else if (note->part()->instrument()->channel(0)->channel() == 9) {
        //!@NOTE This is a case, when part of the note is a
        //       single drum instrument. It seems, that GP
        //       sets to -1 all midi parameters for the note,
        //       but sets the midi pitch value to the program
        //       instead.
        pitch = note->part()->instrument()->channel(0)->program();
    } else {
        pitch
            = note->part()->instrument()->stringData()->getPitch(musescoreString, midiPitch.fret,
                                                                 nullptr) + note->part()->instrument()->transpose().chromatic;
    }

    if (musescoreString == -1) {
        musescoreString = getStringNumberFor(note, pitch);

        fret = note->part()->instrument()->stringData()->fret(pitch, musescoreString, nullptr);
    }

    note->setFret(fret);
    note->setString(musescoreString);

    note->setPitch(pitch);
    note->setTpcFromPitch();
}

int GPConverter::calculateDrumPitch(int element, int variation, const QString& /*instrumentName*/)
{
    //!@NOTE copied from importgtp-gp6.cpp.

    int pitch = 44;
    /* These numbers below were determined by creating all drum
     * notes in a GPX format file and then analyzing the score.gpif
     * file which specifies the score and then matching as much
     * as possible with the gpDrumset...   */
    if (element == 11 && variation == 0) {  // pedal hihat
        pitch = 44;
    } else if (element == 0 && variation == 0) { // Kick (hit)
        pitch = 36;     // or 36
    } else if (element == 5 && variation == 0) { // Tom very low (hit)
        pitch = 41;
    } else if (element == 6 && variation == 0) { // Tom low (hit)
        pitch = 43;
    } else if (element == 7 && variation == 0) { // Tom medium (hit)
        pitch = 45;
    } else if (element == 1 && variation == 0) { // Snare (hit)
        pitch = 40;     //or 40
    } else if (element == 1 && variation == 1) { // Snare (rim shot)
        pitch = 91;
    } else if (element == 1 && variation == 2) { // Snare (side stick)
        pitch = 37;
    } else if (element == 8 && variation == 0) { // Tom high (hit)
        pitch = 48;
    } else if (element == 9 && variation == 0) { // Tom very high (hit)
        pitch = 50;
    } else if (element == 15 && variation == 0) { // Ride (middle)
        pitch = 51;
    } else if (element == 15 && variation == 1) { // Ride (edge)
        pitch = 59;
    } else if (element == 15 && variation == 2) { // Ride (bell)
        pitch = 59;
    } else if (element == 10 && variation == 0) { // Hihat (closed)
        pitch = 42;
    } else if (element == 10 && variation == 1) { // Hihat (half)
        pitch = 46;
    } else if (element == 10 && variation == 2) { // Hihat (open)
        pitch = 46;
    } else if (element == 12 && variation == 0) { // Crash medium (hit)
        pitch = 49;
    } else if (element == 14 && variation == 0) { // Splash (hit)
        pitch = 55;
    } else if (element == 13 && variation == 0) { // Crash high (hit)
        pitch = 57;
    } else if (element == 16 && variation == 0) { // China (hit)
        pitch = 52;
    } else if (element == 4 && variation == 0) { // Cowbell high (hit)
        pitch = 102;
    } else if (element == 3 && variation == 0) { // Cowbell medium (hit)
        pitch = 56;
    } else if (element == 2 && variation == 0) { // Cowbell low (hit)
        pitch = 99;
    }

    return pitch;
}

void GPConverter::addDynamic(const GPBeat* gpb, ChordRest* cr)
{
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    if (_dynamics.count(cr->track())) {
        if (_dynamics.at(cr->track()) == gpb->dynamic()) {
            return;
        }
    }

    auto convertDynamic = [](GPBeat::DynamicType t) {
        if (t == GPBeat::DynamicType::FFF) {
            return "fff";
        } else if (t == GPBeat::DynamicType::FF) {
            return "ff";
        } else if (t == GPBeat::DynamicType::FF) {
            return "ff";
        } else if (t == GPBeat::DynamicType::F) {
            return "f";
        } else if (t == GPBeat::DynamicType::MF) {
            return "mf";
        } else if (t == GPBeat::DynamicType::MP) {
            return "mp";
        } else if (t == GPBeat::DynamicType::P) {
            return "p";
        } else if (t == GPBeat::DynamicType::PP) {
            return "pp";
        }
        return "ppp";
    };

    Dynamic* dynamic = Factory::createDynamic(_score->dummy()->segment());
    dynamic->setTrack(cr->track());
    dynamic->setDynamicType(convertDynamic(gpb->dynamic()));
    cr->segment()->add(dynamic);
    _dynamics[cr->track()] = gpb->dynamic();
}

void GPConverter::addTie(const GPNote* gpnote, Note* note)
{
    if (gpnote->tieType() == GPNote::TieType::None) {
        return;
    }

    using tieMap = std::unordered_multimap<int, Tie*>;

    auto startTie = [](Note* note, Score* sc, tieMap& ties, int curTrack) {
        Tie* tie = Factory::createTie(sc->dummy());
        note->add(tie);
        ties.insert(std::make_pair(curTrack, tie));
    };

    auto endTie = [](Note* note, tieMap& ties, int curTrack) {
        auto range = ties.equal_range(curTrack);
        for (auto it = range.first; it != range.second; it++) {
            Tie* tie = it->second;
            if (tie->startNote()->pitch() == note->pitch()) {
                tie->setEndNote(note);
                note->setTieBack(tie);
                ties.erase(it);
                break;
            }
        }
    };

    if (gpnote->tieType() == GPNote::TieType::Start) {
        startTie(note, _score, _ties, note->track());
    } else if (gpnote->tieType() == GPNote::TieType::Mediate) {
        endTie(note, _ties, note->track());
        startTie(note, _score, _ties, note->track());
    } else if (gpnote->tieType() == GPNote::TieType::End) {
        endTie(note, _ties, note->track());
    }
}

void GPConverter::addLegato(const GPBeat* beat, ChordRest* cr)
{
    if (beat->legatoType() == GPBeat::LegatoType::None) {
        return;
    }

    using slurMap = std::unordered_map<int, Slur*>;

    auto startSlur = [](ChordRest* cr, Score* sc, slurMap& slurs, int curTrack, Fraction curTick) {
        if (auto it = slurs.find(curTrack); it != std::end(slurs)) {
            slurs.erase(it);
        }

        Slur* slur = Factory::createSlur(sc->dummy());
        slur->setStartElement(cr);
        slur->setTrack(curTrack);
        slur->setTick(curTick);
        slurs[curTrack] = slur;
        sc->addSpanner(slur);
    };

    auto mediateSlur = [](ChordRest* cr, slurMap& slurs, int curTrack, Fraction curTick) {
        Slur* slur = slurs[curTrack];
        if (!slur) {
            return;
        }
        slur->setTrack(curTrack);
        slur->setTrack2(curTrack);
        slur->setTick2(curTick);
        slur->setEndElement(cr);
    };

    auto endSlur = [](ChordRest* cr, slurMap& slurs, int curTrack, Fraction curTick) {
        Slur* slur = slurs[curTrack];
        if (!slur) {
            return;
        }
        slur->setTrack2(curTrack);
        slur->setTick2(curTick);
        slur->setEndElement(cr);
    };

    if (beat->legatoType() == GPBeat::LegatoType::Start) {
        startSlur(cr, _score, _slurs, cr->track(), cr->tick());
    } else if (beat->legatoType() == GPBeat::LegatoType::Mediate) {
        mediateSlur(cr, _slurs, cr->track(), cr->tick());
    } else if (beat->legatoType() == GPBeat::LegatoType::End) {
        endSlur(cr, _slurs, cr->track(), cr->tick());
    }
}

void GPConverter::addOttava(const GPBeat* gpb, ChordRest* cr)
{
    auto convertOttava = [](GPBeat::OttavaType t) {
        switch (t) {
        case GPBeat::OttavaType::va8: {
            return Ms::OttavaType::OTTAVA_8VA;
        }
        case GPBeat::OttavaType::vb8: {
            return Ms::OttavaType::OTTAVA_8VB;
        }
        case GPBeat::OttavaType::ma15: {
            return Ms::OttavaType::OTTAVA_15MA;
        }
        case GPBeat::OttavaType::mb15: {
            return Ms::OttavaType::OTTAVA_15MB;
        }
        case GPBeat::OttavaType::None:
            break;
        }
        LOGE() << "wrong ottava type";
        return Ms::OttavaType::OTTAVA_8VA;
    };

    if (gpb->ottavaType() == GPBeat::OttavaType::None) {
        if (_lastOttava) {
            _lastOttava->setTick2(cr->segment()->tick());
            _score->addElement(_lastOttava);
        }
        _lastOttava = nullptr;
        return;
    }

    auto newOttavaType = convertOttava(gpb->ottavaType());

    if (!_lastOttava || newOttavaType != _lastOttava->ottavaType()) {
        Ottava* ottava = Factory::createOttava(_score->dummy());
        ottava->setTrack(cr->track());
        ottava->setTick(cr->segment()->tick());
        ottava->setOttavaType(newOttavaType);
        if (_lastOttava) {
            _lastOttava->setTick2(cr->segment()->tick());
        }
        _lastOttava = ottava;
    }

    Chord* chord = static_cast<Chord*>(cr);
    if (!chord) {
        return;
    }

    auto type = _lastOttava->ottavaType();

    for (auto& note : chord->notes()) {
        int pitch = note->pitch();
        if (type == Ms::OttavaType::OTTAVA_8VA) {
            note->setPitch((pitch - 12 > 0) ? pitch - 12 : pitch);
        } else if (type == Ms::OttavaType::OTTAVA_8VB) {
            note->setPitch((pitch + 12 < 127) ? pitch + 12 : pitch);
        } else if (type == Ms::OttavaType::OTTAVA_15MA) {
            note->setPitch((pitch - 24 > 0) ? pitch - 24 : (pitch - 12 > 0 ? pitch - 12 : pitch));
        } else if (type == Ms::OttavaType::OTTAVA_15MB) {
            note->setPitch((pitch + 24 < 127) ? pitch + 24 : ((pitch + 12 < 127) ? pitch + 12 : pitch));
        }
    }
}

void GPConverter::addFretDiagram(const GPBeat* gpnote, ChordRest* cr, const Context& ctx)
{
    static int last_idx = -1;

    int GPTrackIdx = ctx.curTrack;
    int diaId = gpnote->diagramIdx(GPTrackIdx, ctx.masterBarIndex);

    if (last_idx == diaId) {
        return;
    }

    last_idx = diaId;

    if (diaId == -1) {
        return;
    }

    if (_gpDom->tracks().at(GPTrackIdx)->diagram().count(diaId) == 0) {
        return;
    }

    GPTrack::Diagram diagram = _gpDom->tracks().at(GPTrackIdx)->diagram().at(diaId);

    FretDiagram* fretDiagram = mu::engraving::Factory::createFretDiagram(_score->dummy()->segment());
    fretDiagram->setTrack(cr->track());
    //TODO-ws      fretDiagram->setChordName(name);
    fretDiagram->setStrings(diagram.stringCount);
    fretDiagram->setFretOffset(diagram.baseFret);

    for (int st = 0; st < diagram.stringCount; st++) {
        fretDiagram->setMarker(st, FretMarkerType::CROSS);
    }

    for (const auto& fret: diagram.frets) {
        if (fret.second == 0) {
            fretDiagram->setMarker(fret.first, FretMarkerType::CIRCLE);
        } else {
            fretDiagram->setDot(fret.first, fret.second);
        }
    }

    cr->segment()->add(fretDiagram);
}

void GPConverter::addSlapped(const GPBeat* beat, ChordRest* cr)
{
    if (!beat->slapped() || cr->type() != ElementType::CHORD) {
        return;
    }

    auto ch = static_cast<Chord*>(cr);
    addTextToNote("S", ch->upNote());
}

void GPConverter::addPopped(const GPBeat* beat, ChordRest* cr)
{
    if (!beat->popped() || cr->type() != ElementType::CHORD) {
        return;
    }

    auto ch = static_cast<Chord*>(cr);
    addTextToNote("P", ch->upNote());
}

void GPConverter::addBrush(const GPBeat* beat, ChordRest* cr)
{
    if (beat->brush() == GPBeat::Brush::None) {
        return;
    }

    auto brushType = [](auto&& gpArp) {
        if (gpArp == GPBeat::Brush::Up) {
            return ArpeggioType::DOWN_STRAIGHT;
        } else {
            return ArpeggioType::UP_STRAIGHT;
        }
    };

    Arpeggio* arp = mu::engraving::Factory::createArpeggio(_score->dummy()->chord());
    arp->setArpeggioType(brushType(beat->brush()));
    arp->setStretch(beat->arpeggioStretch());

    cr->add(arp);
}

void GPConverter::addArpeggio(const GPBeat* beat, ChordRest* cr)
{
    if (beat->arpeggio() == GPBeat::Arpeggio::None) {
        return;
    }

    auto arpeggioType = [](auto&& gpArp) {
        if (gpArp == GPBeat::Arpeggio::Up) {
            return ArpeggioType::DOWN;
        } else {
            return ArpeggioType::NORMAL;
        }
    };

    Arpeggio* arp = mu::engraving::Factory::createArpeggio(_score->dummy()->chord());
    arp->setArpeggioType(arpeggioType(beat->arpeggio()));
    arp->setStretch(beat->arpeggioStretch());
    cr->add(arp);
}

void GPConverter::addTimer(const GPBeat* beat, ChordRest* cr)
{
    if (beat->time() == -1) {
        return;
    }

    int time = beat->time();
    int minutes = time / 60;
    int seconds = time % 60;
    Text* st = Factory::createText(cr->segment());
    st->setPlainText(QString::number(minutes)
                     + ":"
                     + (seconds < 10 ? "0" + QString::number(seconds) : QString::number(seconds)));
    st->setParent(cr->segment());
    st->setTrack(cr->track());
    cr->segment()->add(st);
}

void GPConverter::addFreeText(const GPBeat* beat, ChordRest* cr)
{
    if (beat->freeText().isEmpty()) {
        return;
    }

    auto text = Factory::createStaffText(cr->segment());
    text->setTrack(cr->track());
    text->setPlainText(beat->freeText());
    cr->segment()->add(text);
}

void GPConverter::setupTupletStyle(Tuplet* tuplet)
{
    bool real = false;

    switch (tuplet->ratio().numerator()) {
    case 2: {
        real = (tuplet->ratio().denominator() == 3);
        break;
    }
    case 3:
    case 4: {
        real = (tuplet->ratio().denominator() == 2);
        break;
    }
    case 5:
    case 6:
    case 7: {
        real = (tuplet->ratio().denominator() == 4);
        break;
    }
    case 9:
    case 10:
    case 11:
    case 12:
    case 13: {
        real = (tuplet->ratio().denominator() == 8);
        break;
    }
    }
    if (!real) {
        tuplet->setNumberType(TupletNumberType::SHOW_RELATION);
        tuplet->setPropertyFlags(Pid::NUMBER_TYPE, PropertyFlags::UNSTYLED);
    }
}

void GPConverter::addTuplet(const GPBeat* beat, ChordRest* cr)
{
    if (beat->tuplet().num == -1) {
        _lastTuplet = nullptr;
        return;
    }

    if (_lastTuplet) {
        //!NOTE if new measure create new tuplet
        //! if new track create new tuplet
        //! if tuplet is full create new tuplet
        if (_lastTuplet->elements().back()->measure() != cr->measure()) {
            _lastTuplet = nullptr;
        } else if (_lastTuplet->track() != cr->track()) {
            _lastTuplet = nullptr;
        } else if (_lastTuplet->elementsDuration()
                   == _lastTuplet->baseLen().fraction() * _lastTuplet->ratio().numerator()) {
            _lastTuplet = nullptr;
        }
    }

    if (!_lastTuplet) {
        _lastTuplet = Factory::createTuplet(_score->dummy()->measure());
        _lastTuplet->setTrack(cr->track());
        _lastTuplet->setParent(cr->measure());
        _lastTuplet->setTrack(cr->track());
        _lastTuplet->setBaseLen(cr->actualDurationType());
        _lastTuplet->setRatio(Fraction(beat->tuplet().num, beat->tuplet().denum));
        _lastTuplet->setTicks(cr->actualDurationType().ticks() * beat->tuplet().denum);
    }

    setupTupletStyle(_lastTuplet);
    cr->setTuplet(_lastTuplet);
    _lastTuplet->add(cr);
}

void GPConverter::addVibratoByType(const Note* note, Vibrato::Type type)
{
    int track = note->track();
    while (int(_vibratos.size()) < track + 1) {
        _vibratos.push_back(0);
    }

    Chord* chord = note->chord();
    if (_vibratos[track]) {
        Vibrato* v      = _vibratos[track];
        if (v->vibratoType() == type) {
            Chord* lastChord = toChord(v->endCR());
            if (lastChord == note->chord()) {
                return;
            }
            //
            // extend the current "vibrato" or start a new one
            //
            Fraction tick = note->chord()->tick();
            if (v->tick2() < tick) {
                _vibratos[track] = 0;
            } else {
                v->setTick2(chord->tick() + chord->actualTicks());
                v->setEndElement(chord);
            }
        } else {
            _vibratos[track] = 0;
        }
    }
    if (!_vibratos[track]) {
        Vibrato* v = new Vibrato(_score->dummy());
        v->setVibratoType(type);
        _vibratos[track] = v;
        Segment* segment = chord->segment();
        Fraction tick = segment->tick();

        v->setTick(tick);
        v->setTick2(tick + chord->actualTicks());
        v->setTrack(track);
        v->setTrack2(track);
        v->setStartElement(chord);
        v->setEndElement(chord);
        _score->addElement(v);
    }
}

void GPConverter::addVibratoWTremBar(const GPBeat* beat, ChordRest* cr)
{
    if (beat->vibrato() == GPBeat::VibratoWTremBar::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        //this condition may be unnecessary
        //GP 6 and 7 do not allow create vibrato on the rest
        return;
    }

    auto scoreVibrato = [](GPBeat::VibratoWTremBar vr) {
        if (vr == GPBeat::VibratoWTremBar::Slight) {
            return Vibrato::Type::VIBRATO_SAWTOOTH;
        } else {
            return Vibrato::Type::VIBRATO_SAWTOOTH_WIDE;
        }
    };

    addVibratoByType(static_cast<Chord*>(cr)->upNote(), scoreVibrato(beat->vibrato()));
}

void GPConverter::addFadding(const GPBeat* beat, ChordRest* cr)
{
    if (beat->fadding() == GPBeat::Fadding::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    auto scoreFadding = [](const auto& fad) {
        if (fad == GPBeat::Fadding::FadeIn) {
            return SymId::guitarFadeIn;
        } else if (fad == GPBeat::Fadding::FadeOut) {
            return SymId::guitarFadeOut;
        } else {
            return SymId::guitarVolumeSwell;
        }
    };

    Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
    art->setSymId(scoreFadding(beat->fadding()));
    art->setAnchor(ArticulationAnchor::TOP_STAFF);
    if (!_score->toggleArticulation(static_cast<Chord*>(cr)->upNote(), art)) {
        delete art;
    }
}

void GPConverter::addHairPin(const GPBeat* beat, ChordRest* cr)
{
    if (beat->hairpin() == GPBeat::Hairpin::None) {
        _lastHairpin = nullptr;
        return;
    }

    auto scoreHairpin = [](const auto& h) {
        if (h == GPBeat::Hairpin::Crescendo) {
            return HairpinType::CRESC_HAIRPIN;
        } else {
            return HairpinType::DECRESC_HAIRPIN;
        }
    };

    if (!_lastHairpin) {
        _lastHairpin = Factory::createHairpin(_score->dummy()->segment());
        _lastHairpin->setTick(cr->tick());
        _lastHairpin->setTick2(cr->tick());
        _lastHairpin->setHairpinType(scoreHairpin(beat->hairpin()));
        _lastHairpin->setTrack(cr->track());
        _lastHairpin->setTrack2(cr->track());
        _score->addSpanner(_lastHairpin);
    }

    _lastHairpin->setTick2(cr->tick());
    _lastHairpin->setEndElement(cr);
}

void GPConverter::addRasgueado(const GPBeat* beat, ChordRest* cr)
{
    if (beat->rasgueado() == GPBeat::Rasgueado::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    addTextToNote("rasg.", static_cast<Chord*>(cr)->notes().front());
}

void GPConverter::addPickStroke(const GPBeat* beat, ChordRest* cr)
{
    if (beat->pickStroke() == GPBeat::PickStroke::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    auto scorePickStroke = [](const auto& p) {
        if (p == GPBeat::PickStroke::Up) {
            return SymId::stringsUpBow;
        } else {
            return SymId::stringsDownBow;
        }
    };

    Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
    art->setSymId(scorePickStroke(beat->pickStroke()));
    if (!_score->toggleArticulation(static_cast<Chord*>(cr)->upNote(), art)) {
        delete art;
    }
}

void GPConverter::addTremolo(const GPBeat* beat, ChordRest* cr)
{
    if (beat->tremolo().enumerator == -1) {
        return;
    }

    auto scoreTremolo = [](const GPBeat::Tremolo tr) {
        if (tr.denumerator == 2) {
            return TremoloType::R8;
        }
        if (tr.denumerator == 4) {
            return TremoloType::R16;
        } else {
            return TremoloType::R32;
        }
    };

    Tremolo* t = Factory::createTremolo(_score->dummy()->chord());
    t->setTremoloType(scoreTremolo(beat->tremolo()));
    cr->add(t);
}

void GPConverter::addWah(const GPBeat* beat, ChordRest* cr)
{
    if (beat->wah() == GPBeat::Wah::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    auto scoreWah = [] (const auto& wah) {
        if (wah == GPBeat::Wah::Open) {
            return SymId::brassMuteOpen;
        }
        return SymId::brassMuteClosed;
    };

    Articulation* art = Factory::createArticulation(_score->dummy()->chord());
    art->setSymId(scoreWah(beat->wah()));
    if (!_score->toggleArticulation(static_cast<Chord*>(cr)->upNote(), art)) {
        delete art;
    }
}

void GPConverter::addBarre(const GPBeat* beat, ChordRest* cr)
{
    if (beat->barre().fret == -1) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    auto barreType = [](const GPBeat::Barre& barre) {
        if (barre.string == 1) {
            return QString("1/2B ");
        } else {
            return QString("B ");
        }
    };

    QString barreFret;
    int fret = beat->barre().fret;

    for (int i = 0; i < (fret / 10); i++) {
        barreFret += "X";
    }
    int targetMod10 = fret % 10;
    if (targetMod10 == 9) {
        barreFret += "IX";
    } else if (targetMod10 == 4) {
        barreFret += "IV";
    } else {
        if (targetMod10 >= 5) {
            barreFret += "V";
            targetMod10-=5;
        }
        for (int j = 0; j < targetMod10; j++) {
            barreFret += "I";
        }
    }

    addTextToNote(barreType(beat->barre()) + barreFret, static_cast<Chord*>(cr)->upNote());
}

void GPConverter::addLyrics(const GPBeat* beat, ChordRest* cr, const Context& ctx)
{
    int GPTrackIdx = cr->part()->id();

    const std::string& lyrStr = beat->lyrics(GPTrackIdx, ctx.masterBarIndex);
    if (lyrStr.empty()) {
        return;
    }

    Lyrics* lyr = Factory::createLyrics(_score->dummy()->chord());
    lyr->setPlainText(QString::fromUtf8(lyrStr.c_str()));
    cr->add(lyr);
}

void GPConverter::clearDefectedGraceChord(ChordRestContainer& graceGhords)
{
    for (auto [pCr, gpBeat] : graceGhords) {
        UNUSED(gpBeat);

        if (!pCr) {
            continue;
        }

        if (pCr->type() == ElementType::CHORD) {
            Chord* pChord = static_cast<Chord*>(pCr);
            for (Note* pNote : pChord->notes()) {
                auto it = _slideHammerOnMap.begin(), e = _slideHammerOnMap.end();
                for (; it != e; ++it) {
                    if (it->first == pNote) {
                        _slideHammerOnMap.erase(it);
                        break;
                    }
                }
            }

            for (Chord* pGraceChord : pChord->graceNotes()) {
                for (Note* pNote : pGraceChord->notes()) {
                    auto it = _slideHammerOnMap.begin(), e = _slideHammerOnMap.end();
                    for (; it != e; ++it) {
                        if (it->first == pNote) {
                            _slideHammerOnMap.erase(it);
                            break;
                        }
                    }
                }
            }
        }
        delete pCr;
    }
    graceGhords.clear();
}

void GPConverter::addTextToNote(QString string, Note* note)
{
    Measure* measure = note->chord()->measure();
    Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());

    StaffText* text = Factory::createStaffText(segment);

    if (!string.isEmpty()) {
        bool useHarmony = string[string.size() - 1] == '\\';
        if (useHarmony) {
            string.resize(string.size() - 1);
        }
    }
    text->setPlainText(string);
    text->setTrack(note->chord()->track());
    segment->add(text);
}

void GPConverter::clearDefectedSpanner()
{
    for (const auto& element : _ties) {
        Tie* tie = element.second;
        if (tie->startNote()) {
            tie->startNote()->setTieFor(nullptr);
        }
        if (tie->endNote()) {
            tie->endNote()->setTieBack(nullptr);
        }
        delete tie;
    }
}

int GPConverter::getStringNumberFor(Ms::Note* pNote, int pitch) const
{
    const auto& stringTable = pNote->part()->instrument()->stringData()->stringList();

    const size_t stringTableSize = stringTable.size();

    if (!stringTableSize) {
        return -1;
    }

    const int32_t lastIdx = static_cast<int32_t>(stringTableSize) - 1;

    for (int32_t idx = lastIdx; idx >= 0; --idx) {
        auto string = stringTable[idx];
        const int32_t currenStringNumber = lastIdx - idx;

        if (pitch >= string.pitch) {
            bool emptyString = true;

            for (const auto& noteTmp : pNote->chord()->notes()) {
                if (noteTmp->string() == currenStringNumber) {
                    emptyString = false;
                    break;
                }
            }

            if (emptyString) {
                return currenStringNumber;
            }
        }
    }

    return -1;
}
} //end Ms namespace
