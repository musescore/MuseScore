#ifndef MU_IMPORTEXPORT_GPNOTE_H
#define MU_IMPORTEXPORT_GPNOTE_H

#include <unordered_set>
#include <memory>
#include <bitset>

#include <global/types/string.h>

#include "inoteproperty.h"

namespace mu::iex::guitarpro {
class GPNote
{
public:
    enum class TieType {
        None, Start, Mediate, End
    };
    enum class VibratoType {
        None, Slight, Wide
    };
    enum class Ornament {
        None, LowerMordent, UpperMordent, InvertedTurn, Turn
    };
    enum class HammerOn {
        None, Start, End
    };

    enum class PickScrape {
        None, Down, Up
    };

    struct Harmonic {
        enum class Type {
            None, Natural, Artificial, Pinch, Tap, Semi, FeedBack, Types
        };
        float fret{ 0 };
        Type type{ Type::None };

        static bool isArtificial(Type type)
        {
            return type == Type::Artificial
                   || type == Type::Pinch
                   || type == Type::Tap
                   || type == Type::Semi
                   || type == Type::FeedBack;
        }
    };
    struct MidiPitch {
        int midi{ -1 };
        int string{ -1 };
        int fret{ -1 };
        int octave{ -1 };
        int tone{ 0 };
        int element{ 0 };
        int variation{ -1 };
    };
    struct Bend {
        float destinationOffset{ -1 };
        float destinationValue{ -1 };
        float middleOffset1{ -1 };
        float middleOffset2{ -1 };
        float middleValue{ -1 };
        float originOffset{ -1 };
        float originValue{ -1 };
        bool isEmpty() const
        {
            return destinationValue == -1
                   && destinationOffset == -1
                   && middleValue == -1
                   && middleOffset1 == -1
                   && middleOffset2 == -1
                   && originOffset == -1
                   && originValue == -1;
        }
    };
    struct Trill {
        int auxillaryFret{ -1 };
        int speed{ -1 };
    };

    void addProperties(std::unordered_set<std::unique_ptr<INoteProperty> >&& pr) { _properties.swap(pr); }
    void setTieType(GPNote::TieType t) { _tie = t; }

    void setMidi(int m) { _midiPitch.midi = m; }
    int midi() const { return _midiPitch.midi; }
    void setString(int s) { _midiPitch.string = s; }
    int string() const { return _midiPitch.string; }
    void setFret(int f) { _midiPitch.fret = f; }
    int fret() const { return _midiPitch.fret; }
    void setOctave(int o) { _midiPitch.octave = o; }
    void setTone(int t) { _midiPitch.tone = t; }
    void setVariation(int t) { _midiPitch.variation = t; }
    void setElement(int t) { _midiPitch.element = t; }

    void setAccidental(int acc) { _accidental = acc; }
    int accidental() const { return _accidental; }

    void setHarmonicFret(float f) { _harmonic.fret = f; }
    void setHarmonicType(Harmonic::Type t) { _harmonic.type = t; }
    const Harmonic& harmonic() const { return _harmonic; }

    void setLetRing(bool lr) { _letRing = lr; }
    bool letRing() const { return _letRing; }

    void setPalmMute(bool pm) { _palmMuted = pm; }
    bool palmMute() const { return _palmMuted; }

    const MidiPitch& midiPitch() const { return _midiPitch; }
    TieType tieType() const { return _tie; }

    void setBend(std::unique_ptr<Bend>&& b) { _bend.swap(b); }
    const Bend* bend() const { return _bend.get(); }

    void setGhostNote(bool b) { _ghostNote = b; }
    bool ghostNote() const { return _ghostNote; }

    void setMute(bool m) { _muted = m; }
    //! muted = dead note
    bool muted() const { return _muted; }

    void setTapping(bool t) { _tapping = t; }
    bool tapping() const { return _tapping; }

    void setAccent(const std::bitset<4>& b) { _accent = b; }
    const std::bitset<4>& accents() const { return _accent; }

    void setSlides(const std::bitset<6>& s) { _slides = s; }
    const std::bitset<6>& slides() const { return _slides; }

    void setLeftFingering(const muse::String& ch) { _leftFingering = ch; }
    const muse::String& leftFingering() const { return _leftFingering; }

    void setRightFingering(const muse::String& ch) { _rightFingering = ch; }
    const muse::String& rightFingering() const { return _rightFingering; }

    void setShowStringNumber(bool show) { m_showStringNumber = show; }
    bool showStringNumber() const { return m_showStringNumber; }

    void setVibratoType(VibratoType v) { _vibrato = v; }
    VibratoType vibratoType() const { return _vibrato; }

    void setTrillFret(int fr) { _trill.auxillaryFret = fr; }
    void setTrillSpeed(int tr) { _trill.speed = tr; }
    Trill trill() const { return _trill; }

    void setOrnament(Ornament o) { _ornament = o; }
    Ornament ornament() const { return _ornament; }

    void setLeftHandTapped(bool v) { _leftHandTapped = v; }
    bool leftHandTapped() const { return _leftHandTapped; }

    void setHammerOn(HammerOn h) { _hammer = h; }
    HammerOn hammerOn() const { return _hammer; }

    void setPickScrape(PickScrape p) { _pickScrape = p; }
    PickScrape pickScrape() const { return _pickScrape; }

    void setId(int id) { _id = id; }
    int id() const { return _id; }

    const std::unordered_set<std::unique_ptr<INoteProperty> >& properties() const { return _properties; }
    static constexpr int invalidAccidental = -10;

private:
    int _id{ -1 };
    TieType _tie{ TieType::None };
    std::unordered_set<std::unique_ptr<INoteProperty> > _properties;
    MidiPitch _midiPitch;

    int _accidental = invalidAccidental;
    Harmonic _harmonic;
    std::unique_ptr<Bend> _bend;
    bool _letRing{ false };
    bool _palmMuted{ false };
    bool _ghostNote{ false };
    bool _muted{ false };
    bool _tapping{ false };
    //[0] - staccato, [1] - unknown, [2] - heavily accidental, [3] - accidental
    std::bitset<4> _accent{ 0 };
    //[0] shifSlide, [1] - legatoSlide, [2] - slideDownWard, [3] - slidewUpWard, [4] - slideInFormBelow, [5] - slideInFormAbove,
    std::bitset<6> _slides{ 0 };
    muse::String _leftFingering;
    muse::String _rightFingering;
    VibratoType _vibrato{ VibratoType::None };
    Trill _trill;
    Ornament _ornament{ Ornament::None };
    bool _leftHandTapped{ false };
    HammerOn _hammer{ HammerOn::None };
    PickScrape _pickScrape{ PickScrape::None };
    bool m_showStringNumber = false;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPNOTE_H
