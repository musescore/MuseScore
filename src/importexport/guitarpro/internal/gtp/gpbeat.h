#ifndef MU_IMPORTEXPORT_GPBEAT_H
#define MU_IMPORTEXPORT_GPBEAT_H

#include <map>
#include <memory>
#include <string>

#include "gpnote.h"
#include "gprhythm.h"

namespace mu::iex::guitarpro {
class GPBeat
{
public:
    enum class DynamicType {
        FFF, FF, F, MF, MP, P, PP, PPP
    };
    enum class LegatoType {
        None, Start, Mediate, End
    };
    enum class Arpeggio {
        None, Up, Down
    };
    enum class Brush {
        None, Up, Down
    };
    enum class GraceNotes {
        None, OnBeat, BeforeBeat
    };
    enum class VibratoWTremBar {
        None, Slight, Wide
    };
    enum class VibratoLeftHand {
        None, Slight, Wide
    };
    enum class Fadding {
        None, FadeIn, FadeOut, VolumeSwell
    };
    enum class Hairpin {
        None, Crescendo, Decrescendo
    };
    enum class PickStroke {
        None, Up, Down
    };
    enum class Wah {
        None, Open, Closed
    };
    enum class Golpe {
        None, Finger, Thumb
    };
    enum class Rasgueado {
        None, II_1, MII_1, MII_2, PMP_1, PMP_2, PEI_1, PEI_2, PAI_1, PAI_2, AMI_1,
        AMI_2, PPP_1, AMII_1, AMIP_1, EAMI_1, EAMII_1, PEAMI_1
    };

    enum class OttavaType {
        None, ma15, va8, vb8, mb15
    };

    enum class HarmonicMarkType {
        None, Artificial, Pinch, Tap, Semi, FeedBack
    };

    struct HarmonicMarkInfo {
        bool artificial = false;
        bool pinch = false;
        bool tap = false;
        bool semi = false;
        bool feedback = false;
    };

    struct Tremolo {
        int numerator{ -1 };
        int denominator{ -1 };
    };

    struct Barre {
        int fret{ -1 };
        int string{ -1 };
    };

    struct StemOrientation {
        bool up = false; /// taken in consideration only if userDefined == true
        bool userDefined = false;
    };

    enum class BeamMode {
        AUTO,
        JOINED,
        BROKEN,
        BROKEN2,
        BROKEN2_JOINED
    };

    struct {
        bool operator()(const std::shared_ptr<GPNote>& a, const std::shared_ptr<GPNote>& b) const
        {
            auto aPitch = a->midiPitch().octave * 12 + a->midiPitch().tone;
            auto bPitch = b->midiPitch().octave * 12 + b->midiPitch().tone;
            return aPitch > bPitch;
        }
    } comparePitch;

    void addGPNote(const std::shared_ptr<GPNote>& n) { _notes.push_back(n); }
    void sortGPNotes();
    void addGPRhythm(const std::shared_ptr<GPRhythm>& n) { _rhythm = n; }
    void setDynamic(GPBeat::DynamicType t) { _dynamic = t; }
    void setLegatoType(GPBeat::LegatoType t) { _legato = t; }
    void setOttavaType(GPBeat::OttavaType ottavaType) { _ottavaType = ottavaType; }
    GPBeat::OttavaType ottavaType() const { return _ottavaType; }

    bool isRest() const { return _notes.empty(); }

    DynamicType dynamic() const { return _dynamic; }
    LegatoType legatoType() const { return _legato; }

    std::pair<int, GPRhythm::RhytmType> lenth() const;
    GPRhythm::Tuplet tuplet() const;

    void setLetRing(bool letRing) { _letRing = letRing; }
    bool letRing() const { return _letRing; }

    void setPalmMute(bool palmMute) { _palmMute = palmMute; }
    bool palmMute() const { return _palmMute; }

    void setTrill(bool trill) { _trill = trill; }
    bool trill() const { return _trill; }

    void addHarmonicMarkType(GPBeat::HarmonicMarkType type);
    bool harmonicMarkArtificial() const { return _harmonicMarkInfo.artificial; }
    bool harmonicMarkPinch() const { return _harmonicMarkInfo.pinch; }
    bool harmonicMarkTap() const { return _harmonicMarkInfo.tap; }
    bool harmonicMarkSemi() const { return _harmonicMarkInfo.semi; }
    bool harmonicMarkFeedback() const { return _harmonicMarkInfo.feedback; }

    void setSlapped(bool s) { _slapped = s; }
    bool slapped() const { return _slapped; }

    void setPopped(bool s) { _popped = s; }
    bool popped() const { return _popped; }

    void setArpeggio(Arpeggio ar) { _arpeggio = ar; }
    Arpeggio arpeggio() const { return _arpeggio; }

    void setArpeggioStretch(double arpeggioStretch) { _arpeggioStretch = arpeggioStretch; }
    double arpeggioStretch() const { return _arpeggioStretch; }

    void setBrush(Brush br) { _brush = br; }
    Brush brush() const { return _brush; }

    void setGraceNotes(GraceNotes gn) { _graceNotes = gn; }
    GraceNotes graceNotes() const { return _graceNotes; }

    void setFreeText(const muse::String& s) { _freeText = s; }
    const muse::String& freeText() const { return _freeText; }

    void setTime(int t) { _time = t; }
    int time() const { return _time; }

    void setVibratoWTremBar(VibratoWTremBar v) { _vibratoWTremBar = v; }
    VibratoWTremBar vibratoWTremBar() const { return _vibratoWTremBar; }

    void setVibratoLeftHand(VibratoLeftHand v) { _vibratoLeftHand = v; }
    VibratoLeftHand vibratoLeftHand() const { return _vibratoLeftHand; }

    void setFadding(Fadding f) { _fadding = f; }
    Fadding fadding() const { return _fadding; }

    void setHairpin(Hairpin h) { _hairpin = h; }
    Hairpin hairpin() const { return _hairpin; }

    void setRasgueado(Rasgueado r) { _rasgueado = r; }
    Rasgueado rasgueado() const { return _rasgueado; }

    void setPickStroke(PickStroke p) { _pickStroke = p; }
    PickStroke pickStroke() const { return _pickStroke; }

    void setTremolo(Tremolo tr) { _tremolo = tr; }
    Tremolo tremolo() const { return _tremolo; }

    void setWah(Wah w) { _wah = w; }
    Wah wah() const { return _wah; }

    void setGolpe(Golpe g) { m_golpe = g; }
    Golpe golpe() const { return m_golpe; }

    void setBarreFret(int v) { _barre.fret = v; }
    void setBarreString(int v) { _barre.string = v; }
    Barre barre() const { return _barre; }

    void setId(int id) { _id = id; }
    int id() const { return _id; }

    void setDive(bool dive) { m_dive = dive; }
    bool dive() const { return m_dive; }

    void setPickScrape(bool pickScrape) { m_pickScrape = pickScrape; }
    bool pickScrape() const { return m_pickScrape; }

    void setDeadSlapped(bool deadSlapped) { m_deadSlapped = deadSlapped; }
    bool deadSlapped() const { return m_deadSlapped; }

    void setStemOrientationUp(bool up) { m_stemOrientation.up = up; }
    void setStemOrientationUserDefined(bool userDefined) { m_stemOrientation.userDefined = userDefined; }
    bool stemOrientationUp() const { return m_stemOrientation.up; }
    bool stemOrientationUserDefined() const { return m_stemOrientation.userDefined; }

    void setBeamMode(BeamMode mode) { m_beamMode = mode; }
    BeamMode beamMode() const { return m_beamMode; }

    const std::vector<std::shared_ptr<GPNote> >& notes() const { return _notes; }

    //! NOTE In GP version 6.2.0+ already arranged lyrics are written into bits, so the lyrics field of the beat is enough.
    //! In earlier versions of GP, there was no arrangement of lyrics by bits,
    //! i.e. it was necessary to parse the general lyrics each time a tab was opened and arrange them by bits.
    //! At the same time, the same beat can be used in different bars, even if it has different lyrics
    //! (i.e. the lyrics did not participate in determining the uniqueness of the beat).
    //! Therefore, in order to write lyrics into such bits, a key was added - the bar number (master bar)

    struct Key {
        int32_t trackId{ -1 };
        int32_t mbarId{ -1 };
        Key() {}
        Key(int32_t ti, int32_t mi)
            : trackId(ti), mbarId(mi) {}
        bool operator <(const Key& other) const
        {
            if (trackId != other.trackId) {
                return trackId < other.trackId;
            }
            return mbarId < other.mbarId;
        }
    };

    void setLyrics(const std::string& ly) { setLyrics(-1, -1, ly); }
    void setLyrics(int32_t ti, int32_t mi, const std::string& ly) { _lyrics[Key(ti, mi)] = ly; }
    const std::string& lyrics(int32_t ti = -1, int32_t mi = -1) const
    {
        Key key(ti, mi);
        if (_lyrics.find(key) == _lyrics.end()) {
            if (ti != -1 && mi != -1) {
                return lyrics(-1, -1);
            }

            static std::string dummy;
            return dummy;
        }
        return _lyrics.at(key);
    }

    //! NOTE Same problem as with lyrics
    void setDiagramIdx(int idx) { setDiagramIdx(-1, -1, idx); }
    void setDiagramIdx(int32_t ti, int32_t mi, int idx) { _diagramIdx[Key(ti, mi)] = idx; }
    int diagramIdx(int32_t ti = -1, int32_t mi = -1) const
    {
        Key key(ti, mi);
        if (_diagramIdx.find(key) == _diagramIdx.end()) {
            if (ti != -1 && mi != -1) {
                return diagramIdx(-1, -1);
            }

            return -1;
        }
        return _diagramIdx.at(key);
    }

private:
    int _id = -1;
    std::vector<std::shared_ptr<GPNote> > _notes;
    std::map<Key, std::string> _lyrics;
    std::map<Key, int> _diagramIdx;
    std::shared_ptr<GPRhythm> _rhythm;
    DynamicType _dynamic = DynamicType::MF;
    LegatoType _legato = LegatoType::None;
    OttavaType _ottavaType = OttavaType::None;
    bool _letRing = false;
    bool _palmMute = false;
    bool _trill = false;
    bool _slapped = false;
    bool _popped = false;
    HarmonicMarkInfo _harmonicMarkInfo;
    Arpeggio _arpeggio = Arpeggio::None;
    Brush _brush = Brush::None;
    GraceNotes _graceNotes = GraceNotes::None;
    int _time = -1;
    muse::String _freeText;
    VibratoWTremBar _vibratoWTremBar = VibratoWTremBar::None;
    VibratoLeftHand _vibratoLeftHand = VibratoLeftHand::None;
    Fadding _fadding = Fadding::None;
    Hairpin _hairpin = Hairpin::None;
    Rasgueado _rasgueado = Rasgueado::None;
    PickStroke _pickStroke = PickStroke::None;
    Tremolo _tremolo;
    Wah _wah = Wah::None;
    Golpe m_golpe = Golpe::None;
    Barre _barre;
    double _arpeggioStretch = 0.0;
    bool m_dive = false; // TODO-gp: implement dives
    bool m_pickScrape = false;
    bool m_deadSlapped = false;
    StemOrientation m_stemOrientation;
    BeamMode m_beamMode = BeamMode::AUTO;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPBEAT_H
