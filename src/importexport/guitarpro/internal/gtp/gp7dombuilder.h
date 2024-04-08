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
    std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(muse::XmlDomNode* trackNode, muse::XmlDomNode* versionNode) override;

    int readMidiChannel(muse::XmlDomNode* trackChildNode) const;
    int readMidiProgramm(muse::XmlDomNode* trackChildNode) const;
    GPTrack::Sound readSounds(muse::XmlDomNode* trackChildNode) const;
    GPTrack::SoundAutomation readTrackAutomation(muse::XmlDomNode* automationNode) const;
};
} // namespace mu::iex::guitarpro
#endif // MU_IMPORTEXPORT_GP7DOMBUILDER_H
