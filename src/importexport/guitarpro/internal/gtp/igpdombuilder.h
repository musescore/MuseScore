#ifndef IGPDOMBUILDER_H
#define IGPDOMBUILDER_H

#include <memory>

#include "gpdommodel.h"

namespace HackQt {
class TabDomNodeHack;
class TabDomElementHack;
}

namespace Ms {
class IGPDomBuilder
{
public:
    virtual ~IGPDomBuilder() = default;
    virtual void buildGPDomModel(QDomElement* qdomElem) = 0;
    virtual std::unique_ptr<GPDomModel> getGPDomModel() = 0;
};
} // end MSTab namespace
#endif // IGPDOMBUILDER_H
