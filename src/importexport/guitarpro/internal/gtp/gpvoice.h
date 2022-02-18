#ifndef GPVOICE_H
#define GPVOICE_H

#include <vector>

#include "gpbeat.h"

namespace Ms {
class GPVoice
{
public:

    void addGPBeat(const std::shared_ptr<GPBeat>& b) { _beats.push_back(b); }

    void setId(int id) { _id = id; }

    const std::vector<std::shared_ptr<GPBeat> >& beats() const { return _beats; }

private:

    int _id{ -1 };
    std::vector<std::shared_ptr<GPBeat> > _beats;
};
}

#endif // GPVOICE_H
