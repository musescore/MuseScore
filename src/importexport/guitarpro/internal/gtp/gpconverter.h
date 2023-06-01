#ifndef MU_IMPORTEXPORT_GPCONVERTER_H
#define MU_IMPORTEXPORT_GPCONVERTER_H

#include <unordered_map>

#include "gpmasterbar.h"
#include "gpbar.h"
#include "gpbeat.h"
#include "gpdrumsetresolver.h"
#include "gpmastertracks.h"
#include "types/fraction.h"

#include "libmscore/vibrato.h"

#include "iengravingconfiguration.h"

namespace mu::iex::guitarpro {
class GPScore;
class GPTrack;
class GPDomModel;

class GPConverter
{
    INJECT(mu::engraving::IEngravingConfiguration, engravingConfiguration);

public:
    GPConverter(mu::engraving::Score* score, std::unique_ptr<GPDomModel>&& gpDom);

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
    using track_idx_t = mu::engraving::track_idx_t;
    using Fraction = mu::engraving::Fraction;
    using Measure = mu::engraving::Measure;
    using ChordRest = mu::engraving::ChordRest;
    using Note = mu::engraving::Note;

    using ChordRestContainer = std::vector<std::pair<mu::engraving::ChordRest*, const GPBeat*> >;
    using TieMap = std::unordered_map<track_idx_t, std::vector<mu::engraving::Tie*> >;

    struct Context {
        int32_t masterBarIndex{ 0 };
        track_idx_t curTrack{ 0 };
        Fraction curTick;
    };

    void convert(const std::vector<std::unique_ptr<GPMasterBar> >& mBs);
    void clearDefectedSpanner();

    void convertMasterBar(const GPMasterBar* mB, Context ctx);
    void fixEmptyMeasures();
    void convertBars(const std::vector<std::unique_ptr<GPBar> >& bars, Context ctx);
    void convertBar(const GPBar* bar, Context ctx);
    void convertVoices(const std::vector<std::unique_ptr<GPVoice> >&, Context ctx);
    void convertVoice(const GPVoice*, Context ctx);
    void convertBeats(const std::vector<std::shared_ptr<GPBeat> >& beats, Context ctx);
    Fraction convertBeat(const GPBeat* beat, ChordRestContainer& graceChords, Context ctx);
    void configureGraceChord(const GPBeat* beat, ChordRest* cr, GPBeat::OttavaType type);
    void convertNotes(const std::vector<std::shared_ptr<GPNote> >& notes, ChordRest* cr);
    void convertNote(const GPNote* note, mu::engraving::ChordRest* cr);

    void setUpGPScore(const GPScore* gpscore);
    void setUpTracks(const std::map<int, std::unique_ptr<GPTrack> >& tracks);
    void setUpTrack(const std::unique_ptr<GPTrack>& tracks);
    void collectTempoMap(const GPMasterTracks* mTr);
    void collectFermatas(const GPMasterBar* mB, mu::engraving::Measure* measure);
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
    void addVibratoByType(const Note* note, mu::engraving::VibratoType type);
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
    void setupTupletStyle(mu::engraving::Tuplet* tuplet);
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
    void buildContiniousElement(ChordRest* cr, std::vector<mu::engraving::SLine*>& elements, mu::engraving::ElementType muType,
                                LineImportType importType, bool elemExists, bool splitByRests = false);

    void setBeamMode(const GPBeat* beat, ChordRest* cr, Measure* measure, Fraction tick);

    mu::engraving::Score* _score;
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
    std::unordered_map<track_idx_t, std::vector<mu::engraving::Tie*> > _ties; // map(track, tie)
    std::unordered_map<track_idx_t, std::vector<mu::engraving::Tie*> > _harmonicTies; // map(track, tie between harmonic note)
    std::unordered_map<Note*, int> m_originalPitches; // info of changed pitches for keeping track of ties
    std::unordered_map<mu::engraving::Chord*, mu::engraving::TremoloType> m_tremolosInChords;
    std::unordered_map<track_idx_t, mu::engraving::Slur*> _slurs; // map(track, slur)

    mutable GPBeat* m_currentGPBeat = nullptr; // used for passing info from notes
    std::unordered_map<track_idx_t, std::unordered_map<mu::engraving::ElementType, LineImportType> > m_lastImportTypes;

    struct ContiniousElement {
        std::vector<mu::engraving::SLine*> elements;
        bool endedOnRest = false;
    };

    std::unordered_map<track_idx_t, std::unordered_map<LineImportType, ContiniousElement> > m_elementsToAddToScore;

    std::vector<mu::engraving::SLine*> m_palmMutes;
    std::vector<mu::engraving::SLine*> m_letRings;
    std::vector<mu::engraving::SLine*> m_dives;
    std::vector<mu::engraving::SLine*> m_pickScrapes;
    std::vector<mu::engraving::SLine*> m_rasgueados;
    std::vector<mu::engraving::Vibrato*> _vibratos;
    std::unordered_map<GPBeat::HarmonicMarkType, std::vector<mu::engraving::SLine*> > m_harmonicMarks;
    std::unordered_map<GPBeat::OttavaType, std::vector<mu::engraving::SLine*> > m_ottavas;
    std::unordered_map<GPBeat::Hairpin, std::vector<mu::engraving::SLine*> > m_hairpins;
    std::vector<mu::engraving::SLine*> m_trillElements;

    std::map<uint16_t, uint16_t > _drumExtension;
    mu::engraving::Volta* _lastVolta = nullptr;
    int _lastDiagramIdx = -1;

    struct NextTupletInfo {
        Fraction ratio;
        Fraction duration;                // duration of all current elements
        std::vector<ChordRest*> elements; // elements that will be added to tuplet
        track_idx_t track;
        mu::engraving::Tuplet* tuplet = nullptr;
        mu::engraving::Measure* measure = nullptr;
        int lowestBase = LOWEST_BASE;     // expected denominator
        static constexpr int LOWEST_BASE = 1024;
    } m_nextTupletInfo;

    // Index is the number of sharps. Using flat keysigs for signatures with double sharps
    std::vector<int> m_sharpsToKeyConverter{ 0, 1, 2, 3, 4, 5, 6, 7, -4, -3, -2, -1 };
    // Index is the number of sharps. Using sharp keysigs for signatures with double flats
    std::vector<int> m_sharpsToFlatKeysConverter{ 0, 1, 2, 3, 4, -7, -6, -5, -4, -3, -2, -1 };

    std::vector<mu::engraving::Bend*> m_bends;
    std::vector<mu::engraving::StretchedBend*> m_stretchedBends;
    bool m_useStretchedBends = false;

    static constexpr mu::engraving::voice_idx_t VOICES = 4;

    Measure* _lastMeasure = nullptr;
    bool m_showCapo = true; // TODO-gp : settings
    std::unordered_map<Measure*, std::array<int, VOICES> > m_chordsInMeasureByVoice; /// if measure has any chord for specific voice, rests are hidden
    std::unordered_map<Measure*, size_t> m_chordsInMeasure;
    mu::engraving::BeamMode m_previousBeamMode = mu::engraving::BeamMode::AUTO;

    std::unique_ptr<GPDrumSetResolver> _drumResolver;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GPCONVERTER_H
