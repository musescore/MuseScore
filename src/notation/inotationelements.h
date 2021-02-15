#ifndef INOTATIONELEMENTS_H
#define INOTATIONELEMENTS_H

#include <vector>

#include "modularity/imoduleexport.h"
#include "notationtypes.h"

namespace mu::notation {
class INotationElements
{
public:
    virtual ~INotationElements() = default;

    virtual Ms::Score* msScore() const = 0;

    virtual Element* search(const std::string& searchText) const = 0;
    virtual std::vector<Element*> elements(const FilterElementsOptions& elementOptions = FilterElementsOptions()) const = 0;

    virtual Measure* measure(const int measureIndex) const = 0;

    virtual PageList pages() const = 0;
};

using INotationElementsPtr = std::shared_ptr<INotationElements>;
}

#endif // INOTATIONELEMENTS_H
