#ifndef SCOREDOMBUILDER_H
#define SCOREDOMBUILDER_H

#include <memory>
#include <unordered_map>

#include "gpmasterbar.h"
#include "gpbar.h"
#include "gpbeat.h"
#include "gpdrumsetresolver.h"
#include "gpmastertracks.h"
#include "types/fraction.h"

#include "libmscore/vibrato.h"

#include "iengravingconfiguration.h"

namespace mu::engraving {
class GPNote;
class GPVoice;
class GPScore;
class GPTrack;

class Score;
class Measure;
class GPDomModel;
class ChordRest;
class Chord;
class Note;
class Tie;
class Slur;
class Volta;
class Glissando;
class Tuplet;
class Hairpin;
class LetRing;
class PalmMute;
class Vibrato;
class Ottava;
class Bend;

class GPConverter
{
    INJECT(mu::engraving::IEngravingConfiguration, engravingConfiguration);

public:
    GPConverter(Score* score, std::unique_ptr<GPDomModel>&& gpDom);

    void convertGP();

    const std::unique_ptr<GPDomModel>& gpDom() const;

    enum class LineImportType {
        NONE,
        LET_RING,
        PALM_MUTE,
        WHAMMY_BAR,
        RASGUEADO,
        PICK_SCRAPE,

        /// harmonics
        HARMONIC_ARTIFICIAL,
        HARMONIC_PINCH,
        HARMONIC_TAP,
        HARMONIC_SEMI,
        HARMONIC_FEEDBACK,

        /// ottavas
        OTTAVA_MA15,
        OTTAVA_VA8,
        OTTAVA_VB8,
        OTTAVA_MB15,

        /// trill
        TRILL,

        /// hairpin
        CRESCENDO,
        DIMINUENDO,
    };

private:

    static constexpr int PERC_CHANNEL = 9;

    using ChordRestContainer = std::vector<std::pair<ChordRest*, const GPBeat*> >;
    using TieMap = std::unordered_map<track_idx_t, std::vector<Tie*> >;

    struct Context {
        int32_t masterBarIndex{ 0 };
        track_idx_t curTrack{ 0 };
        Fraction curTick;
    };

    void convert(const std::vector<std::unique_ptr<GPMasterBar> >& mBs);
    void clearDefectedSpanner();

    void convertMasterBar(const GPMasterBar* mB, Context ctx);
    void convertBars(const std::vector<std::unique_ptr<GPBar> >& bars, Context ctx);
    void convertBar(const GPBar* bar, Context ctx);
    void convertVoices(const std::vector<std::unique_ptr<GPVoice> >&, Context ctx);
    void convertVoice(const GPVoice*, Context ctx);
    void convertBeats(const std::vector<std::shared_ptr<GPBeat> >& beats, Context ctx);
    Fraction convertBeat(const GPBeat* beat, ChordRestContainer& graceChords, Context ctx);
    void configureGraceChord(const GPBeat* beat, ChordRest* cr, GPBeat::OttavaType type);
    void convertNotes(const std::vector<std::shared_ptr<GPNote> >& notes, ChordRest* cr);
    void convertNote(const GPNote* note, ChordRest* cr);

    void setUpGPScore(const GPScore* gpscore);
    void setUpTracks(const std::map<int, std::unique_ptr<GPTrack> >& tracks);
    void setUpTrack(const std::unique_ptr<GPTrack>& tracks);
    void collectTempoMap(const GPMasterTracks* mTr);
    void collectFermatas(const GPMasterBar* mB, Measure* measure);
    void fixPercussion();

    Measure* addMeasure(const GPMasterBar* mB);
    void addTimeSig(const GPMasterBar* mB, Measure* measure);
    void addKeySig(const GPMasterBar* mB, Measure* measure);
    void addTripletFeel(const GPMasterBar* mB, Measure* measure);
    void addDirection(const GPMasterBar* mB, Measure* measure);
    void addSection(const GPMasterBar* mB, Measure* measure);
    void addRepeat(const GPMasterBar* mB, Measure* measure);
    void addVolta(const GPMasterBar* mB, Measure* measure);
    void doAddVolta(const GPMasterBar* mB, Measure* measure);
    void addClef(const GPBar* bar, int curTrack);
    bool addSimileMark(const GPBar* bar, int curTrack);
    void addBarline(const GPMasterBar* mB, Measure* measure);

    void addTie(const GPNote* gpnote, Note* note, TieMap& ties);
    void addFretDiagram(const GPBeat* gpnote, ChordRest* note, const Context& ctx, bool asHarmony = true);
    ChordRest* addChordRest(const GPBeat* beats, const Context& ctx);
    void addOrnament(const GPNote* gpnote, Note* note);
    void addVibratoLeftHand(const GPNote* gpnote, Note* note);
    void addVibratoByType(const Note* note, VibratoType type);
    void addTrill(const GPNote* gpnote, Note* note);
    Note* addHarmonic(const GPNote* gpnote, Note* note);
    void addFingering(const GPNote* gpnote, Note* note);
    void addAccent(const GPNote* gpnote, Note* note);
    void addLeftHandTapping(const GPNote* gpnote, Note* note);
    void addStringNumber(const GPNote* gpnote, Note* note);
    void addTapping(const GPNote* gpnote, Note* note);
    void addSlide(const GPNote* gpnote, Note* note);
    void addSingleSlide(const GPNote* gpnote, Note* note);
    void addPickScrape(const GPNote* gpnote, Note* note);
    void addLetRing(const GPNote* gpnote, Note* note);
    void addPalmMute(const GPNote* gpnote, Note* note);
    void collectContinuousSlide(const GPNote* gpnote, Note* note);
    void collectHammerOn(const GPNote* gpnote, Note* note);
    void addBend(const GPNote* gpnote, Note* note);
    void setPitch(Note* note, const GPNote::MidiPitch& midiPitch);
    void setTpc(Note* note, int accidental);
    int calculateDrumPitch(int element, int variation, const String& instrumentName);
    void addTextToNote(String string, Note* note);

    void addLegato(const GPBeat* beat, ChordRest* cr);
    void addOttava(const GPBeat* gpb, ChordRest* cr);
    void addDynamic(const GPBeat* beat, ChordRest* cr);
    void addSlapped(const GPBeat* beat, ChordRest* cr);
    void addPopped(const GPBeat* beat, ChordRest* cr);
    void addBrush(const GPBeat* beat, ChordRest* cr);
    void addArpeggio(const GPBeat* beat, ChordRest* cr);
    void addTimer(const GPBeat* beat, ChordRest* cr);
    void addFreeText(const GPBeat* beat, ChordRest* cr);
    void addTuplet(const GPBeat* beat, ChordRest* cr);
    void addLetRing(const GPBeat* gpbeat, ChordRest* cr);
    void addPalmMute(const GPBeat* gpbeat, ChordRest* cr);
    void addTrill(const GPBeat* gpbeat, ChordRest* cr);
    void addDive(const GPBeat* beat, ChordRest* cr);
    void addPickScrape(const GPBeat* beat, ChordRest* cr);
    void addHarmonicMark(const GPBeat* gpbeat, ChordRest* cr);
    void setupTupletStyle(Tuplet* tuplet);
    void addVibratoWTremBar(const GPBeat* beat, ChordRest* cr);
    void addFadding(const GPBeat* beat, ChordRest* cr);
    void addHairPin(const GPBeat* beat, ChordRest* cr);
    void addRasgueado(const GPBeat* beat, ChordRest* cr);
    void addPickStroke(const GPBeat* beat, ChordRest* cr);
    void addTremolo(const GPBeat* beat, ChordRest* cr);
    void addWah(const GPBeat* beat, ChordRest* cr);
    void addGolpe(const GPBeat* beat, ChordRest* cr);
    void addBarre(const GPBeat* beat, ChordRest* cr);
    void addLyrics(const GPBeat* beat, ChordRest* cr, const Context& ctx);
    void clearDefectedGraceChord(ChordRestContainer& graceGhords);

    void addContinuousSlideHammerOn();
    void addFermatas();
    void addTempoMap();
    void addInstrumentChanges();
    void fillUncompletedMeasure(const Context& ctx);
    void hideRestsInEmptyMeasures(track_idx_t track);
    int getStringNumberFor(Note* pNote, int pitch) const;
    void fillTuplet();
    bool tupletParamsChanged(const GPBeat* beat, const ChordRest* cr);

    /**
     * Making the current element of continious type (octave, let ring, trill etc.. inherited from SLine) longer or starting a new one
     *
     * @param cr ChordRest to which element will be added
     * @param elements vector storing current continious elements for each existing track
     * @param muType type from MU
     * @param importType type of imported element
     * @param elemExists indicates if element exists in imported file on current beat
     * @param splitByRests indicates if continious elements of current type should be split by rests
     */
    void buildContiniousElement(ChordRest* cr, std::vector<SLine*>& elements, ElementType muType, LineImportType importType,
                                bool elemExists, bool splitByRests = false);

    void setBeamMode(const GPBeat* beat, ChordRest* cr, Measure* measure, Fraction tick);

    Score* _score;
    std::unique_ptr<GPDomModel> _gpDom;

    enum class SlideHammerOn {
        LegatoSlide, Slide, HammerOn
    };
    std::list<std::pair<Note*, SlideHammerOn> > _slideHammerOnMap;

    GPMasterBar::TimeSig _lastTimeSig;
    GPMasterBar::TripletFeelType _lastTripletFeel = GPMasterBar::TripletFeelType::None;
    std::unordered_map<size_t, GPMasterBar::KeySig> _lastKeySigs;
    std::list<std::pair<Measure*, GPMasterBar::Fermata> > _fermatas;
    std::unordered_multimap<int, GPMasterTracks::Automation> _tempoMap;
    std::unordered_map<track_idx_t, GPBar::Clef> _clefs;
    std::unordered_map<track_idx_t, GPBeat::DynamicType> _dynamics;
    std::unordered_map<track_idx_t, bool> m_hasCapo;
    std::unordered_map<track_idx_t, std::vector<Tie*> > _ties; // map(track, tie)
    std::unordered_map<track_idx_t, std::vector<Tie*> > _harmonicTies; // map(track, tie between harmonic note)
    std::unordered_map<Note*, int> m_originalPitches; // info of changed pitches for keeping track of ties
    std::unordered_map<Chord*, TremoloType> m_tremolosInChords;
    std::unordered_map<track_idx_t, Slur*> _slurs; // map(track, slur)

    mutable GPBeat* m_currentGPBeat = nullptr; // used for passing info from notes
    std::map<track_idx_t, std::map<ElementType, LineImportType> > m_lastImportTypes;

    struct ContiniousElement {
        std::vector<SLine*> elements;
        bool endedOnRest = false;
    };

    std::map<track_idx_t, std::map<LineImportType, ContiniousElement> > m_elementsToAddToScore;

    std::vector<SLine*> m_palmMutes;
    std::vector<SLine*> m_letRings;
    std::vector<SLine*> m_dives;
    std::vector<SLine*> m_pickScrapes;
    std::vector<SLine*> m_rasgueados;
    std::vector<Vibrato*> _vibratos;
    std::map<GPBeat::HarmonicMarkType, std::vector<SLine*> > m_harmonicMarks;
    std::map<GPBeat::OttavaType, std::vector<SLine*> > m_ottavas;
    std::map<GPBeat::Hairpin, std::vector<SLine*> > m_hairpins;
    std::vector<SLine*> m_trillElements;

    std::map<uint16_t, uint16_t > _drumExtension;

    Volta* _lastVolta = nullptr;
    int _lastDiagramIdx = -1;

    struct NextTupletInfo {
        Fraction ratio;
        Fraction duration;                // duration of all current elements
        std::vector<ChordRest*> elements; // elements that will be added to tuplet
        track_idx_t track;
        Tuplet* tuplet = nullptr;
        Measure* measure = nullptr;
        int lowestBase = LOWEST_BASE;     // expected denominator
        static constexpr int LOWEST_BASE = 1024;
    } m_nextTupletInfo;

    std::vector<Bend*> m_bends;
    std::vector<StretchedBend*> m_stretchedBends;
    bool m_useStretchedBends = false;

    static constexpr voice_idx_t VOICES = 4;

    Measure* _lastMeasure = nullptr;
    bool m_showCapo = true; // TODO-gp : settings
    std::unordered_map<Measure*, std::array<int, VOICES> > m_chordsInMeasureByVoice; /// if measure has any chord for specific voice, rests are hidden
    std::unordered_map<Measure*, size_t> m_chordsInMeasure;
    BeamMode m_previousBeamMode = BeamMode::AUTO;

    std::unique_ptr<GPDrumSetResolver> _drumResolver;
};
} //end Ms namespace
#endif // SCOREDOMBUILDER_H
