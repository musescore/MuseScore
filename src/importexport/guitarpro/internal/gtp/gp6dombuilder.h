#ifndef GP6DOMBUILDER_H
#define GP6DOMBUILDER_H

#pragma once

#include "gp67dombuilder.h"

namespace mu::engraving {
class GP6DomBuilder : public GP67DomBuilder
{
public:
    GP6DomBuilder() = default;

private:
    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(XmlDomNode* trackNode);
    void setUpInstrument(XmlDomNode* trackChildNode, GPTrack* track);
    GPTrack::SoundAutomation readRsePickUp(XmlDomNode& rseNode) const;
};
} //end Ms namespace
#endif // GP6DOMBUILDER_H
