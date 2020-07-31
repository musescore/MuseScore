#ifndef INOTATIONELEMENTS_H
#define INOTATIONELEMENTS_H

#include "modularity/imoduleexport.h"

namespace Ms{
class Measure;
}

namespace mu {
namespace domain {
namespace notation {
class INotationElementsRepository
{
public:
    virtual ~INotationElementsRepository() = default;

    virtual Ms::Measure* measureByIndex(const int measureIndex) const = 0;
};
}
}
}

#endif // INOTATIONELEMENTS_H
