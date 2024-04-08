#ifndef MU_IMPORTEXPORT_GP6DOMBUILDER_H
#define MU_IMPORTEXPORT_GP6DOMBUILDER_H

#include "gp67dombuilder.h"

namespace mu::iex::guitarpro {
class GP6DomBuilder : public GP67DomBuilder
{
public:
    GP6DomBuilder() = default;

private:
    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(muse::XmlDomNode* trackNode, muse::XmlDomNode* versionNode);
    void setUpInstrument(muse::XmlDomNode* trackChildNode, GPTrack* track);
    GPTrack::SoundAutomation readRsePickUp(muse::XmlDomNode& rseNode) const;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GP6DOMBUILDER_H
