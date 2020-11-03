#ifndef INOTATIONELEMENTS_H
#define INOTATIONELEMENTS_H

#include <vector>

#include "modularity/imoduleexport.h"
#include "notationtypes.h"

namespace Ms {
class Element;
class Measure;
}

namespace mu {
namespace notation {
class INotationElements
{
public:
    virtual ~INotationElements() = default;

    virtual Ms::Element* search(const std::string& searchCommand) const = 0;
    virtual std::vector<Ms::Element*> searchSimilar(const SearchElementOptions* elementOptions) const = 0;

    virtual Ms::Measure* measure(const int measureIndex) const = 0;
};

using INotationElementsPtr = std::shared_ptr<INotationElements>;
}
}

#endif // INOTATIONELEMENTS_H
