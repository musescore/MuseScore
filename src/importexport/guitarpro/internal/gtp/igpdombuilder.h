#ifndef IGPDOMBUILDER_H
#define IGPDOMBUILDER_H

#include <memory>

#include "gpdommodel.h"

namespace HackQt {
class TabDomNodeHack;
class TabDomElementHack;
}

namespace mu::engraving {
class IGPDomBuilder
{
public:

    using TabImportOption = GPDomModel::TabImportOption;
    using GPProperties = GPDomModel::GPProperties;

    virtual ~IGPDomBuilder() = default;
    virtual void buildGPDomModel(QDomElement* qdomElem) = 0;
    virtual std::unique_ptr<GPDomModel> getGPDomModel() = 0;
    virtual void setProperties(const GPProperties& properties) = 0;
    virtual GPProperties properties() const = 0;
};
} // end MSTab namespace
#endif // IGPDOMBUILDER_H
