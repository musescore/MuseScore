#ifndef GPMASTERTRACKS_H
#define GPMASTERTRACKS_H

#include <vector>

namespace Ms {
class GPMasterTracks
{
public:
    struct Automation {
        enum class Type {
            tempo, volume, pan
        };
        int bar{ 0 };
        Type type{ Type::tempo };
        float position{ 0 };
        bool linear{ false };
        int value{ 0 };
        int tempoUnit{ 0 };
        friend bool operator<(const Automation& l, const Automation& r) { return l.bar < r.bar; }
    };

    GPMasterTracks() = default;
    ~GPMasterTracks() = default;
    GPMasterTracks(const GPMasterTracks& masterTrack) = default;
    GPMasterTracks(GPMasterTracks&& masterTrack) = default;
    GPMasterTracks& operator=(const GPMasterTracks& mT) = default;
    GPMasterTracks& operator=(GPMasterTracks&& mT) = default;

    void setTempoMap(std::vector<Automation>&& tM) { _tempoMap.swap(tM); }
    const std::vector<Automation>& tempoMap() const { return _tempoMap; }
    void setTracksCount(size_t tc) { _tracksCount = tc; }
    size_t tracksCount() { return _tracksCount; }

private:
    std::vector<Automation> _tempoMap;
    size_t _tracksCount;
};
}

#endif // GPMASTERTRACKS_H
