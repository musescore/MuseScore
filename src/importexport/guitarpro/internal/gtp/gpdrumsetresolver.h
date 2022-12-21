#ifndef GPDRUMSETRESOLVER_H
#define GPDRUMSETRESOLVER_H

#include <map>
#include <list>

#include "types/string.h"

namespace mu::engraving {
struct GPDrum {
    int32_t pitch = -1;
    int32_t element = -1;
    int32_t variation = -1;
    String _name;

    GPDrum() {}
    GPDrum(int32_t pitch, int32_t element, int32_t variation, const String& name = u"drmkt")
        : pitch(pitch), element(element), variation(variation), _name(name) {}
};

class GPDrumSetResolver
{
public:

    GPDrumSetResolver() = default;
    virtual ~GPDrumSetResolver() = default;

    void initGPDrum();

    int32_t pitch(int32_t element, int32_t variation, const String& name) const;

private:

    void addDrum(const GPDrum& drum);

    std::map<String, std::list<GPDrum> > _drum;
};
} // end mu::engraving namespace
#endif // GPDRUMSETRESOLVER_H
