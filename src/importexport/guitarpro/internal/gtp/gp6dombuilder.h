#ifndef GP6DOMBUILDER_H
#define GP6DOMBUILDER_H

#pragma once

#include "gp67dombuilder.h"

namespace Ms {
class GP6DomBuilder : public GP67DomBuilder
{
public:
    GP6DomBuilder() = default;

private:
    virtual std::pair<int, std::unique_ptr<GPTrack> > createGPTrack(QDomNode* trackNode);
    void setUpInstrument(QDomNode* trackChildNode, GPTrack* track);
};
} //end Ms namespace
#endif // GP6DOMBUILDER_H
