#ifndef MU_IMPORTEXPORT_GPTRACK_H
#define MU_IMPORTEXPORT_GPTRACK_H

#include <unordered_map>
#include <string>

#include "types/string.h"

namespace mu::iex::guitarpro {
class GPTrack
{
public:
    struct RSE {
        float pan{ 0 };
        float volume{ 0 };
        float chorus{ 0 };
        float reverb{ 0 };
        float phase{ 0 };
        float tremolo{ 0 };
    };

    struct StaffProperty {
        int fretCount{ 24 };
        int capoFret{ 0 };
        std::vector<int> tunning;
        bool useFlats{ false };
        bool ignoreFlats{ false };
    };

    struct InstrumentString {
        int num{ 0 };
        int tunning{ 0 };
    };

    struct Diagram {
        int id{ 0 };
        muse::String name;
        int stringCount{ 0 };
        int fretCount{ 0 };
        int baseFret{ 0 };
        std::unordered_map<int, int> frets;
        // there are some other tags
        // which cant be used in MU
    };

    struct Sound {
        int programm{ 0 };
        muse::String name;
        muse::String label;
        muse::String path;
        muse::String role;
    };

    struct SoundAutomation {
        muse::String type;
        muse::String value;
        int bar = 0;
        bool linear = 0;
        float position = 0;
    };

    GPTrack(int idx)
        : _idx(idx) {}
    virtual ~GPTrack() = default;

    void setName(const muse::String& n) { _name = n; }
    muse::String name() const { return _name; }

    void setShortName(const muse::String& s) { _shortName = s; }
    muse::String shortName() const { return _shortName; }

    void setInstrument(const muse::String& s) { _instrument = s; }
    muse::String instrument() const { return _instrument; }

    void setRSE(const RSE& r) { _rse = r; }
    const RSE& rse() const { return _rse; }

    void setProgramm(int pr) { _programm = pr; }
    int programm() const { return _programm; }

    void setMidiChannel(int ch) { _midiChannel = ch; }
    int midiChannel() const { return _midiChannel; }

    void setIsGuitar(bool arg) { _isGuitar = arg; }
    bool isGuitar() const { return _isGuitar; }

    void addStaffProperty(const StaffProperty& st) { _staffProperty.push_back(st); }
    const std::vector<StaffProperty>& staffProperty() const { return _staffProperty; }

    void addSound(Sound sound);

    struct SoundAutomationPos {
        int bar = 0;
        float pos = 0;

        bool operator<(const SoundAutomationPos& other) const { return std::tie(bar, pos) < std::tie(other.bar, other.pos); }
    };

    void addSoundAutomation(SoundAutomation val) { _automations.insert({ { val.bar, val.position }, val }); }
    const std::unordered_map<muse::String, Sound>& sounds() { return _sounds; }
    const std::map<SoundAutomationPos, SoundAutomation>& soundAutomations() { return _automations; }

    std::vector<InstrumentString> strings() const
    {
        std::vector<InstrumentString> ss;
        if (_staffProperty.empty()) {
            return ss;
        }

        const StaffProperty& sp = _staffProperty.at(0);
        for (size_t i = 0; i < sp.tunning.size(); ++i) {
            ss.push_back({ static_cast<int>(i + 1), sp.tunning.at(i) });
        }

        return ss;
    }

    void setStaffCount(size_t n) { _staffCount = n; }
    size_t staffCount() const { return _staffCount; }

    void setTranspose(int t) { _transpose = t; }
    int transpose() const { return _transpose; }

    void addDiagram(std::pair<int, Diagram>&& d) { _diagrams.insert(d); }
    const std::unordered_map<int, Diagram>& diagram() const { return _diagrams; }

    void setLyrics(const std::string& l) { _lyrics = l; }
    const std::string& lyrics() const { return _lyrics; }

    void setLyricsOffset(int n) { _lyricsOffset = n; }
    int lyricsOffset() const { return _lyricsOffset; }

    void setLineCount(int n) { _lineCount = n; }
    int lineCount() const { return _lineCount; }

    int idx() const { return _idx; }

protected:

    muse::String _name;
    muse::String _shortName;
    muse::String _instrument;
    RSE _rse;
    int _programm{ 0 };
    int _midiChannel{ 0 };
    bool _isGuitar{ false };
    int _idx{ -1 };
    size_t _staffCount{ 1 };
    std::vector<StaffProperty> _staffProperty;
    std::unordered_map<muse::String, Sound> _sounds;
    std::map<SoundAutomationPos, SoundAutomation> _automations;
    int _transpose{ 0 };
    std::unordered_map<int, Diagram> _diagrams;
    std::string _lyrics;
    int _lyricsOffset = { 0 };
    int _lineCount{ 5 }; // for percussion lines
};

class GP6Track : public GPTrack
{
public:
    GP6Track(int idx)
        : GPTrack(idx) {}

private:
};

class GP7Track : public GPTrack
{
public:
    GP7Track(int idx)
        : GPTrack(idx) {}

private:
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GPTRACK_H
