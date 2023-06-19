#ifndef MU_IMPORTEXPORT_GPDOMMODEL_H
#define MU_IMPORTEXPORT_GPDOMMODEL_H

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "gpbar.h"
#include "gpbeat.h"
#include "gpvoice.h"
#include "gpmasterbar.h"
#include "gpmastertracks.h"
#include "gpnote.h"
#include "gpscore.h"
#include "gptrack.h"
#include "inoteproperty.h"

namespace mu::iex::guitarpro {
class GPDomModel
{
public:

    GPDomModel() = default;
    virtual ~GPDomModel() = default;

    GPDomModel(const GPDomModel& gp) = delete;
    GPDomModel& operator=(const GPDomModel&) = delete;

    void addGPScore(std::unique_ptr<GPScore>&& score) { _score = std::move(score); }

    void addGPMasterTracks(std::unique_ptr<GPMasterTracks>&& mTr) { _masterTracks.swap(mTr); }
    void addGPTracks(std::map<int, std::unique_ptr<GPTrack> >&& tr) { _tracks.swap(tr); }
    void addGPMasterBars(std::vector<std::unique_ptr<GPMasterBar> >&& mB) { _masterBars.swap(mB); }

    const GPScore* score() const { return _score.get(); }
    const GPMasterTracks* masterTracks() const { return _masterTracks.get(); }
    const std::map<int, std::unique_ptr<GPTrack> >& tracks() const { return _tracks; }
    const std::vector<std::unique_ptr<GPMasterBar> >& masterBars() const { return _masterBars; }

private:

    std::unique_ptr<GPScore> _score;
    std::unique_ptr<GPMasterTracks> _masterTracks;
    std::map<int, std::unique_ptr<GPTrack> > _tracks;
    std::vector<std::unique_ptr<GPMasterBar> > _masterBars;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GPDOMMODEL_H
