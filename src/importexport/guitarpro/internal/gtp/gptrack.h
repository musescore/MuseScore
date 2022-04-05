#ifndef GPTRACK_H
#define GPTRACK_H

#include <unordered_map>
#include <string>

namespace Ms {
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
    };

    struct String {
        int num{ 0 };
        int tunning{ 0 };
    };

    struct Diagram {
        int id{ 0 };
        QString name;
        int stringCount{ 0 };
        int fretCount{ 0 };
        int baseFret{ 0 };
        std::unordered_map<int, int> frets;
        // there are some other tags
        // which cant be used in MU
    };

    GPTrack(int idx)
        : _idx(idx) {}
    virtual ~GPTrack() = default;

    void setName(const QString& n) { _name = n; }
    QString name() const { return _name; }

    void setShortName(const QString& s) { _shortName = s; }
    QString shortName() const { return _shortName; }

    void setInstrument(const QString& s) { _instrument = s; }
    QString instrument() const { return _instrument; }

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

    std::vector<String> strings() const
    {
        std::vector<String> ss;
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

    void setTransponce(int t) { _transponce = t; }
    int transponce() const { return _transponce; }

    void addDiagram(std::pair<int, Diagram>&& d) { _diagrams.insert(d); }
    const std::unordered_map<int, Diagram>& diagram() const { return _diagrams; }

    void setLyrics(const std::string& l) { _lyrics = l; }
    const std::string& lyrics() const { return _lyrics; }

    void setLyricsOffset(int n) { _lyricsOffset = n; }
    int lyricsOffset() const { return _lyricsOffset; }

    int idx() const { return _idx; }

protected:

    QString _name;
    QString _shortName;
    QString _instrument;
    RSE _rse;
    int _programm{ 0 };
    int _midiChannel{ 0 };
    bool _isGuitar{ false };
    int _idx{ -1 };
    size_t _staffCount{ 1 };
    std::vector<StaffProperty> _staffProperty;
    int _transponce{ 0 };
    std::unordered_map<int, Diagram> _diagrams;
    std::string _lyrics;
    int _lyricsOffset = { 0 };
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
}
#endif // GPTRACK_H
