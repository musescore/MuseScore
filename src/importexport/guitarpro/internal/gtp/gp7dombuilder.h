#ifndef MU_IMPORTEXPORT_GP7DOMBUILDER_H
#define MU_IMPORTEXPORT_GP7DOMBUILDER_H

#include "gpdommodel.h"
#include "gp67dombuilder.h"

namespace mu::iex::guitarpro {
class GP7DomBuilder : public GP67DomBuilder
{
public:
    GP7DomBuilder() = default;

private:
    std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(XmlDomNode* trackNode, XmlDomNode* versionNode) override;

    int readMidiChannel(XmlDomNode* trackChildNode) const;
    int readMidiProgramm(XmlDomNode* trackChildNode) const;
    GPTrack::Sound readSounds(XmlDomNode* trackChildNode) const;
    GPTrack::SoundAutomation readTrackAutomation(XmlDomNode* automationNode) const;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GP7DOMBUILDER_H
