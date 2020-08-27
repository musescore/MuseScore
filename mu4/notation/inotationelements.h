#ifndef INOTATIONELEMENTS_H
#define INOTATIONELEMENTS_H

#include "modularity/imoduleexport.h"

namespace Ms {
class Measure;
}

namespace mu {
namespace notation {
class INotationElements
{
public:
    virtual ~INotationElements() = default;

    virtual Ms::Measure* measureByIndex(const int measureIndex) const = 0;
};
}
}

#endif // INOTATIONELEMENTS_H
