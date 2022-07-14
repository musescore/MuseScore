#ifndef GP7DOMBUILDER_H
#define GP7DOMBUILDER_H

#include "gpdommodel.h"
#include "gp67dombuilder.h"

namespace mu::engraving {
class GP7DomBuilder : public GP67DomBuilder
{
public:
    GP7DomBuilder() = default;

private:
    std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(XmlDomNode* trackNode) override;

    int readMidiChannel(XmlDomNode* trackChildNode) const;
    int readMidiProgramm(XmlDomNode* trackChildNode) const;
};
} //end Ms namespace
#endif // GP7DOMBUILDER_H
