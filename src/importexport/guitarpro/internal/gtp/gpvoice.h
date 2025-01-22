#ifndef MU_IMPORTEXPORT_GPVOICE_H
#define MU_IMPORTEXPORT_GPVOICE_H

#include <vector>

#include "gpbeat.h"

namespace mu::iex::guitarpro {
class GPVoice
{
public:

    void addGPBeat(const std::shared_ptr<GPBeat>& b) { _beats.push_back(b); }

    void setId(int id) { _id = id; }
    void setPosition(int pos) { _pos = pos; }
    int position() const { return _pos; }

    const std::vector<std::shared_ptr<GPBeat> >& beats() const { return _beats; }

private:

    int _id = -1; // imported id
    int _pos = 0; // for defining correct track number
    std::vector<std::shared_ptr<GPBeat> > _beats;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPVOICE_H
