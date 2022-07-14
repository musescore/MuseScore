#include "gpconverter.h"

#include <chrono>

#include "gpdommodel.h"

#include "libmscore/factory.h"
#include "libmscore/arpeggio.h"
#include "libmscore/box.h"
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
#include "libmscore/chordline.h"
#include "libmscore/slur.h"
#include "libmscore/spanner.h"
#include "libmscore/stafftext.h"
#include "libmscore/staff.h"
#include "types/symid.h"
#include "libmscore/tempotext.h"
#include "libmscore/gradualtempochange.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tripletfeel.h"
#include "libmscore/trill.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/harmonicmark.h"
#include "libmscore/stretchedbend.h"

#include "../importgtp.h"

#include "log.h"
#include "translation.h"

using namespace mu::engraving;

namespace mu::engraving {
static JumpType jumpType(const String& typeString)
{
    static std::map<String, JumpType> types {
        { u"DaCapo", JumpType::DC },
        { u"DaCapoAlFine", JumpType::DC_AL_FINE },
        { u"DaCapoAlCoda", JumpType::DC_AL_CODA },
        { u"DaCapoAlDoubleCoda", JumpType::DC_AL_DBLCODA },
        { u"DaSegnoAlCoda", JumpType::DS_AL_CODA },
        { u"DaSegnoAlDoubleCoda", JumpType::DS_AL_DBLCODA },
        { u"DaSegnoAlFine", JumpType::DS_AL_FINE },
        { u"DaSegnoSegno", JumpType::DSS },
        { u"DaSegnoSegnoAlCoda", JumpType::DSS_AL_CODA },
        { u"DaSegnoSegnoAlDoubleCoda", JumpType::DSS_AL_DBLCODA },
        { u"DaSegnoSegnoAlFine", JumpType::DSS_AL_FINE },
        { u"DaCoda", JumpType::DCODA },
        { u"DaDoubleCoda", JumpType::DDBLCODA },
        { u"DaCoda", JumpType::DCODA },
    };

    if (types.find(typeString) != types.end()) {
        return types[typeString];
    }

    LOGE() << "wrong jump type";
    return JumpType::USER;
}

static MarkerType markerType(const String& typeString)
{
    static std::map<String, MarkerType> types {
        { u"Segno", MarkerType::SEGNO },
        { u"SegnoSegno", MarkerType::VARSEGNO },
        { u"Coda", MarkerType::CODA },
        { u"DoubleCoda", MarkerType::VARCODA },
        { u"Fine", MarkerType::FINE }
    };

    if (types.find(typeString) != types.end()) {
        return types[typeString];
    }

    LOGE() << "wrong direction marker type";
    return MarkerType::USER;
}

static String harmonicText(const GPBeat::HarmonicMarkType& type)
{
    static std::map<GPBeat::HarmonicMarkType, String> names {
        { GPBeat::HarmonicMarkType::Artificial, u"A.H." },
        { GPBeat::HarmonicMarkType::Pinch, u"P.H." },
        { GPBeat::HarmonicMarkType::Tap, u"T.H." },
        { GPBeat::HarmonicMarkType::Semi, u"S.H." },
        { GPBeat::HarmonicMarkType::FeedBack, u"Fdbk." },
    };

    if (names.find(type) != names.end()) {
        return names[type];
    }

    LOGE() << "wrong harmonic type";
    return String();
}

static TripletFeelType tripletFeelType(GPMasterBar::TripletFeelType tf)
{
    static std::map<GPMasterBar::TripletFeelType, TripletFeelType> types {
        { GPMasterBar::TripletFeelType::Triplet8th, TripletFeelType::TRIPLET_8TH },
        { GPMasterBar::TripletFeelType::Triplet16th, TripletFeelType::TRIPLET_16TH },
        { GPMasterBar::TripletFeelType::Dotted8th, TripletFeelType::DOTTED_8TH },
        { GPMasterBar::TripletFeelType::Dotted16th, TripletFeelType::DOTTED_16TH },
        { GPMasterBar::TripletFeelType::Scottish8th, TripletFeelType::SCOTTISH_8TH },
        { GPMasterBar::TripletFeelType::Scottish16th, TripletFeelType::SCOTTISH_16TH },
        { GPMasterBar::TripletFeelType::None, TripletFeelType::NONE }
    };

    if (types.find(tf) != types.end()) {
        return types[tf];
    }

    return TripletFeelType::NONE;
}

static OttavaType ottavaType(GPBeat::OttavaType t)
{
    static std::map<GPBeat::OttavaType, mu::engraving::OttavaType> types {
        { GPBeat::OttavaType::va8,  OttavaType::OTTAVA_8VA },
        { GPBeat::OttavaType::vb8,  OttavaType::OTTAVA_8VB },
        { GPBeat::OttavaType::ma15, OttavaType::OTTAVA_15MA },
        { GPBeat::OttavaType::mb15, OttavaType::OTTAVA_15MB }
    };

    if (types.find(t) != types.end()) {
        return types[t];
    }

    LOGE() << "wrong ottava type";
    return OttavaType::OTTAVA_8VA; /// there is no type "NONE"
}

static GPBeat::HarmonicMarkType harmonicTypeNoteToBeat(GPNote::Harmonic::Type t)
{
    static std::map<GPNote::Harmonic::Type, GPBeat::HarmonicMarkType> types {
        { GPNote::Harmonic::Type::Artificial, GPBeat::HarmonicMarkType::Artificial },
        { GPNote::Harmonic::Type::Pinch, GPBeat::HarmonicMarkType::Pinch },
        { GPNote::Harmonic::Type::Tap, GPBeat::HarmonicMarkType::Tap },
        { GPNote::Harmonic::Type::Semi, GPBeat::HarmonicMarkType::Semi },
        { GPNote::Harmonic::Type::FeedBack, GPBeat::HarmonicMarkType::FeedBack }
    };

    if (types.find(t) != types.end()) {
        return types[t];
    }

    //LOGE() << "wrong harmonic type"; TODO: fix
    return GPBeat::HarmonicMarkType::None;
}

static GPConverter::TextLineImportType harmonicMarkToImportType(GPBeat::HarmonicMarkType t)
{
    static std::map<GPBeat::HarmonicMarkType, GPConverter::TextLineImportType> types {
        { GPBeat::HarmonicMarkType::Artificial, GPConverter::TextLineImportType::HARMONIC_ARTIFICIAL },
        { GPBeat::HarmonicMarkType::Pinch, GPConverter::TextLineImportType::HARMONIC_PINCH },
        { GPBeat::HarmonicMarkType::Tap, GPConverter::TextLineImportType::HARMONIC_TAP },
        { GPBeat::HarmonicMarkType::Semi, GPConverter::TextLineImportType::HARMONIC_SEMI },
        { GPBeat::HarmonicMarkType::FeedBack, GPConverter::TextLineImportType::HARMONIC_FEEDBACK }
    };

    if (types.find(t) != types.end()) {
        return types[t];
    }

    //LOGE() << "wrong harmonic type"; TODO: fix
    return GPConverter::TextLineImportType::NONE;
}

static GPConverter::TextLineImportType ottavaToImportType(GPBeat::OttavaType t)
{
    static std::map<GPBeat::OttavaType, GPConverter::TextLineImportType> types {
        { GPBeat::OttavaType::ma15, GPConverter::TextLineImportType::OTTAVA_MA15 },
        { GPBeat::OttavaType::va8, GPConverter::TextLineImportType::OTTAVA_VA8 },
        { GPBeat::OttavaType::vb8, GPConverter::TextLineImportType::OTTAVA_VB8 },
        { GPBeat::OttavaType::mb15, GPConverter::TextLineImportType::OTTAVA_MB15 }
    };

    if (types.find(t) != types.end()) {
        return types[t];
    }

    // LOGE() << "wrong ottava type"; TODO: fix
    return GPConverter::TextLineImportType::NONE;
}

GPConverter::GPConverter(Score* score, std::unique_ptr<GPDomModel>&& gpDom)
    : _score(score), _gpDom(std::move(gpDom))
{
}

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
    fixPercussion();
}

void GPConverter::fixPercussion()
{
    // After reading some of the GP files, there're
    // parts, which contain only one percussion instrument.
    // In this case the program value for this instrument will
    // be set to the pitch value of the notes of the instrument,
    // thus we need to reset this value to 0.
    for (size_t i = 0; i < _score->parts().size(); ++i) {
        mu::engraving::Part* pPart = _score->parts()[i];
        IF_ASSERT_FAILED(!!pPart) {
            continue;
        }

        mu::engraving::Instrument* pInstrument = pPart->instrument();
        if (!pInstrument) {
            continue;
        }

        if (!pInstrument->useDrumset()) {
            continue;
        }

        for (size_t j = 0; j < pInstrument->channel().size(); ++j) {
            mu::engraving::InstrChannel* pChannel = pInstrument->channel()[j];
            IF_ASSERT_FAILED(!!pChannel) {
                continue;
            }

            if (!(pChannel->channel() == 9)) {
                continue;
            }

            pChannel->setProgram(0);
        }
    }
}

void GPConverter::fillTuplet()
{
    if (m_nextTupletInfo.elements.empty() || !m_nextTupletInfo.tuplet) {
        LOGE() << "not able to add elements to tuplet";
        return;
    }

    Tuplet* tuplet = m_nextTupletInfo.tuplet;
    ChordRest* firstCr = m_nextTupletInfo.elements.front();
    tuplet->setTrack(firstCr->track());
    tuplet->setParent(firstCr->measure());
    tuplet->setTick(firstCr->tick());
    tuplet->setBaseLen(Fraction(1, m_nextTupletInfo.lowestBase));
    tuplet->setTicks(m_nextTupletInfo.duration);
    setupTupletStyle(tuplet);

    for (ChordRest* elem : m_nextTupletInfo.elements) {
        tuplet->add(elem);
    }

    /// TODO: solve correctly within libmscore
    /// avoiding brackets on single note
    if (tuplet->elements().size() == 1) {
        tuplet->setBracketType(TupletBracketType::SHOW_NO_BRACKET);
    }

    m_nextTupletInfo.elements.clear();
    m_nextTupletInfo.tuplet = nullptr;
    m_nextTupletInfo.duration = Fraction();
    m_nextTupletInfo.lowestBase = NextTupletInfo::LOWEST_BASE;
}

void GPConverter::convert(const std::vector<std::unique_ptr<GPMasterBar> >& masterBars)
{
    for (uint32_t mi = 0; mi < masterBars.size(); ++mi) {
        Context ctx;
        ctx.masterBarIndex = mi;
        convertMasterBar(masterBars.at(mi).get(), ctx);
    }

    // glueing line segment elements separated with rests
    for (auto& trackMaps : m_elementsToAddToScore) {
        for (auto& typeMaps : trackMaps.second) {
            for (TextLineBase* elem : typeMaps.second) {
                if (elem) {
                    _score->addElement(elem);
                }
            }
        }
    }

    // fixing last measure barline
    if (_lastMeasure) {
        for (size_t staffIdx = 0; staffIdx < _score->staves().size(); staffIdx++) {
            _lastMeasure->setEndBarLineType(mu::engraving::BarLineType::FINAL, staffIdx * VOICES);
        }
    }

    addTempoMap();

#ifdef ENGRAVING_USE_STRETCHED_BENDS
    StretchedBend::prepareBends(m_bends);
#endif

    addFermatas();
    addContinuousSlideHammerOn();
}

void GPConverter::convertMasterBar(const GPMasterBar* mB, Context ctx)
{
    Measure* measure = addMeasure(mB);

    addTimeSig(mB, measure);

    addKeySig(mB, measure);

    addBarline(mB, measure);

    addRepeat(mB, measure);

    collectFermatas(mB, measure);

    convertBars(mB->bars(), ctx);

    addTripletFeel(mB, measure);

    addSection(mB, measure);

    addDirection(mB, measure);

    _lastMeasure = measure;
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
    addClef(bar, static_cast<int>(ctx.curTrack));

    if (addSimileMark(bar, static_cast<int>(ctx.curTrack))) {
        return;
    }
    convertVoices(bar->voices(), ctx);
}

void GPConverter::addBarline(const GPMasterBar* mB, Measure* measure)
{
    static bool insideFreeTime = false;
    size_t staves = _score->staves().size();

    if (mB->barlineType() == GPMasterBar::BarlineType::DOUBLE) {
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            measure->setEndBarLineType(mu::engraving::BarLineType::DOUBLE, staffIdx * VOICES);
        }
    }

    GPMasterBar::TimeSig sig = mB->timeSig();
    auto scoreTimeSig = Fraction(sig.numerator, sig.denominator);

    if (mB->freeTime()) {
        if (mB->barlineType() != GPMasterBar::BarlineType::DOUBLE) {
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                measure->setEndBarLineType(mu::engraving::BarLineType::BROKEN, staffIdx * VOICES);
            }
        }
        if (!insideFreeTime) {
            insideFreeTime = true;

            // Free time text
            for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Segment* s = measure->getSegment(SegmentType::TimeSig, measure->tick());
                StaffText* st = Factory::createStaffText(s);
                st->setTrack(staffIdx * VOICES);
                st->setPlainText(mu::mtrc("iex_guitarpro", "Free time"));
                s->add(st);

                // if timeSig is different, it was added before, here we handle "freetime"
                if (_lastTimeSig.numerator != sig.numerator
                    || _lastTimeSig.denominator != sig.denominator) {
                    return;
                }
                TimeSig* ts = Factory::createTimeSig(s);
                ts->setSig(scoreTimeSig);
                ts->setTrack(staffIdx * VOICES);
                ts->setLargeParentheses(true);
                s->add(ts);
            }
        }
    } else {
        insideFreeTime = false;
    }
    _lastTimeSig.numerator = sig.numerator;
    _lastTimeSig.denominator = sig.denominator;
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
        m_currentGPBeat = beat.get();
        ctx.curTick = convertBeat(beat.get(), graceGhords, ctx);
    }

    fillUncompletedMeasure(ctx);
    clearDefectedGraceChord(graceGhords);
}

Fraction GPConverter::convertBeat(const GPBeat* beat, ChordRestContainer& graceGhords, Context ctx)
{
    Measure* lastMeasure = _score->lastMeasure();

    if (ctx.curTick >= lastMeasure->ticks() + lastMeasure->tick()) {
        return ctx.curTick;
    }

    ChordRest* cr = addChordRest(beat, ctx);
    auto curSegment = lastMeasure->getSegment(SegmentType::ChordRest, ctx.curTick);

    if (beat->graceNotes() != GPBeat::GraceNotes::None) {
        if (cr->type() == ElementType::REST) {
            delete cr;
            return ctx.curTick;
        }

        curSegment->add(cr);

        configureGraceChord(beat, cr);
        graceGhords.push_back({ cr, beat });

        return ctx.curTick;
    }

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
    addTremolo(beat, cr);
    addPickStroke(beat, cr);
    addDynamic(beat, cr);
    addWah(beat, cr);
    addGolpe(beat, cr);
    addFretDiagram(beat, cr, ctx);
    addBarre(beat, cr);
    addSlapped(beat, cr);
    addPopped(beat, cr);
    addBrush(beat, cr);
    addArpeggio(beat, cr);
    addLyrics(beat, cr, ctx);
    addLegato(beat, cr);

    // line segment elements
    addOttava(beat, cr);
    addLetRing(beat, cr);
    addPalmMute(beat, cr);
    addRasgueado(beat, cr);
    addDive(beat, cr);
    addHarmonicMark(beat, cr);

    ctx.curTick += cr->actualTicks();

    return ctx.curTick;
}

void GPConverter::convertNotes(const std::vector<std::shared_ptr<GPNote> >& notes, ChordRest* cr)
{
    for (const auto& note : notes) {
        convertNote(note.get(), cr);
    }

    //! NOTE: later notes order is used in linked staff to create ties, glissando
    if (cr->isChord()) {
        static_cast<Chord*>(cr)->sortNotes();
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
    setPitch(note, gpnote->midiPitch());
    addBend(gpnote, note);
    addLetRing(gpnote, note);
    addPalmMute(gpnote, note);
    note->setGhost(gpnote->ghostNote());
    note->setDeadNote(gpnote->muted());
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

    Fraction fr(1, 8);
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
    auto scoreTimeSig = Fraction(sig.numerator, sig.denominator);
    measure->setTicks(scoreTimeSig);
    size_t staves = _score->staves().size();

    if (_lastTimeSig.numerator == sig.numerator
        && _lastTimeSig.denominator == sig.denominator) {
        return;
    }
    _lastTimeSig.numerator = sig.numerator;
    _lastTimeSig.denominator = sig.denominator;

    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Staff* staff = _score->staff(staffIdx);
        if (staff->staffType()->genTimesig()) {
            TimeSig* t = Factory::createTimeSig(_score->dummy()->segment());
            track_idx_t curTrack = staffIdx * VOICES;
            t->setTrack(curTrack);
            t->setSig(scoreTimeSig);
            if (mB->freeTime()) {
                t->setLargeParentheses(true);
            }
            Segment* s = measure->getSegment(SegmentType::TimeSig, tick);
            s->add(t);

            /// adding "Capo fret" text
            // TODO-gp: settings if we need to show capo
            if (m_showCapo && !m_hasCapo[curTrack]) {
                Fraction fr = { 0, 1 };
                int capo = staff->capo(fr);

                if (capo != 0) {
                    StaffText* st = Factory::createStaffText(s);
                    st->setTrack(curTrack);
                    String capoText = String(u"Capo fret %1").arg(capo);
                    st->setPlainText(mu::mtrc("iex_guitarpro", capoText));
                    s->add(st);
                    m_hasCapo[curTrack] = true;
                }
            }
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

    if (_lastVolta && _lastVolta->endings().size() != mB->alternateEnding().size()) {
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
    mu::engraving::Volta* volta;
    if (_lastVolta) {
        volta = _lastVolta;
        _score->removeElement(volta);
    } else {
        volta = new mu::engraving::Volta(_score->dummy());
        volta->setTick(measure->tick());
        _lastVolta = volta;
    }

    volta->endings().clear();
    volta->setTick2(measure->tick() + measure->ticks());

    String str;
    for (const auto& end : mB->alternateEnding()) {
        volta->endings().push_back(end);
        if (str.isEmpty()) {
            str += String(u"%1").arg(end);
        } else {
            str += String(u"-%1").arg(end);
        }
    }
    volta->setText(str);

    _score->addElement(volta);
}

void GPConverter::addDirection(const GPMasterBar* mB, Measure* measure)
{
    for (const auto& dir : mB->directions()) {
        if (dir.type == GPMasterBar::Direction::Type::Jump) {
            Jump* jump = Factory::createJump(measure);
            jump->setJumpType(jumpType(dir.name));
            jump->setTrack(0);
            measure->add(jump);
        } else {
            Marker* marker = Factory::createMarker(measure);
            marker->setMarkerType(markerType(dir.name));
            marker->setTrack(0);
            measure->add(marker);
        }
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

void GPConverter::addTripletFeel(const GPMasterBar* mB, Measure* measure)
{
    if (mB->tripletFeel() == _lastTripletFeel) {
        return; // if last triplet of last measure is equal current dont create new system text
    }

    Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
    TripletFeelType type = tripletFeelType(mB->tripletFeel());
    TripletFeel* tripletFeel = Factory::createTripletFeel(segment, type);
    _lastTripletFeel = mB->tripletFeel();
    tripletFeel->setTrack(0);
    segment->add(tripletFeel);
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
    size_t staves = _score->staves().size();

    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
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
    std::vector<String> fieldNames = { gpscore->title(), gpscore->subTitle(), gpscore->artist(),
                                       gpscore->album(), gpscore->composer(), gpscore->poet() };

    bool createTitleField
        = std::any_of(fieldNames.begin(), fieldNames.end(), [](const String& fieldName) { return !fieldName.isEmpty(); });

    if (!createTitleField) {
        return;
    }

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
        String str;
        if (!gpscore->subTitle().isEmpty()) {
            str.append(gpscore->subTitle());
        }
        if (!gpscore->artist().isEmpty()) {
            if (!str.isEmpty()) {
                str.append(u'\n');
            }
            str.append(gpscore->artist());
        }
        if (!gpscore->album().isEmpty()) {
            if (!str.isEmpty()) {
                str.append(u'\n');
            }
            str.append(gpscore->album());
        }
        s->setPlainText(str);
        m->add(s);
    }
    if (!gpscore->composer().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::COMPOSER);
        s->setPlainText(mu::mtrc("iex_guitarpro", "Music by %1").arg(gpscore->composer()));
        m->add(s);
    }
    if (!gpscore->poet().isEmpty()) {
        Text* s = Factory::createText(_score->dummy(), TextStyleType::POET);
        s->setPlainText(mu::mtrc("iex_guitarpro", "Words by %1").arg(gpscore->poet()));
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
    part->setPlainShortName(tR->shortName());
    part->setPartName(tR->name());
    part->setId(idx);

    _score->appendPart(part);
    for (size_t staffIdx = 0; staffIdx < tR->staffCount(); staffIdx++) {
        Staff* s = Factory::createStaff(part);
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

        String drumInstrName = tR->instrument();
        if (!drumInstrName.empty()) {
            part->setShortName(drumInstrName);
        }

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
            part->setCapoFret(capoFret);

            auto tunning = staffProperty[0].tunning;
            auto fretCount = staffProperty[0].fretCount;

            if (tunning.empty()) {
                tunning = standartTuning;
            }

            int transpose = tR->transpose();
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

    // this code sets score lyrics from the first processed track.
//    if (_score->OffLyrics.isEmpty())
//        _score->OffLyrics = tR->lyrics();

    part->instrument()->setTranspose(tR->transpose());
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
    if (m_nextTupletInfo.tuplet) {
        fillTuplet();
    }

    Measure* lastMeasure = _score->lastMeasure();
    int tickOffset = lastMeasure->ticks().ticks() + lastMeasure->tick().ticks() - ctx.curTick.ticks();
    if (tickOffset > 0) {
        _score->setRest(ctx.curTick, ctx.curTrack, Fraction::fromTicks(tickOffset), true, nullptr);

        for (auto& trackMaps : m_elementsToAddToScore) {
            for (auto& typeMaps : trackMaps.second) {
                auto& elemsToAdd = typeMaps.second;
                if (!elemsToAdd.empty() && elemsToAdd.back() != nullptr) {
                    elemsToAdd.push_back(nullptr);
                }
            }
        }
    }
}

void GPConverter::addContinuousSlideHammerOn()
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

        return nextChord->upNote();
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
        track_idx_t track = startNote->track();

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
            gl->setText(u"");
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
                String hammerText = (startNote->pitch() > endNote->pitch()) ? u"P" : u"H";

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
        Fraction tick = Fraction::fromTicks(mu::engraving::Constants::division * gpFermata.offsetNum / gpFermata.offsetDenom);
        // bellow how gtp fermata timeStretch converting to MU timeStretch
        float convertingLength = 1.5f - gpFermata.length * 0.5f + gpFermata.length * gpFermata.length * 3;
        Segment* seg = measure->getSegmentR(SegmentType::ChordRest, tick);

        for (size_t staffIdx = 0; staffIdx < _score->staves().size(); staffIdx++) {
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
    GradualTempoChange* _lastGradualTempoChange = nullptr;
    int previousTempo = -1;

    for (auto m = _score->firstMeasure(); m; m = m->nextMeasure()) {
        auto range = _tempoMap.equal_range(measureIdx); //one measure can have several tempo changing
        measureIdx++;
        for (auto tempIt = range.first; tempIt != range.second; tempIt++) {
            Fraction tick = m->tick() + Fraction::fromTicks(
                tempIt->second.position * Constants::division * 4 * m->ticks().numerator() / m->ticks().denominator());
            Segment* existedSegment = m->findSegment(SegmentType::ChordRest, tick);
            Segment* segment = m->getSegment(SegmentType::ChordRest, tick);
            Segment* closestChordRestSegment = segment->next(SegmentType::ChordRest);
            if (!closestChordRestSegment) {
                closestChordRestSegment = segment->prev(SegmentType::ChordRest);
            }

            Segment* nextSegment = segment->next(SegmentType::ChordRest | SegmentType::BarLineType);

            int realTemp = realTempo(tempIt->second);
            TempoText* tt = Factory::createTempoText(segment);
            tt->setTempo((qreal)realTemp / 60);
            tt->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(realTemp));
            tt->setTrack(0);
            segment->add(tt);
            _score->setTempo(tick, tt->tempo());

            if (_lastGradualTempoChange) {
                // extend duration until the end of chord
                if (nextSegment) {
                    tick = nextSegment->tick();
                }

                _lastGradualTempoChange->setTick2(tick);
                if (realTemp > previousTempo) {
                    _lastGradualTempoChange->setTempoChangeType(GradualTempoChangeType::Accelerando);
                    _lastGradualTempoChange->setBeginText(u"accel");
                } else {
                    _lastGradualTempoChange->setTempoChangeType(GradualTempoChangeType::Rallentando);
                    _lastGradualTempoChange->setBeginText(u"rall");
                }

                _score->addElement(_lastGradualTempoChange);
                _lastGradualTempoChange = nullptr;
            }

            if (tempIt->second.linear) {
                GradualTempoChange* tempoChange = Factory::createGradualTempoChange(segment);

                /// if segment was created for tempo, set the tick to closest chordRestSegment
                if (!existedSegment && closestChordRestSegment) {
                    tick = closestChordRestSegment->tick();
                }

                tempoChange->setTick(tick);
                tempoChange->setTrack(0);
                _lastGradualTempoChange = tempoChange;
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
    staff_idx_t staffIdx = track2staff(curTrack);

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
    auto scoreTimeSig = Fraction(sig.numerator, sig.denominator);
    measure->setTimesig(scoreTimeSig);
    measure->setTicks(scoreTimeSig);
    _score->measures()->add(measure);

    return measure;
}

ChordRest* GPConverter::addChordRest(const GPBeat* beat, const Context& ctx)
{
    auto rhythm = [](GPRhythm::RhytmType rhythm) {
        if (rhythm == GPRhythm::RhytmType::Whole) {
            return 1;
        } else if (rhythm == GPRhythm::RhytmType::Half) {
            return 2;
        } else if (rhythm == GPRhythm::RhytmType::Quarter) {
            return 4;
        } else if (rhythm == GPRhythm::RhytmType::Eighth) {
            return 8;
        } else if (rhythm == GPRhythm::RhytmType::Sixteenth) {
            return 16;
        } else if (rhythm == GPRhythm::RhytmType::ThirtySecond) {
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

    Fraction fr(1, rhythm(beat->lenth().second));
    for (int dot = 1; dot <= beat->lenth().first; dot++) {
        fr += Fraction(1, rhythm(beat->lenth().second) * 2 * dot);
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

    auto scoreFinger = [](const String& str) {
        if (str == u"Open") {
            return String(u'O');
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
            return SymId::ornamentTurnInverted;
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
            return VibratoType::GUITAR_VIBRATO;
        case GPNote::VibratoType::Wide:
            return VibratoType::GUITAR_VIBRATO_WIDE;
        default:
            return VibratoType::GUITAR_VIBRATO;
        }
    };

    VibratoType vibratoType = scoreVibratoType(gpnote->vibratoType());
    addVibratoByType(note, vibratoType);
}

void GPConverter::addHarmonic(const GPNote* gpnote, Note* note)
{
    if (gpnote->harmonic().type == GPNote::Harmonic::Type::None) {
        return;
    }

    Note* hnote = nullptr;
    if (gpnote->harmonic().type != GPNote::Harmonic::Type::Natural) {
        hnote = mu::engraving::Factory::createNote(_score->dummy()->chord());
        hnote->setTrack(note->track());
        hnote->setString(note->string());
        hnote->setPitch(note->pitch());
        hnote->setFret(note->fret());
        note->setDisplayFret(Note::DisplayFretOption::ArtificialHarmonic);
        hnote->setDisplayFret(Note::DisplayFretOption::Hide);
        hnote->setTpcFromPitch();
        note->chord()->add(hnote);
        hnote->setPlay(false);
        addTie(gpnote, note);
        note->setHarmonicFret(note->fret() + gpnote->harmonic().fret);
    } else {
        note->setHarmonicFret(gpnote->harmonic().fret);
        note->setDisplayFret(Note::DisplayFretOption::NaturalHarmonic);
    }

    Note* harmonicNote = hnote ? hnote : note;

    int gproHarmonicType = static_cast<int>(gpnote->harmonic().type);
    int harmonicFret = GuitarPro::harmonicOvertone(note, gpnote->harmonic().fret, gproHarmonicType);
    int string = harmonicNote->string();
    int harmonicPitch = harmonicNote->part()->instrument()->stringData()->getPitch(string,
                                                                                   harmonicFret + harmonicNote->part()->capoFret(),
                                                                                   harmonicNote->staff());

    harmonicNote->setPitch(harmonicPitch);
    harmonicNote->setTpcFromPitch();
    harmonicNote->setHarmonic(true);

    if (GPNote::Harmonic::isArtificial(gpnote->harmonic().type) && m_currentGPBeat) {
        m_currentGPBeat->setHarmonicMarkType(harmonicTypeNoteToBeat(gpnote->harmonic().type));
    }
}

void GPConverter::addAccent(const GPNote* gpnote, Note* note)
{
    if (gpnote->accents().none()) {
        return;
    }

    auto symbolsIds = note->chord()->articulationSymbolIds();

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
        if (gpnote->accents()[flagIdx] && symbolsIds.find(accentType(flagIdx)) == symbolsIds.end()) {
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

    Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
    art->setSymId(SymId::guitarRightHandTapping);
    if (!note->score()->toggleArticulation(note, art)) {
        delete art;
    }
}

void GPConverter::addSlide(const GPNote* gpnote, Note* note)
{
    if (gpnote->slides().none()) {
        return;
    }

    addSingleSlide(gpnote, note);
    collectContinuousSlide(gpnote, note);
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

            ChordLine* cl = mu::engraving::Factory::createChordLine(_score->dummy()->chord());
            cl->setChordLineType(type.first);
            cl->setStraight(true);
            note->chord()->add(cl);
            cl->setNote(note);

            Note::Slide sl{ type.second, nullptr };
            note->attachSlide(sl);
        }
    }
}

void GPConverter::collectContinuousSlide(const GPNote* gpnote, Note* note)
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

#ifdef ENGRAVING_USE_STRETCHED_BENDS
    StretchedBend* bend = mu::engraving::Factory::createStretchedBend(note);
#else
    Bend* bend = mu::engraving::Factory::createBend(note);
#endif

    auto gpBend = gpnote->bend();

    bool bendHasMiddleValue{ true };
    if (gpBend->middleOffset1 == 12 || gpBend->middleOffset2 == 12) {
        bendHasMiddleValue = false;
    }

    bend->points().push_back(PitchValue(0, gpBend->originValue));

    PitchValue lastPoint = bend->points().back();

    if (bendHasMiddleValue) {
        if (PitchValue value(gpBend->middleOffset1, gpBend->middleValue);
            gpBend->middleOffset1 >= 0 && gpBend->middleOffset1 < gpBend->destinationOffset && value != lastPoint) {
            bend->points().push_back(std::move(value));
        }
        if (PitchValue value(gpBend->middleOffset2, gpBend->middleValue);
            gpBend->middleOffset2 >= 0 && gpBend->middleOffset2 != gpBend->middleOffset1
            && gpBend->middleOffset2 < gpBend->destinationOffset
            && value != lastPoint) {
            bend->points().push_back(std::move(value));
        }

        if (gpBend->middleOffset1 == -1 && gpBend->middleOffset2 == -1 && gpBend->middleValue != -1) {
            //!@NOTE It seems when middle point is places exactly in the middle
            //!of bend  GP6 stores this value equal -1
            if (gpBend->destinationOffset > 50 || gpBend->destinationOffset == -1) {
                bend->points().push_back(PitchValue(50, gpBend->middleValue));
            }
        }
    }

    if (gpBend->destinationOffset <= 0) {
        bend->points().push_back(PitchValue(100, gpBend->destinationValue)); //! In .gpx this value might be exist
    } else {
        if (PitchValue value(gpBend->destinationOffset, gpBend->destinationValue); value != lastPoint) {
            bend->points().push_back(std::move(value));
        }
    }

    bend->setTrack(note->track());
    note->add(bend);
    m_bends.push_back(bend);
}

void GPConverter::addLineElement(ChordRest* cr, std::vector<TextLineBase*>& elements, ElementType muType, TextLineImportType importType,
                                 bool forceSplitByRests)
{
    track_idx_t track = cr->track();

    auto& lastTypeForTrack = m_lastImportTypes[track][muType];

    while (elements.size() < track + 1) {
        elements.push_back(nullptr);
    }

    auto& elem = elements[track];

    if (!forceSplitByRests) {
        if (!cr->isRest()) {
            elem = nullptr; // should start new element
            return;
        }

        if (lastTypeForTrack != TextLineImportType::NONE) {
            auto& lastTypeElementsToAdd = m_elementsToAddToScore[track][lastTypeForTrack];

            /// adding nullptr to 'elements to be added' to track the REST
            if (!lastTypeElementsToAdd.empty() && lastTypeElementsToAdd.back() != nullptr) {
                lastTypeElementsToAdd.push_back(nullptr);
            }
        }

        return;
    }

    if (lastTypeForTrack != importType) {
        elem = nullptr;
    }

    Fraction tick = cr->tick();

    if (elem) {
        ChordRest* lastCR = elem->endCR();
        if (lastCR == cr) {
            return;
        }

        if (elem->tick2() < tick) {
            if (lastTypeForTrack != TextLineImportType::NONE) {
                auto& lastTypeElementsToAdd = m_elementsToAddToScore[track][lastTypeForTrack];

                /// removing info about the REST and updating last element's ticks
                if (!lastTypeElementsToAdd.empty() && lastTypeElementsToAdd.back() == nullptr) {
                    lastTypeElementsToAdd.pop_back();
                    TextLineBase* prevElem = lastTypeElementsToAdd.back();
                    if (!prevElem) {
                        LOGE() << "error while importing";
                        return;
                    }

                    elem = prevElem;
                    elem->setTick2(tick + cr->actualTicks());
                }
            }
        } else {
            elem->setTick2(cr->tick() + cr->actualTicks());
            elem->setEndElement(cr);
        }
    }

    if (!elem) {
        EngravingItem* engItem = Factory::createItem(muType, _score->dummy());

        TextLineBase* newElem = dynamic_cast<TextLineBase*>(engItem);
        if (!newElem) {
            qFatal("wrong type of imported element");
        }

        elem = newElem;

        newElem->setTick(tick);
        newElem->setTick2(tick + cr->actualTicks());
        newElem->setTrack(track);
        newElem->setTrack2(track);
        newElem->setStartElement(cr);
        newElem->setEndElement(cr);

        auto& elementsToAdd = m_elementsToAddToScore[track][importType];
        elementsToAdd.push_back(newElem);
        lastTypeForTrack = importType;
    }
}

void GPConverter::setPitch(Note* note, const GPNote::MidiPitch& midiPitch)
{
    int32_t fret = midiPitch.fret;
    int32_t musescoreString{ -1 };
    if (midiPitch.string != -1) {
        musescoreString = static_cast<int32_t>(note->part()->instrument()->stringData()->strings()) - 1 - midiPitch.string;
    }

    int pitch = 0;
    if (midiPitch.midi != -1) {
        pitch = midiPitch.midi;
    } else if (midiPitch.octave != -1 || midiPitch.tone != 0) {
        pitch = midiPitch.octave * 12 + midiPitch.tone;
    } else if (midiPitch.variation != -1) {
        pitch = calculateDrumPitch(midiPitch.element, midiPitch.variation, note->part()->shortName());
    } else if (note->part()->midiChannel() == 9) {
        //!@NOTE This is a case, when part of the note is a
        //       single drum instrument. It seems, that GP
        //       sets to -1 all midi parameters for the note,
        //       but sets the midi pitch value to the program
        //       instead.
        pitch = note->part()->instrument()->channel(0)->program();
    } else {
        pitch
            = note->part()->instrument()->stringData()->getPitch(musescoreString, midiPitch.fret + note->part()->capoFret(),
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

int GPConverter::calculateDrumPitch(int element, int variation, const String& /*instrumentName*/)
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
            return u"fff";
        } else if (t == GPBeat::DynamicType::FF) {
            return u"ff";
        } else if (t == GPBeat::DynamicType::FF) {
            return u"ff";
        } else if (t == GPBeat::DynamicType::F) {
            return u"f";
        } else if (t == GPBeat::DynamicType::MF) {
            return u"mf";
        } else if (t == GPBeat::DynamicType::MP) {
            return u"mp";
        } else if (t == GPBeat::DynamicType::P) {
            return u"p";
        } else if (t == GPBeat::DynamicType::PP) {
            return u"pp";
        }
        return u"ppp";
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

    using tieMap = std::unordered_multimap<track_idx_t, Tie*>;

    auto startTie = [](Note* note, Score* sc, tieMap& ties, track_idx_t curTrack) {
        Tie* tie = Factory::createTie(sc->dummy());
        note->add(tie);
        ties.insert(std::make_pair(curTrack, tie));
    };

    auto endTie = [](Note* note, tieMap& ties, track_idx_t curTrack) {
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

    using slurMap = std::unordered_map<track_idx_t, Slur*>;

    auto startSlur = [](ChordRest* cr, Score* sc, slurMap& slurs, track_idx_t curTrack, Fraction curTick) {
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

    auto mediateSlur = [](ChordRest* cr, slurMap& slurs, track_idx_t curTrack, Fraction curTick) {
        Slur* slur = slurs[curTrack];
        if (!slur) {
            return;
        }
        slur->setTrack(curTrack);
        slur->setTrack2(curTrack);
        slur->setTick2(curTick);
        slur->setEndElement(cr);
    };

    auto endSlur = [](ChordRest* cr, slurMap& slurs, track_idx_t curTrack, Fraction curTick) {
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
    addLineElement(cr, m_ottavas[gpb->ottavaType()], ElementType::OTTAVA, ottavaToImportType(gpb->ottavaType()),
                   gpb->ottavaType() != GPBeat::OttavaType::None);

    if (!cr->isChord() || gpb->ottavaType() == GPBeat::OttavaType::None) {
        return;
    }

    const Chord* chord = toChord(cr);
    mu::engraving::OttavaType type = ottavaType(gpb->ottavaType());

    TextLineBase* textLineElem = m_ottavas[gpb->ottavaType()].back();
    Ottava* ottava = dynamic_cast<Ottava*>(textLineElem);

    if (!ottava) {
        return;
    }

    ottava->setOttavaType(type);

    for (mu::engraving::Note* note : chord->notes()) {
        int pitch = note->pitch();
        if (type == mu::engraving::OttavaType::OTTAVA_8VA) {
            note->setPitch((pitch - 12 > 0) ? pitch - 12 : pitch);
        } else if (type == mu::engraving::OttavaType::OTTAVA_8VB) {
            note->setPitch((pitch + 12 < 127) ? pitch + 12 : pitch);
        } else if (type == mu::engraving::OttavaType::OTTAVA_15MA) {
            note->setPitch((pitch - 24 > 0) ? pitch - 24 : (pitch - 12 > 0 ? pitch - 12 : pitch));
        } else if (type == mu::engraving::OttavaType::OTTAVA_15MB) {
            note->setPitch((pitch + 24 < 127) ? pitch + 24 : ((pitch + 12 < 127) ? pitch + 12 : pitch));
        }
    }
}

void GPConverter::addLetRing(const GPNote* gpnote, Note* /*note*/)
{
    if (gpnote->letRing() && m_currentGPBeat) {
        m_currentGPBeat->setLetRing(true);
        //note->setLetRing(true); TODO-gp: let ring playback
    }
}

void GPConverter::addPalmMute(const GPNote* gpnote, Note* /*note*/)
{
    if (gpnote->palmMute() && m_currentGPBeat) {
        m_currentGPBeat->setPalmMute(true);
        //note->setPalmMute(true); TODO-gp: palm mute playback
    }
}

void GPConverter::addLetRing(const GPBeat* gpbeat, ChordRest* cr)
{
    addLineElement(cr, m_letRings, ElementType::LET_RING, TextLineImportType::LET_RING, gpbeat->letRing());
}

void GPConverter::addPalmMute(const GPBeat* gpbeat, ChordRest* cr)
{
    addLineElement(cr, m_palmMutes, ElementType::PALM_MUTE, TextLineImportType::PALM_MUTE, gpbeat->palmMute());
}

void GPConverter::addDive(const GPBeat* beat, ChordRest* cr)
{
    addLineElement(cr, m_dives, ElementType::WHAMMY_BAR, TextLineImportType::WHAMMY_BAR, beat->dive());
}

void GPConverter::addRasgueado(const GPBeat* beat, ChordRest* cr)
{
    addLineElement(cr, m_rasgueados, ElementType::RASGUEADO, TextLineImportType::RASGUEADO, beat->rasgueado() != GPBeat::Rasgueado::None);
}

void GPConverter::addHarmonicMark(const GPBeat* gpbeat, ChordRest* cr)
{
    auto harmonicMarkType = gpbeat->harmonicMarkType();
    auto& textLineElems = m_harmonicMarks[harmonicMarkType];
    addLineElement(cr, textLineElems, ElementType::HARMONIC_MARK, harmonicMarkToImportType(
                       harmonicMarkType), harmonicMarkType != GPBeat::HarmonicMarkType::None);
    if (!textLineElems.empty()) {
        auto& elem = textLineElems.back();
        if (elem) {
            elem->setBeginText(harmonicText(harmonicMarkType));
        }
    }
}

void GPConverter::addFretDiagram(const GPBeat* gpnote, ChordRest* cr, const Context& ctx)
{
    static int last_idx = -1;

    int GPTrackIdx = static_cast<int>(ctx.curTrack);
    int diaId = gpnote->diagramIdx(GPTrackIdx, ctx.masterBarIndex);

    if (last_idx == diaId) {
        return;
    }

    last_idx = diaId;

    if (diaId == -1) {
        return;
    }

    auto trackIt = _gpDom->tracks().find(GPTrackIdx);
    if (trackIt == _gpDom->tracks().cend()) {
        return;
    }

    if (trackIt->second->diagram().count(diaId) == 0) {
        return;
    }

    GPTrack::Diagram diagram = trackIt->second->diagram().at(diaId);

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

    Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
    art->setTextType(Articulation::TextType::SLAP);
    cr->add(art);
}

void GPConverter::addPopped(const GPBeat* beat, ChordRest* cr)
{
    if (!beat->popped() || cr->type() != ElementType::CHORD) {
        return;
    }

    Articulation* art = mu::engraving::Factory::createArticulation(_score->dummy()->chord());
    art->setTextType(Articulation::TextType::POP);
    cr->add(art);
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
            return ArpeggioType::UP;
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
    st->setPlainText(String::number(minutes)
                     + u':'
                     + (seconds < 10 ? u'0' + String::number(seconds) : String::number(seconds)));
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
        real = ((tuplet->ratio().denominator() == 2) || (tuplet->ratio().denominator() == 6));
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

bool GPConverter::tupletParamsChanged(const GPBeat* beat, const ChordRest* cr)
{
    return beat->tuplet().num == -1
           || Fraction(beat->tuplet().num, beat->tuplet().denom) != m_nextTupletInfo.ratio
           || cr->track() != m_nextTupletInfo.track
           || cr->measure() != m_nextTupletInfo.measure
           || (m_nextTupletInfo.elements.size() > 1
               && m_nextTupletInfo.duration == Fraction(m_nextTupletInfo.ratio.denominator(), m_nextTupletInfo.lowestBase));
}

void GPConverter::addTuplet(const GPBeat* beat, ChordRest* cr)
{
    Fraction currentTupletType = Fraction(beat->tuplet().num, beat->tuplet().denom);

    if (m_nextTupletInfo.tuplet && tupletParamsChanged(beat, cr)) {
        fillTuplet();
    }

    if (beat->tuplet().num == -1) {
        return;
    }

    m_nextTupletInfo.lowestBase = std::min(m_nextTupletInfo.lowestBase, cr->ticks().denominator());

    if (!m_nextTupletInfo.tuplet) {
        m_nextTupletInfo.tuplet  = Factory::createTuplet(_score->dummy()->measure());
        m_nextTupletInfo.tuplet->setRatio(currentTupletType);
        m_nextTupletInfo.ratio = currentTupletType;
        m_nextTupletInfo.measure = cr->measure();
        m_nextTupletInfo.track = cr->track();
    }

    m_nextTupletInfo.elements.push_back(cr);
    cr->setTuplet(m_nextTupletInfo.tuplet);
    m_nextTupletInfo.duration += cr->actualTicks();
}

void GPConverter::addVibratoByType(const Note* note, VibratoType type)
{
    track_idx_t track = note->track();
    while (_vibratos.size() < track + 1) {
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
            return VibratoType::VIBRATO_SAWTOOTH;
        } else {
            return VibratoType::VIBRATO_SAWTOOTH_WIDE;
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
    if (!cr->isChord() || beat->tremolo().numerator == -1) {
        return;
    }

    auto scoreTremolo = [](const GPBeat::Tremolo tr) {
        if (tr.denominator == 2) {
            return TremoloType::R8;
        }
        if (tr.denominator == 4) {
            return TremoloType::R16;
        } else {
            return TremoloType::R32;
        }
    };

    Tremolo* t = Factory::createTremolo(_score->dummy()->chord());
    t->setTremoloType(scoreTremolo(beat->tremolo()));
    static_cast<Chord*>(cr)->add(t);
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

void GPConverter::addGolpe(const GPBeat* beat, ChordRest* cr)
{
    if (beat->golpe() == GPBeat::Golpe::None) {
        return;
    }
    if (cr->type() != ElementType::CHORD) {
        return;
    }

    Articulation* art = Factory::createArticulation(_score->dummy()->chord());
    art->setSymId(SymId::guitarGolpe);

    if (beat->golpe() == GPBeat::Golpe::Thumb) {
        art->setAnchor(ArticulationAnchor::BOTTOM_STAFF);
    }

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
            return std::string("1/2B ");
        } else {
            return std::string("B ");
        }
    };

    std::string barreFret;
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

    addTextToNote(String::fromStdString(barreType(beat->barre()) + barreFret), static_cast<Chord*>(cr)->upNote());
}

void GPConverter::addLyrics(const GPBeat* beat, ChordRest* cr, const Context& ctx)
{
    ID GPTrackIdx = cr->part()->id();

    const std::string& lyrStr = beat->lyrics(GPTrackIdx.toUint64(), ctx.masterBarIndex);
    if (lyrStr.empty()) {
        return;
    }

    Lyrics* lyr = Factory::createLyrics(_score->dummy()->chord());
    lyr->setPlainText(String::fromUtf8(lyrStr.c_str()));
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

void GPConverter::addTextToNote(String string, Note* note)
{
    Segment* segment = note->chord()->segment();
    StaffText* text = Factory::createStaffText(segment);

    if (!string.isEmpty()) {
        bool useHarmony = string[string.size() - 1] == '\\';
        if (useHarmony) {
            string.chop(1);
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

int GPConverter::getStringNumberFor(mu::engraving::Note* pNote, int pitch) const
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
